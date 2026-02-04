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

VBlankAttachment::VBlankAttachment (Component* c, std::function<z0()> callbackIn)
    : owner (c),
      callback ([cb = std::move (callbackIn)] (auto) { cb(); })
{
    jassert (owner != nullptr && callback);

    updateOwner();
    updatePeer();
}

VBlankAttachment::VBlankAttachment (Component* c, std::function<z0 (f64)> callbackIn)
    : owner (c),
      callback (std::move (callbackIn))
{
    jassert (owner != nullptr && callback);

    updateOwner();
    updatePeer();
}

VBlankAttachment::VBlankAttachment (VBlankAttachment&& other)
    : VBlankAttachment (other.owner, std::move (other.callback))
{
    other.cleanup();
}

VBlankAttachment& VBlankAttachment::operator= (VBlankAttachment&& other)
{
    cleanup();

    owner = other.owner;
    callback = std::move (other.callback);
    updateOwner();
    updatePeer();

    other.cleanup();

    return *this;
}

VBlankAttachment::~VBlankAttachment()
{
    cleanup();
}

z0 VBlankAttachment::onVBlank (f64 timestampMs)
{
    NullCheckedInvocation::invoke (callback, timestampMs);
}

z0 VBlankAttachment::componentParentHierarchyChanged (Component&)
{
    updatePeer();
}

z0 VBlankAttachment::updateOwner()
{
    if (auto previousLastOwner = std::exchange (lastOwner, owner); previousLastOwner != owner)
    {
        if (previousLastOwner != nullptr)
            previousLastOwner->removeComponentListener (this);

        if (owner != nullptr)
            owner->addComponentListener (this);
    }
}

z0 VBlankAttachment::updatePeer()
{
    if (owner != nullptr)
    {
        if (auto* peer = owner->getPeer())
        {
            peer->addVBlankListener (this);

            if (lastPeer != peer && ComponentPeer::isValidPeer (lastPeer))
                lastPeer->removeVBlankListener (this);

            lastPeer = peer;
        }
    }
    else if (auto peer = std::exchange (lastPeer, nullptr); ComponentPeer::isValidPeer (peer))
    {
        peer->removeVBlankListener (this);
    }
}

z0 VBlankAttachment::cleanup()
{
    owner = nullptr;
    updateOwner();
    updatePeer();
}

} // namespace drx
