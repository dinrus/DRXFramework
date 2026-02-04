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

/**
    Calculates and applies a sequence of changes to convert one text string into
    another.

    Once created, the TextDiff object contains an array of change objects, where
    each change can be either an insertion or a deletion. When applied in order
    to the original string, these changes will convert it to the target string.

    @tags{Core}
*/
class DRX_API TextDiff
{
public:
    /** Creates a set of diffs for converting the original string into the target. */
    TextDiff (const Txt& original,
              const Txt& target);

    /** Applies this sequence of changes to the original string, producing the
        target string that was specified when generating them.

        Obviously it only makes sense to call this function with the string that
        was originally passed to the constructor. Any other input will produce an
        undefined result.
    */
    Txt appliedTo (Txt text) const;

    /** Describes a change, which can be either an insertion or deletion. */
    struct Change
    {
        Txt insertedText; /**< If this change is a deletion, this string will be empty; otherwise,
                                  it'll be the text that should be inserted at the index specified by start. */
        i32 start;    /**< Specifies the character index in a string at which text should be inserted or deleted. */
        i32 length;   /**< If this change is a deletion, this specifies the number of characters to delete. For an
                           insertion, this is the length of the new text being inserted. */

        /** Возвращает true, если this change is a deletion, or false for an insertion. */
        b8 isDeletion() const noexcept;

        /** Returns the result of applying this change to a string. */
        Txt appliedTo (const Txt& original) const noexcept;
    };

    /** The list of changes required to perform the transformation.
        Applying each of these, in order, to the original string will produce the target.
    */
    Array<Change> changes;
};

} // namespace drx
