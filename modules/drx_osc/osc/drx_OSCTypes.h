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
/** The type used for OSC type tags. */
using OSCType = t8;


/** The type used for OSC type tag strings. */
using OSCTypeList = Array<OSCType>;

//==============================================================================
/** The definitions of supported OSC types and their associated OSC type tags,
    as defined in the OpenSoundControl 1.0 specification.

    Note: This implementation does not support any additional type tags that
    are not part of the specification.

    @tags{OSC}
*/
class DRX_API  OSCTypes
{
public:
    static const OSCType i32;
    static const OSCType float32;
    static const OSCType string;
    static const OSCType blob;
    static const OSCType colour;

    static b8 isSupportedType (OSCType type) noexcept
    {
        return type == OSCTypes::i32
            || type == OSCTypes::float32
            || type == OSCTypes::string
            || type == OSCTypes::blob
            || type == OSCTypes::colour;
    }
};


//==============================================================================
/**
    Holds a 32-bit RGBA colour for passing to and from an OSCArgument.
    @see OSCArgument, OSCTypes::colour
    @tags{OSC}
*/
struct OSCColor
{
    u8 red, green, blue, alpha;

    u32 toInt32() const;
    static OSCColor fromInt32 (u32);
};


//==============================================================================
/** Base class for exceptions that can be thrown by methods in the OSC module.

    @tags{OSC}
*/
struct OSCException  : public std::exception
{
    OSCException (const Txt& desc)
        : description (desc)
    {
       #if ! DRX_UNIT_TESTS
        DBG ("OSCFormatError: " + description);
       #endif
    }

    Txt description;
};

//==============================================================================
/** Exception type thrown when the OSC module fails to parse something because
    of a data format not compatible with the OpenSoundControl 1.0 specification.

    @tags{OSC}
*/
struct OSCFormatError : public OSCException
{
    OSCFormatError (const Txt& desc) : OSCException (desc) {}
};

//==============================================================================
/** Exception type thrown in cases of unexpected errors in the OSC module.

    Note: This should never happen, and all the places where this is thrown
    should have a preceding jassertfalse to facilitate debugging.

    @tags{OSC}
*/
struct OSCInternalError : public OSCException
{
    OSCInternalError (const Txt& desc) : OSCException (desc) {}
};

} // namespace drx
