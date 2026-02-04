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

#pragma once


//==============================================================================
namespace CodeHelpers
{
    Txt indent (const Txt& code, i32 numSpaces, b8 indentFirstLine);
    Txt unindent (const Txt& code, i32 numSpaces);

    Txt createIncludeStatement (const File& includedFile, const File& targetFile);
    Txt createIncludeStatement (const Txt& includePath);
    Txt createIncludePathIncludeStatement (const Txt& includedFilename);

    Txt stringLiteral (const Txt& text, i32 maxLineLength = -1);
    Txt floatLiteral (f64 value, i32 numDecPlaces);
    Txt boolLiteral (b8 value);

    Txt colourToCode (Color);
    Txt justificationToCode (Justification);

    Txt alignFunctionCallParams (const Txt& call, const StringArray& parameters, i32 maxLineLength);

    Txt getLeadingWhitespace (Txt line);
    i32 getBraceCount (Txt::CharPointerType line);
    b8 getIndentForCurrentBlock (CodeDocument::Position pos, const Txt& tab,
                                   Txt& blockIndent, Txt& lastLineIndent);
}
