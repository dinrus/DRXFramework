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

//==============================================================================
/**
    Represents a string identifier, designed for accessing properties by name.

    Comparing two Identifier objects is very fast (an O(1) operation), but creating
    them can be slower than just using a Txt directly, so the optimal way to use them
    is to keep some static Identifier objects for the things you use often.

    @see NamedValueSet, ValueTree

    @tags{Core}
*/
class DRX_API  Identifier  final
{
public:
    /** Creates a null identifier. */
    Identifier() noexcept;

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (tukk name);

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (const Txt& name);

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (Txt::CharPointerType nameStart, Txt::CharPointerType nameEnd);

    /** Creates a copy of another identifier. */
    Identifier (const Identifier& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier& operator= (const Identifier& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier (Identifier&& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier& operator= (Identifier&& other) noexcept;

    /** Destructor */
    ~Identifier() noexcept;

    /** Compares two identifiers. This is a very fast operation. */
    inline b8 operator== (const Identifier& other) const noexcept     { return name.getCharPointer() == other.name.getCharPointer(); }

    /** Compares two identifiers. This is a very fast operation. */
    inline b8 operator!= (const Identifier& other) const noexcept     { return name.getCharPointer() != other.name.getCharPointer(); }

    /** Compares the identifier with a string. */
    inline b8 operator== (StringRef other) const noexcept             { return name == other; }

    /** Compares the identifier with a string. */
    inline b8 operator!= (StringRef other) const noexcept             { return name != other; }

    /** Compares the identifier with a string. */
    inline b8 operator<  (StringRef other) const noexcept             { return name <  other; }

    /** Compares the identifier with a string. */
    inline b8 operator<= (StringRef other) const noexcept             { return name <= other; }

    /** Compares the identifier with a string. */
    inline b8 operator>  (StringRef other) const noexcept             { return name >  other; }

    /** Compares the identifier with a string. */
    inline b8 operator>= (StringRef other) const noexcept             { return name >= other; }

    /** Returns this identifier as a string. */
    const Txt& toString() const noexcept                             { return name; }

    /** Returns this identifier's raw string pointer. */
    operator Txt::CharPointerType() const noexcept                   { return name.getCharPointer(); }

    /** Returns this identifier's raw string pointer. */
    Txt::CharPointerType getCharPointer() const noexcept             { return name.getCharPointer(); }

    /** Returns this identifier as a StringRef. */
    operator StringRef() const noexcept                                 { return name; }

    /** Возвращает true, если this Identifier is not null */
    b8 isValid() const noexcept                                       { return name.isNotEmpty(); }

    /** Возвращает true, если this Identifier is null */
    b8 isNull() const noexcept                                        { return name.isEmpty(); }

    /** A null identifier. */
    static Identifier null;

    /** Checks a given string for characters that might not be valid in an Identifier.
        Since Identifiers are used as a script variables and XML attributes, they should only contain
        alphanumeric characters, underscores, or the '-' and ':' characters.
    */
    static b8 isValidIdentifier (const Txt& possibleIdentifier) noexcept;

private:
    Txt name;
};

} // namespace drx
