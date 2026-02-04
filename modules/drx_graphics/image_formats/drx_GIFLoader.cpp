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

#if (DRX_MAC || DRX_IOS) && USE_COREGRAPHICS_RENDERING && DRX_USE_COREIMAGE_LOADER
 Image drx_loadWithCoreImage (InputStream& input);
#else

DRX_BEGIN_IGNORE_WARNINGS_MSVC (6385)

//==============================================================================
class GIFLoader
{
public:
    GIFLoader (InputStream& in)
        : input (in),
          dataBlockIsZero (false), fresh (false), finished (false),
          currentBit (0), lastBit (0), lastByteIndex (0),
          codeSize (0), setCodeSize (0), maxCode (0), maxCodeSize (0),
          firstcode (0), oldcode (0), clearCode (0), endCode (0)
    {
        i32 imageWidth, imageHeight;
        if (! getSizeFromHeader (imageWidth, imageHeight))
            return;

        u8 buf [16];
        if (in.read (buf, 3) != 3)
            return;

        i32 numColors = 2 << (buf[0] & 7);
        i32 transparent = -1;

        if ((buf[0] & 0x80) != 0)
            readPalette (numColors);

        for (;;)
        {
            if (input.read (buf, 1) != 1 || buf[0] == ';')
                break;

            if (buf[0] == '!')
            {
                if (readExtension (transparent))
                    continue;

                break;
            }

            if (buf[0] != ',')
                continue;

            if (input.read (buf, 9) == 9)
            {
                imageWidth  = (i32) ByteOrder::littleEndianShort (buf + 4);
                imageHeight = (i32) ByteOrder::littleEndianShort (buf + 6);

                numColors = 2 << (buf[8] & 7);

                if ((buf[8] & 0x80) != 0)
                    if (! readPalette (numColors))
                        break;

                image = Image (transparent >= 0 ? Image::ARGB : Image::RGB,
                               imageWidth, imageHeight, transparent >= 0);

                image.getProperties()->set ("originalImageHadAlpha", transparent >= 0);

                readImage ((buf[8] & 0x40) != 0, transparent);
            }

            break;
        }
    }

    Image image;

private:
    InputStream& input;
    u8 buffer [260];
    PixelARGB palette [256];
    b8 dataBlockIsZero, fresh, finished;
    i32 currentBit, lastBit, lastByteIndex;
    i32 codeSize, setCodeSize;
    i32 maxCode, maxCodeSize;
    i32 firstcode, oldcode;
    i32 clearCode, endCode;
    enum { maxGifCode = 1 << 12 };
    i32 table [2] [maxGifCode];
    i32 stack [2 * maxGifCode];
    i32* sp;

    b8 getSizeFromHeader (i32& w, i32& h)
    {
        // Add an extra byte for the zero terminator
        t8 b[7]{};

        if (input.read (b, 6) == 6
             && (strncmp ("GIF87a", b, 6) == 0
                  || strncmp ("GIF89a", b, 6) == 0))
        {
            if (input.read (b, 4) == 4)
            {
                w = (i32) ByteOrder::littleEndianShort (b);
                h = (i32) ByteOrder::littleEndianShort (b + 2);
                return w > 0 && h > 0;
            }
        }

        return false;
    }

    b8 readPalette (i32k numCols)
    {
        for (i32 i = 0; i < numCols; ++i)
        {
            u8 rgb[4];
            input.read (rgb, 3);

            palette[i].setARGB (0xff, rgb[0], rgb[1], rgb[2]);
            palette[i].premultiply();
        }

        return true;
    }

    i32 readDataBlock (u8* const dest)
    {
        u8 n;
        if (input.read (&n, 1) == 1)
        {
            dataBlockIsZero = (n == 0);

            if (dataBlockIsZero || (input.read (dest, n) == n))
                return n;
        }

        return -1;
    }

    i32 readExtension (i32& transparent)
    {
        u8 type;
        if (input.read (&type, 1) != 1)
            return false;

        u8 b [260];
        i32 n = 0;

        if (type == 0xf9)
        {
            n = readDataBlock (b);
            if (n < 0)
                return 1;

            if ((b[0] & 1) != 0)
                transparent = b[3];
        }

        do
        {
            n = readDataBlock (b);
        }
        while (n > 0);

        return n >= 0;
    }

    z0 clearTable()
    {
        i32 i;
        for (i = 0; i < clearCode; ++i)
        {
            table[0][i] = 0;
            table[1][i] = i;
        }

        for (; i < maxGifCode; ++i)
        {
            table[0][i] = 0;
            table[1][i] = 0;
        }
    }

    z0 initialise (i32k inputCodeSize)
    {
        setCodeSize = inputCodeSize;
        codeSize = setCodeSize + 1;
        clearCode = 1 << setCodeSize;
        endCode = clearCode + 1;
        maxCodeSize = 2 * clearCode;
        maxCode = clearCode + 2;

        getCode (0, true);

        fresh = true;
        clearTable();
        sp = stack;
    }

    i32 readLZWByte()
    {
        if (fresh)
        {
            fresh = false;

            for (;;)
            {
                firstcode = oldcode = getCode (codeSize, false);

                if (firstcode != clearCode)
                    return firstcode;
            }
        }

        if (sp > stack)
            return *--sp;

        i32 code;

        while ((code = getCode (codeSize, false)) >= 0)
        {
            if (code == clearCode)
            {
                clearTable();
                codeSize = setCodeSize + 1;
                maxCodeSize = 2 * clearCode;
                maxCode = clearCode + 2;
                sp = stack;
                firstcode = oldcode = getCode (codeSize, false);
                return firstcode;
            }
            else if (code == endCode)
            {
                if (dataBlockIsZero)
                    return -2;

                u8 buf [260];
                i32 n;

                while ((n = readDataBlock (buf)) > 0)
                {}

                if (n != 0)
                    return -2;
            }

            i32k incode = code;

            if (code >= maxCode)
            {
                *sp++ = firstcode;
                code = oldcode;
            }

            while (code >= clearCode)
            {
                *sp++ = table[1][code];
                if (code == table[0][code])
                    return -2;

                code = table[0][code];
            }

            *sp++ = firstcode = table[1][code];

            if ((code = maxCode) < maxGifCode)
            {
                table[0][code] = oldcode;
                table[1][code] = firstcode;
                ++maxCode;

                if (maxCode >= maxCodeSize && maxCodeSize < maxGifCode)
                {
                    maxCodeSize <<= 1;
                    ++codeSize;
                }
            }

            oldcode = incode;

            if (sp > stack)
                return *--sp;
        }

        return code;
    }

    i32 getCode (i32k codeSize_, const b8 shouldInitialise)
    {
        if (shouldInitialise)
        {
            currentBit = 0;
            lastBit = 0;
            finished = false;
            return 0;
        }

        if ((currentBit + codeSize_) >= lastBit)
        {
            if (finished)
                return -1;

            buffer[0] = buffer [jmax (0, lastByteIndex - 2)];
            buffer[1] = buffer [jmax (0, lastByteIndex - 1)];

            i32k n = readDataBlock (buffer + 2);

            if (n == 0)
                finished = true;

            lastByteIndex = 2 + n;
            currentBit = (currentBit - lastBit) + 16;
            lastBit = (2 + n) * 8 ;
        }

        i32 result = 0;
        i32 i = currentBit;

        for (i32 j = 0; j < codeSize_; ++j)
        {
            result |= ((buffer[i >> 3] & (1 << (i & 7))) != 0) << j;
            ++i;
        }

        currentBit += codeSize_;
        return result;
    }

    b8 readImage (i32k interlace, i32k transparent)
    {
        u8 c;
        if (input.read (&c, 1) != 1)
            return false;

        initialise (c);

        if (transparent >= 0)
            palette [transparent].setARGB (0, 0, 0, 0);

        i32 xpos = 0, ypos = 0, yStep = 8, pass = 0;

        const Image::BitmapData destData (image, Image::BitmapData::writeOnly);
        u8* p = destData.getPixelPointer (0, 0);
        const b8 hasAlpha = image.hasAlphaChannel();

        for (;;)
        {
            i32k index = readLZWByte();
            if (index < 0)
                break;

            if (hasAlpha)
                ((PixelARGB*) p)->set (palette [index]);
            else
                ((PixelRGB*)  p)->set (palette [index]);

            p += destData.pixelStride;

            if (++xpos == destData.width)
            {
                xpos = 0;

                if (interlace)
                {
                    ypos += yStep;

                    while (ypos >= destData.height)
                    {
                        switch (++pass)
                        {
                            case 1:     ypos = 4; yStep = 8; break;
                            case 2:     ypos = 2; yStep = 4; break;
                            case 3:     ypos = 1; yStep = 2; break;
                            default:    return true;
                        }
                    }
                }
                else
                {
                    if (++ypos >= destData.height)
                        break;
                }

                p = destData.getPixelPointer (xpos, ypos);
            }
        }

        return true;
    }

    DRX_DECLARE_NON_COPYABLE (GIFLoader)
};

DRX_END_IGNORE_WARNINGS_MSVC

#endif

//==============================================================================
GIFImageFormat::GIFImageFormat() {}
GIFImageFormat::~GIFImageFormat() {}

Txt GIFImageFormat::getFormatName()                  { return "GIF"; }
b8 GIFImageFormat::usesFileExtension (const File& f)  { return f.hasFileExtension ("gif"); }

b8 GIFImageFormat::canUnderstand (InputStream& in)
{
    t8 header [4];

    return (in.read (header, sizeof (header)) == (i32) sizeof (header))
             && header[0] == 'G'
             && header[1] == 'I'
             && header[2] == 'F';
}

Image GIFImageFormat::decodeImage (InputStream& in)
{
   #if (DRX_MAC || DRX_IOS) && USE_COREGRAPHICS_RENDERING && DRX_USE_COREIMAGE_LOADER
    return drx_loadWithCoreImage (in);
   #else
    const std::unique_ptr<GIFLoader> loader (new GIFLoader (in));
    return loader->image;
   #endif
}

b8 GIFImageFormat::writeImageToStream (const Image& /*sourceImage*/, OutputStream& /*destStream*/)
{
    jassertfalse; // writing isn't implemented for GIFs!
    return false;
}

} // namespace drx
