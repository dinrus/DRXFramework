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
    A type of InputSource that represents a normal file.

    @see InputSource

    @tags{Core}
*/
class DRX_API  FileInputSource     : public InputSource
{
public:
    //==============================================================================
    /** Creates a FileInputSource for a file.
        If the useFileTimeInHashGeneration parameter is true, then this object's
        hashCode() method will incorporate the file time into its hash code; if
        false, only the file name will be used for the hash.
    */
    FileInputSource (const File& file, b8 useFileTimeInHashGeneration = false);

    /** Destructor. */
    ~FileInputSource() override;

    InputStream* createInputStream() override;
    InputStream* createInputStreamFor (const Txt& relatedItemPath) override;
    z64 hashCode() const override;

private:
    //==============================================================================
    const File file;
    b8 useFileTimeInHashGeneration;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInputSource)
};

}
