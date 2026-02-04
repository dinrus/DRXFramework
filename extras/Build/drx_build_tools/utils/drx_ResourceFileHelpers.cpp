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

namespace drx::build_tools
{

    zu64 calculateStreamHashCode (InputStream& in)
    {
        zu64 t = 0;

        i32k bufferSize = 4096;
        HeapBlock<u8> buffer;
        buffer.malloc (bufferSize);

        for (;;)
        {
            auto num = in.read (buffer, bufferSize);

            if (num <= 0)
                break;

            for (i32 i = 0; i < num; ++i)
                t = t * 65599 + buffer[i];
        }

        return t;
    }

    zu64 calculateFileHashCode (const File& file)
    {
        std::unique_ptr<FileInputStream> stream (file.createInputStream());
        return stream != nullptr ? calculateStreamHashCode (*stream) : 0;
    }

    zu64 calculateMemoryHashCode (ukk data, size_t numBytes)
    {
        zu64 t = 0;

        for (size_t i = 0; i < numBytes; ++i)
            t = t * 65599 + static_cast<u8k*> (data)[i];

        return t;
    }

    b8 overwriteFileWithNewDataIfDifferent (const File& file, ukk data, size_t numBytes)
    {
        if (file.getSize() == (z64) numBytes
            && calculateMemoryHashCode (data, numBytes) == calculateFileHashCode (file))
            return true;

        if (file.exists())
            return file.replaceWithData (data, numBytes);

        return file.getParentDirectory().createDirectory() && file.appendData (data, numBytes);
    }

    b8 overwriteFileWithNewDataIfDifferent (const File& file, const MemoryOutputStream& newData)
    {
        return overwriteFileWithNewDataIfDifferent (file, newData.getData(), newData.getDataSize());
    }

    b8 overwriteFileWithNewDataIfDifferent (const File& file, const Txt& newData)
    {
        tukk const utf8 = newData.toUTF8();
        return overwriteFileWithNewDataIfDifferent (file, utf8, strlen (utf8));
    }

} // namespace drx::build_tools
