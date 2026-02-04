/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

namespace ValueTreeSynchroniserHelpers
{
    enum ChangeType
    {
        propertyChanged  = 1,
        fullSync         = 2,
        childAdded       = 3,
        childRemoved     = 4,
        childMoved       = 5,
        propertyRemoved  = 6
    };

    static z0 getValueTreePath (ValueTree v, const ValueTree& topLevelTree, Array<i32>& path)
    {
        while (v != topLevelTree)
        {
            ValueTree parent (v.getParent());

            if (! parent.isValid())
                break;

            path.add (parent.indexOf (v));
            v = parent;
        }
    }

    static z0 writeHeader (MemoryOutputStream& stream, ChangeType type)
    {
        stream.writeByte ((t8) type);
    }

    static z0 writeHeader (ValueTreeSynchroniser& target, MemoryOutputStream& stream,
                             ChangeType type, ValueTree v)
    {
        writeHeader (stream, type);

        Array<i32> path;
        getValueTreePath (v, target.getRoot(), path);

        stream.writeCompressedInt (path.size());

        for (i32 i = path.size(); --i >= 0;)
            stream.writeCompressedInt (path.getUnchecked (i));
    }

    static ValueTree readSubTreeLocation (MemoryInputStream& input, ValueTree v)
    {
        i32k numLevels = input.readCompressedInt();

        if (! isPositiveAndBelow (numLevels, 65536)) // sanity-check
            return {};

        for (i32 i = numLevels; --i >= 0;)
        {
            i32k index = input.readCompressedInt();

            if (! isPositiveAndBelow (index, v.getNumChildren()))
                return {};

            v = v.getChild (index);
        }

        return v;
    }
}

ValueTreeSynchroniser::ValueTreeSynchroniser (const ValueTree& tree)  : valueTree (tree)
{
    valueTree.addListener (this);
}

ValueTreeSynchroniser::~ValueTreeSynchroniser()
{
    valueTree.removeListener (this);
}

z0 ValueTreeSynchroniser::sendFullSyncCallback()
{
    MemoryOutputStream m;
    writeHeader (m, ValueTreeSynchroniserHelpers::fullSync);
    valueTree.writeToStream (m);
    stateChanged (m.getData(), m.getDataSize());
}

z0 ValueTreeSynchroniser::valueTreePropertyChanged (ValueTree& vt, const Identifier& property)
{
    MemoryOutputStream m;

    if (auto* value = vt.getPropertyPointer (property))
    {
        ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::propertyChanged, vt);
        m.writeString (property.toString());
        value->writeToStream (m);
    }
    else
    {
        ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::propertyRemoved, vt);
        m.writeString (property.toString());
    }

    stateChanged (m.getData(), m.getDataSize());
}

z0 ValueTreeSynchroniser::valueTreeChildAdded (ValueTree& parentTree, ValueTree& childTree)
{
    i32k index = parentTree.indexOf (childTree);
    jassert (index >= 0);

    MemoryOutputStream m;
    ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::childAdded, parentTree);
    m.writeCompressedInt (index);
    childTree.writeToStream (m);
    stateChanged (m.getData(), m.getDataSize());
}

z0 ValueTreeSynchroniser::valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, i32 oldIndex)
{
    MemoryOutputStream m;
    ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::childRemoved, parentTree);
    m.writeCompressedInt (oldIndex);
    stateChanged (m.getData(), m.getDataSize());
}

z0 ValueTreeSynchroniser::valueTreeChildOrderChanged (ValueTree& parent, i32 oldIndex, i32 newIndex)
{
    MemoryOutputStream m;
    ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::childMoved, parent);
    m.writeCompressedInt (oldIndex);
    m.writeCompressedInt (newIndex);
    stateChanged (m.getData(), m.getDataSize());
}

b8 ValueTreeSynchroniser::applyChange (ValueTree& root, ukk data, size_t dataSize, UndoManager* undoManager)
{
    MemoryInputStream input (data, dataSize, false);

    const ValueTreeSynchroniserHelpers::ChangeType type = (ValueTreeSynchroniserHelpers::ChangeType) input.readByte();

    if (type == ValueTreeSynchroniserHelpers::fullSync)
    {
        root = ValueTree::readFromStream (input);
        return true;
    }

    ValueTree v (ValueTreeSynchroniserHelpers::readSubTreeLocation (input, root));

    if (! v.isValid())
        return false;

    switch (type)
    {
        case ValueTreeSynchroniserHelpers::propertyChanged:
        {
            Identifier property (input.readString());
            v.setProperty (property, var::readFromStream (input), undoManager);
            return true;
        }

        case ValueTreeSynchroniserHelpers::propertyRemoved:
        {
            Identifier property (input.readString());
            v.removeProperty (property, undoManager);
            return true;
        }

        case ValueTreeSynchroniserHelpers::childAdded:
        {
            i32k index = input.readCompressedInt();
            v.addChild (ValueTree::readFromStream (input), index, undoManager);
            return true;
        }

        case ValueTreeSynchroniserHelpers::childRemoved:
        {
            i32k index = input.readCompressedInt();

            if (isPositiveAndBelow (index, v.getNumChildren()))
            {
                v.removeChild (index, undoManager);
                return true;
            }

            jassertfalse; // Either received some corrupt data, or the trees have drifted out of sync
            break;
        }

        case ValueTreeSynchroniserHelpers::childMoved:
        {
            i32k oldIndex = input.readCompressedInt();
            i32k newIndex = input.readCompressedInt();

            if (isPositiveAndBelow (oldIndex, v.getNumChildren())
                 && isPositiveAndBelow (newIndex, v.getNumChildren()))
            {
                v.moveChild (oldIndex, newIndex, undoManager);
                return true;
            }

            jassertfalse; // Either received some corrupt data, or the trees have drifted out of sync
            break;
        }

        case ValueTreeSynchroniserHelpers::fullSync:
            break;

        default:
            jassertfalse; // Seem to have received some corrupt data?
            break;
    }

    return false;
}

} // namespace drx
