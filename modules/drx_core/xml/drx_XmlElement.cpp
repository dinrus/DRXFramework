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

static b8 isValidXmlNameStartCharacter (t32 character) noexcept
{
    return character == ':'
        || character == '_'
        || (character >= 'a'     && character <= 'z')
        || (character >= 'A'     && character <= 'Z')
        || (character >= 0xc0    && character <= 0xd6)
        || (character >= 0xd8    && character <= 0xf6)
        || (character >= 0xf8    && character <= 0x2ff)
        || (character >= 0x370   && character <= 0x37d)
        || (character >= 0x37f   && character <= 0x1fff)
        || (character >= 0x200c  && character <= 0x200d)
        || (character >= 0x2070  && character <= 0x218f)
        || (character >= 0x2c00  && character <= 0x2fef)
        || (character >= 0x3001  && character <= 0xd7ff)
        || (character >= 0xf900  && character <= 0xfdcf)
        || (character >= 0xfdf0  && character <= 0xfffd)
        || (character >= 0x10000 && character <= 0xeffff);
}

static b8 isValidXmlNameBodyCharacter (t32 character) noexcept
{
    return isValidXmlNameStartCharacter (character)
        || character == '-'
        || character == '.'
        || character == 0xb7
        || (character >= '0'    && character <= '9')
        || (character >= 0x300  && character <= 0x036f)
        || (character >= 0x203f && character <= 0x2040);
}

XmlElement::XmlAttributeNode::XmlAttributeNode (const XmlAttributeNode& other) noexcept
    : name (other.name),
      value (other.value)
{
}

XmlElement::XmlAttributeNode::XmlAttributeNode (const Identifier& n, const Txt& v) noexcept
    : name (n), value (v)
{
    jassert (isValidXmlName (name));
}

XmlElement::XmlAttributeNode::XmlAttributeNode (Txt::CharPointerType nameStart, Txt::CharPointerType nameEnd)
    : name (nameStart, nameEnd)
{
    jassert (isValidXmlName (name));
}

//==============================================================================
XmlElement::XmlElement (const Txt& tag)
    : tagName (StringPool::getGlobalPool().getPooledString (tag))
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (tukk tag)
    : tagName (StringPool::getGlobalPool().getPooledString (tag))
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (StringRef tag)
    : tagName (StringPool::getGlobalPool().getPooledString (tag))
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (const Identifier& tag)
    : tagName (tag.toString())
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (Txt::CharPointerType tagNameStart, Txt::CharPointerType tagNameEnd)
    : tagName (StringPool::getGlobalPool().getPooledString (tagNameStart, tagNameEnd))
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (i32 /*dummy*/) noexcept
{
}

XmlElement::XmlElement (const XmlElement& other)
    : tagName (other.tagName)
{
    copyChildrenAndAttributesFrom (other);
}

XmlElement& XmlElement::operator= (const XmlElement& other)
{
    if (this != &other)
    {
        removeAllAttributes();
        deleteAllChildElements();
        tagName = other.tagName;
        copyChildrenAndAttributesFrom (other);
    }

    return *this;
}

XmlElement::XmlElement (XmlElement&& other) noexcept
    : nextListItem      (std::move (other.nextListItem)),
      firstChildElement (std::move (other.firstChildElement)),
      attributes        (std::move (other.attributes)),
      tagName           (std::move (other.tagName))
{
}

XmlElement& XmlElement::operator= (XmlElement&& other) noexcept
{
    jassert (this != &other); // hopefully the compiler should make this situation impossible!

    removeAllAttributes();
    deleteAllChildElements();

    nextListItem      = std::move (other.nextListItem);
    firstChildElement = std::move (other.firstChildElement);
    attributes        = std::move (other.attributes);
    tagName           = std::move (other.tagName);

    return *this;
}

z0 XmlElement::copyChildrenAndAttributesFrom (const XmlElement& other)
{
    jassert (firstChildElement.get() == nullptr);
    firstChildElement.addCopyOfList (other.firstChildElement);

    jassert (attributes.get() == nullptr);
    attributes.addCopyOfList (other.attributes);
}

XmlElement::~XmlElement() noexcept
{
    firstChildElement.deleteAll();
    attributes.deleteAll();
}

//==============================================================================
namespace XmlOutputFunctions
{
    namespace LegalCharLookupTable
    {
        template <i32 c>
        struct Bit
        {
            enum { v = ((c >= 'a' && c <= 'z')
                     || (c >= 'A' && c <= 'Z')
                     || (c >= '0' && c <= '9')
                     || c == ' ' || c == '.'  || c == ',' || c == ';'
                     || c == ':' || c == '-'  || c == '(' || c == ')'
                     || c == '_' || c == '+'  || c == '=' || c == '?'
                     || c == '!' || c == '$'  || c == '#' || c == '@'
                     || c == '[' || c == ']'  || c == '/' || c == '|'
                     || c == '*' || c == '%'  || c == '~' || c == '{'
                     || c == '}' || c == '\'' || c == '\\')
                        ? (1 << (c & 7)) : 0 };
        };

        template <i32 tableIndex>
        struct Byte
        {
            enum { v = (i32) Bit<tableIndex * 8 + 0>::v | (i32) Bit<tableIndex * 8 + 1>::v
                     | (i32) Bit<tableIndex * 8 + 2>::v | (i32) Bit<tableIndex * 8 + 3>::v
                     | (i32) Bit<tableIndex * 8 + 4>::v | (i32) Bit<tableIndex * 8 + 5>::v
                     | (i32) Bit<tableIndex * 8 + 6>::v | (i32) Bit<tableIndex * 8 + 7>::v };
        };

        static b8 isLegal (u32 c) noexcept
        {
            static u8k legalChars[] = { Byte< 0>::v, Byte< 1>::v, Byte< 2>::v, Byte< 3>::v,
                                                        Byte< 4>::v, Byte< 5>::v, Byte< 6>::v, Byte< 7>::v,
                                                        Byte< 8>::v, Byte< 9>::v, Byte<10>::v, Byte<11>::v,
                                                        Byte<12>::v, Byte<13>::v, Byte<14>::v, Byte<15>::v };

            return c < sizeof (legalChars) * 8
                     && (legalChars[c >> 3] & (1 << (c & 7))) != 0;
        }
    }

    static z0 escapeIllegalXmlChars (OutputStream& outputStream, const Txt& text, b8 changeNewLines)
    {
        auto t = text.getCharPointer();

        for (;;)
        {
            auto character = (u32) t.getAndAdvance();

            if (character == 0)
                break;

            if (LegalCharLookupTable::isLegal (character))
            {
                outputStream << (t8) character;
            }
            else
            {
                switch (character)
                {
                    case '&':   outputStream << "&amp;"; break;
                    case '"':   outputStream << "&quot;"; break;
                    case '>':   outputStream << "&gt;"; break;
                    case '<':   outputStream << "&lt;"; break;

                    case '\n':
                    case '\r':
                        if (! changeNewLines)
                        {
                            outputStream << (t8) character;
                            break;
                        }
                        DRX_FALLTHROUGH
                    default:
                        outputStream << "&#" << ((i32) character) << ';';
                        break;
                }
            }
        }
    }

    static z0 writeSpaces (OutputStream& out, const size_t numSpaces)
    {
        out.writeRepeatedByte (' ', numSpaces);
    }
}

z0 XmlElement::writeElementAsText (OutputStream& outputStream,
                                     i32 indentationLevel,
                                     i32 lineWrapLength,
                                     tukk newLineChars) const
{
    if (indentationLevel >= 0)
        XmlOutputFunctions::writeSpaces (outputStream, (size_t) indentationLevel);

    if (! isTextElement())
    {
        outputStream.writeByte ('<');
        outputStream << tagName;

        {
            auto attIndent = (size_t) (indentationLevel + tagName.length() + 1);
            i32 lineLen = 0;

            for (auto* att = attributes.get(); att != nullptr; att = att->nextListItem)
            {
                if (lineLen > lineWrapLength && indentationLevel >= 0)
                {
                    outputStream << newLineChars;
                    XmlOutputFunctions::writeSpaces (outputStream, attIndent);
                    lineLen = 0;
                }

                auto startPos = outputStream.getPosition();
                outputStream.writeByte (' ');
                outputStream << att->name;
                outputStream.write ("=\"", 2);
                XmlOutputFunctions::escapeIllegalXmlChars (outputStream, att->value, true);
                outputStream.writeByte ('"');
                lineLen += (i32) (outputStream.getPosition() - startPos);
            }
        }

        if (auto* child = firstChildElement.get())
        {
            outputStream.writeByte ('>');
            b8 lastWasTextNode = false;

            for (; child != nullptr; child = child->nextListItem)
            {
                if (child->isTextElement())
                {
                    XmlOutputFunctions::escapeIllegalXmlChars (outputStream, child->getText(), false);
                    lastWasTextNode = true;
                }
                else
                {
                    if (indentationLevel >= 0 && ! lastWasTextNode)
                        outputStream << newLineChars;

                    child->writeElementAsText (outputStream,
                                               lastWasTextNode ? 0 : (indentationLevel + (indentationLevel >= 0 ? 2 : 0)), lineWrapLength,
                                               newLineChars);
                    lastWasTextNode = false;
                }
            }

            if (indentationLevel >= 0 && ! lastWasTextNode)
            {
                outputStream << newLineChars;
                XmlOutputFunctions::writeSpaces (outputStream, (size_t) indentationLevel);
            }

            outputStream.write ("</", 2);
            outputStream << tagName;
            outputStream.writeByte ('>');
        }
        else
        {
            outputStream.write ("/>", 2);
        }
    }
    else
    {
        XmlOutputFunctions::escapeIllegalXmlChars (outputStream, getText(), false);
    }
}

XmlElement::TextFormat::TextFormat() {}

XmlElement::TextFormat XmlElement::TextFormat::singleLine() const
{
    auto f = *this;
    f.newLineChars = nullptr;
    return f;
}

XmlElement::TextFormat XmlElement::TextFormat::withoutHeader() const
{
    auto f = *this;
    f.addDefaultHeader = false;
    return f;
}

Txt XmlElement::toString (const TextFormat& options) const
{
    MemoryOutputStream mem (2048);
    writeTo (mem, options);
    return mem.toUTF8();
}

z0 XmlElement::writeTo (OutputStream& output, const TextFormat& options) const
{
    if (options.customHeader.isNotEmpty())
    {
        output << options.customHeader;

        if (options.newLineChars == nullptr)
            output.writeByte (' ');
        else
            output << options.newLineChars
                   << options.newLineChars;
    }
    else if (options.addDefaultHeader)
    {
        output << "<?xml version=\"1.0\" encoding=\"";

        if (options.customEncoding.isNotEmpty())
            output << options.customEncoding;
        else
            output << "UTF-8";

        output << "\"?>";

        if (options.newLineChars == nullptr)
            output.writeByte (' ');
        else
            output << options.newLineChars
                   << options.newLineChars;
    }

    if (options.dtd.isNotEmpty())
    {
        output << options.dtd;

        if (options.newLineChars == nullptr)
            output.writeByte (' ');
        else
            output << options.newLineChars;
    }

    writeElementAsText (output, options.newLineChars == nullptr ? -1 : 0,
                        options.lineWrapLength,
                        options.newLineChars);

    if (options.newLineChars != nullptr)
        output << options.newLineChars;
}

b8 XmlElement::writeTo (const File& destinationFile, const TextFormat& options) const
{
    TemporaryFile tempFile (destinationFile);

    {
        FileOutputStream out (tempFile.getFile());

        if (! out.openedOk())
            return false;

        writeTo (out, options);
        out.flush(); // (called explicitly to force an fsync on posix)

        if (out.getStatus().failed())
            return false;
    }

    return tempFile.overwriteTargetFileWithTemporary();
}

Txt XmlElement::createDocument (StringRef dtdToUse, b8 allOnOneLine, b8 includeXmlHeader,
                                   StringRef encodingType, i32 lineWrapLength) const
{
    TextFormat options;
    options.dtd = dtdToUse;
    options.customEncoding = encodingType;
    options.addDefaultHeader = includeXmlHeader;
    options.lineWrapLength = lineWrapLength;

    if (allOnOneLine)
        options.newLineChars = nullptr;

    return toString (options);
}

z0 XmlElement::writeToStream (OutputStream& output, StringRef dtdToUse,
                                b8 allOnOneLine, b8 includeXmlHeader,
                                StringRef encodingType, i32 lineWrapLength) const
{
    TextFormat options;
    options.dtd = dtdToUse;
    options.customEncoding = encodingType;
    options.addDefaultHeader = includeXmlHeader;
    options.lineWrapLength = lineWrapLength;

    if (allOnOneLine)
        options.newLineChars = nullptr;

    writeTo (output, options);
}

b8 XmlElement::writeToFile (const File& file, StringRef dtdToUse,
                              StringRef encodingType, i32 lineWrapLength) const
{
    TextFormat options;
    options.dtd = dtdToUse;
    options.customEncoding = encodingType;
    options.lineWrapLength = lineWrapLength;

    return writeTo (file, options);
}

//==============================================================================
b8 XmlElement::hasTagName (StringRef possibleTagName) const noexcept
{
    const b8 matches = tagName.equalsIgnoreCase (possibleTagName);

    // XML tags should be case-sensitive, so although this method allows a
    // case-insensitive match to pass, you should try to avoid this.
    jassert ((! matches) || tagName == possibleTagName);

    return matches;
}

Txt XmlElement::getNamespace() const
{
    return tagName.upToFirstOccurrenceOf (":", false, false);
}

Txt XmlElement::getTagNameWithoutNamespace() const
{
    return tagName.fromLastOccurrenceOf (":", false, false);
}

b8 XmlElement::hasTagNameIgnoringNamespace (StringRef possibleTagName) const
{
    return hasTagName (possibleTagName) || getTagNameWithoutNamespace() == possibleTagName;
}

XmlElement* XmlElement::getNextElementWithTagName (StringRef requiredTagName) const
{
    auto* e = nextListItem.get();

    while (e != nullptr && ! e->hasTagName (requiredTagName))
        e = e->nextListItem;

    return e;
}

z0 XmlElement::setTagName (StringRef newTagName)
{
    jassert (isValidXmlName (newTagName));
    tagName = StringPool::getGlobalPool().getPooledString (newTagName);
}

//==============================================================================
i32 XmlElement::getNumAttributes() const noexcept
{
    return attributes.size();
}

static const Txt& getEmptyStringRef() noexcept
{
    static Txt empty;
    return empty;
}

const Txt& XmlElement::getAttributeName (i32k index) const noexcept
{
    if (auto* att = attributes[index].get())
        return att->name.toString();

    return getEmptyStringRef();
}

const Txt& XmlElement::getAttributeValue (i32k index) const noexcept
{
    if (auto* att = attributes[index].get())
        return att->value;

    return getEmptyStringRef();
}

XmlElement::XmlAttributeNode* XmlElement::getAttribute (StringRef attributeName) const noexcept
{
    for (auto* att = attributes.get(); att != nullptr; att = att->nextListItem)
        if (att->name == attributeName)
            return att;

    return nullptr;
}

b8 XmlElement::hasAttribute (StringRef attributeName) const noexcept
{
    return getAttribute (attributeName) != nullptr;
}

//==============================================================================
const Txt& XmlElement::getStringAttribute (StringRef attributeName) const noexcept
{
    if (auto* att = getAttribute (attributeName))
        return att->value;

    return getEmptyStringRef();
}

Txt XmlElement::getStringAttribute (StringRef attributeName, const Txt& defaultReturnValue) const
{
    if (auto* att = getAttribute (attributeName))
        return att->value;

    return defaultReturnValue;
}

i32 XmlElement::getIntAttribute (StringRef attributeName, i32k defaultReturnValue) const
{
    if (auto* att = getAttribute (attributeName))
        return att->value.getIntValue();

    return defaultReturnValue;
}

f64 XmlElement::getDoubleAttribute (StringRef attributeName, const f64 defaultReturnValue) const
{
    if (auto* att = getAttribute (attributeName))
        return att->value.getDoubleValue();

    return defaultReturnValue;
}

b8 XmlElement::getBoolAttribute (StringRef attributeName, const b8 defaultReturnValue) const
{
    if (auto* att = getAttribute (attributeName))
    {
        auto firstChar = *(att->value.getCharPointer().findEndOfWhitespace());

        return firstChar == '1'
            || firstChar == 't'
            || firstChar == 'y'
            || firstChar == 'T'
            || firstChar == 'Y';
    }

    return defaultReturnValue;
}

b8 XmlElement::compareAttribute (StringRef attributeName,
                                   StringRef stringToCompareAgainst,
                                   const b8 ignoreCase) const noexcept
{
    if (auto* att = getAttribute (attributeName))
        return ignoreCase ? att->value.equalsIgnoreCase (stringToCompareAgainst)
                          : att->value == stringToCompareAgainst;

    return false;
}

//==============================================================================
z0 XmlElement::setAttribute (const Identifier& attributeName, const Txt& value)
{
    if (attributes == nullptr)
    {
        attributes = new XmlAttributeNode (attributeName, value);
    }
    else
    {
        for (auto* att = attributes.get(); ; att = att->nextListItem)
        {
            if (att->name == attributeName)
            {
                att->value = value;
                break;
            }

            if (att->nextListItem == nullptr)
            {
                att->nextListItem = new XmlAttributeNode (attributeName, value);
                break;
            }
        }
    }
}

z0 XmlElement::setAttribute (const Identifier& attributeName, i32k number)
{
    setAttribute (attributeName, Txt (number));
}

z0 XmlElement::setAttribute (const Identifier& attributeName, const f64 number)
{
    setAttribute (attributeName, serialiseDouble (number));
}

z0 XmlElement::removeAttribute (const Identifier& attributeName) noexcept
{
    for (auto* att = &attributes; att->get() != nullptr; att = &(att->get()->nextListItem))
    {
        if (att->get()->name == attributeName)
        {
            delete att->removeNext();
            break;
        }
    }
}

z0 XmlElement::removeAllAttributes() noexcept
{
    attributes.deleteAll();
}

//==============================================================================
i32 XmlElement::getNumChildElements() const noexcept
{
    return firstChildElement.size();
}

XmlElement* XmlElement::getChildElement (i32k index) const noexcept
{
    return firstChildElement[index].get();
}

XmlElement* XmlElement::getChildByName (StringRef childName) const noexcept
{
    jassert (! childName.isEmpty());

    for (auto* child = firstChildElement.get(); child != nullptr; child = child->nextListItem)
        if (child->hasTagName (childName))
            return child;

    return nullptr;
}

XmlElement* XmlElement::getChildByAttribute (StringRef attributeName, StringRef attributeValue) const noexcept
{
    jassert (! attributeName.isEmpty());

    for (auto* child = firstChildElement.get(); child != nullptr; child = child->nextListItem)
        if (child->compareAttribute (attributeName, attributeValue))
            return child;

    return nullptr;
}

z0 XmlElement::addChildElement (XmlElement* const newNode) noexcept
{
    if (newNode != nullptr)
    {
        // The element being added must not be a child of another node!
        jassert (newNode->nextListItem == nullptr);

        firstChildElement.append (newNode);
    }
}

z0 XmlElement::insertChildElement (XmlElement* const newNode, i32 indexToInsertAt) noexcept
{
    if (newNode != nullptr)
    {
        // The element being added must not be a child of another node!
        jassert (newNode->nextListItem == nullptr);

        firstChildElement.insertAtIndex (indexToInsertAt, newNode);
    }
}

z0 XmlElement::prependChildElement (XmlElement* newNode) noexcept
{
    if (newNode != nullptr)
    {
        // The element being added must not be a child of another node!
        jassert (newNode->nextListItem == nullptr);

        firstChildElement.insertNext (newNode);
    }
}

XmlElement* XmlElement::createNewChildElement (StringRef childTagName)
{
    auto newElement = new XmlElement (childTagName);
    addChildElement (newElement);
    return newElement;
}

b8 XmlElement::replaceChildElement (XmlElement* const currentChildElement,
                                      XmlElement* const newNode) noexcept
{
    if (newNode != nullptr)
    {
        if (auto* p = firstChildElement.findPointerTo (currentChildElement))
        {
            if (currentChildElement != newNode)
                delete p->replaceNext (newNode);

            return true;
        }
    }

    return false;
}

z0 XmlElement::removeChildElement (XmlElement* const childToRemove,
                                     const b8 shouldDeleteTheChild) noexcept
{
    if (childToRemove != nullptr)
    {
        jassert (containsChildElement (childToRemove));

        firstChildElement.remove (childToRemove);

        if (shouldDeleteTheChild)
            delete childToRemove;
    }
}

b8 XmlElement::isEquivalentTo (const XmlElement* const other,
                                 const b8 ignoreOrderOfAttributes) const noexcept
{
    if (this != other)
    {
        if (other == nullptr || tagName != other->tagName)
            return false;

        if (ignoreOrderOfAttributes)
        {
            i32 totalAtts = 0;

            for (auto* att = attributes.get(); att != nullptr; att = att->nextListItem)
            {
                if (! other->compareAttribute (att->name, att->value))
                    return false;

                ++totalAtts;
            }

            if (totalAtts != other->getNumAttributes())
                return false;
        }
        else
        {
            auto* thisAtt = attributes.get();
            auto* otherAtt = other->attributes.get();

            for (;;)
            {
                if (thisAtt == nullptr || otherAtt == nullptr)
                {
                    if (thisAtt == otherAtt) // both nullptr, so it's a match
                        break;

                    return false;
                }

                if (thisAtt->name != otherAtt->name
                     || thisAtt->value != otherAtt->value)
                {
                    return false;
                }

                thisAtt = thisAtt->nextListItem;
                otherAtt = otherAtt->nextListItem;
            }
        }

        auto* thisChild = firstChildElement.get();
        auto* otherChild = other->firstChildElement.get();

        for (;;)
        {
            if (thisChild == nullptr || otherChild == nullptr)
            {
                if (thisChild == otherChild) // both 0, so it's a match
                    break;

                return false;
            }

            if (! thisChild->isEquivalentTo (otherChild, ignoreOrderOfAttributes))
                return false;

            thisChild = thisChild->nextListItem;
            otherChild = otherChild->nextListItem;
        }
    }

    return true;
}

z0 XmlElement::deleteAllChildElements() noexcept
{
    firstChildElement.deleteAll();
}

z0 XmlElement::deleteAllChildElementsWithTagName (StringRef name) noexcept
{
    for (auto* child = firstChildElement.get(); child != nullptr;)
    {
        auto* nextChild = child->nextListItem.get();

        if (child->hasTagName (name))
            removeChildElement (child, true);

        child = nextChild;
    }
}

b8 XmlElement::containsChildElement (const XmlElement* const possibleChild) const noexcept
{
    return firstChildElement.contains (possibleChild);
}

XmlElement* XmlElement::findParentElementOf (const XmlElement* const elementToLookFor) noexcept
{
    if (this == elementToLookFor || elementToLookFor == nullptr)
        return nullptr;

    for (auto* child = firstChildElement.get(); child != nullptr; child = child->nextListItem)
    {
        if (elementToLookFor == child)
            return this;

        if (auto* found = child->findParentElementOf (elementToLookFor))
            return found;
    }

    return nullptr;
}

z0 XmlElement::getChildElementsAsArray (XmlElement** elems) const noexcept
{
    firstChildElement.copyToArray (elems);
}

z0 XmlElement::reorderChildElements (XmlElement** elems, i32 num) noexcept
{
    auto* e = elems[0];
    firstChildElement = e;

    for (i32 i = 1; i < num; ++i)
    {
        e->nextListItem = elems[i];
        e = e->nextListItem;
    }

    e->nextListItem = nullptr;
}

//==============================================================================
b8 XmlElement::isTextElement() const noexcept
{
    return tagName.isEmpty();
}

static const Txt& getDrxXmlTextContentAttributeName()
{
    static Txt result { "text" };
    return result;
}

const Txt& XmlElement::getText() const noexcept
{
    jassert (isTextElement());  // you're trying to get the text from an element that
                                // isn't actually a text element.. If this contains text sub-nodes, you
                                // probably want to use getAllSubText instead.

    return getStringAttribute (getDrxXmlTextContentAttributeName());
}

z0 XmlElement::setText (const Txt& newText)
{
    if (isTextElement())
        setAttribute (getDrxXmlTextContentAttributeName(), newText);
    else
        jassertfalse; // you can only change the text in a text element, not a normal one.
}

Txt XmlElement::getAllSubText() const
{
    if (isTextElement())
        return getText();

    if (getNumChildElements() == 1)
        return firstChildElement.get()->getAllSubText();

    MemoryOutputStream mem (1024);

    for (auto* child = firstChildElement.get(); child != nullptr; child = child->nextListItem)
        mem << child->getAllSubText();

    return mem.toUTF8();
}

Txt XmlElement::getChildElementAllSubText (StringRef childTagName, const Txt& defaultReturnValue) const
{
    if (auto* child = getChildByName (childTagName))
        return child->getAllSubText();

    return defaultReturnValue;
}

XmlElement* XmlElement::createTextElement (const Txt& text)
{
    auto e = new XmlElement ((i32) 0);
    e->setAttribute (getDrxXmlTextContentAttributeName(), text);
    return e;
}

b8 XmlElement::isValidXmlName (StringRef text) noexcept
{
    if (text.isEmpty() || ! isValidXmlNameStartCharacter (text.text.getAndAdvance()))
        return false;

    for (;;)
    {
        if (text.isEmpty())
            return true;

        if (! isValidXmlNameBodyCharacter (text.text.getAndAdvance()))
            return false;
    }
}

z0 XmlElement::addTextElement (const Txt& text)
{
    addChildElement (createTextElement (text));
}

z0 XmlElement::deleteAllTextElements() noexcept
{
    for (auto* child = firstChildElement.get(); child != nullptr;)
    {
        auto* next = child->nextListItem.get();

        if (child->isTextElement())
            removeChildElement (child, true);

        child = next;
    }
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class XmlElementTests final : public UnitTest
{
public:
    XmlElementTests()
        : UnitTest ("XmlElement", UnitTestCategories::xml)
    {}

    z0 runTest() override
    {
        {
            beginTest ("Float formatting");

            auto element = std::make_unique<XmlElement> ("test");
            Identifier number ("number");

            std::map<f64, Txt> tests;
            tests[1] = "1.0";
            tests[1.1] = "1.1";
            tests[1.01] = "1.01";
            tests[0.76378] = "0.76378";
            tests[-10] = "-10.0";
            tests[10.01] = "10.01";
            tests[0.0123] = "0.0123";
            tests[-3.7e-27] = "-3.7e-27";
            tests[1e+40] = "1.0e40";
            tests[-12345678901234567.0] = "-1.234567890123457e16";
            tests[192000] = "192000.0";
            tests[1234567] = "1.234567e6";
            tests[0.00006] = "0.00006";
            tests[0.000006] = "6.0e-6";

            for (auto& test : tests)
            {
                element->setAttribute (number, test.first);
                expectEquals (element->getStringAttribute (number), test.second);
            }
        }
    }
};

static XmlElementTests xmlElementTests;

#endif

} // namespace drx
