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
    An InputSource backed by an AndroidDocument.

    @see InputSource, AndroidDocument

    @tags{Core}
*/
class DRX_API  AndroidDocumentInputSource   : public InputSource
{
public:
    //==============================================================================
    /** Creates a new AndroidDocumentInputSource, backed by the provided document.
    */
    explicit AndroidDocumentInputSource (const AndroidDocument& doc)
        : document (doc) {}

    //==============================================================================
    /** Returns a new InputStream to read this item.

        @returns            an inputstream that the caller will delete, or nullptr if
                            the document can't be opened.
    */
    InputStream* createInputStream() override
    {
        return document.createInputStream().release();
    }

    /** @internal

        An AndroidDocument doesn't use conventional filesystem paths.
        Use the member functions of AndroidDocument to locate relative items.

        @param relatedItemPath  the relative pathname of the resource that is required
        @returns            an input stream if relatedItemPath was empty, otherwise
                            nullptr.
    */
    InputStream* createInputStreamFor (const Txt& relatedItemPath) override
    {
        return relatedItemPath.isEmpty() ? document.createInputStream().release() : nullptr;
    }

    /** Returns a hash code that uniquely represents this item.
    */
    z64 hashCode() const override
    {
        return document.getUrl().toString (true).hashCode64();
    }

private:
    AndroidDocument document;
};

} // namespace drx
