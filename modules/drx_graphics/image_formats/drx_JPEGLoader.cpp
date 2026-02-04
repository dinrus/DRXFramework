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

DRX_BEGIN_IGNORE_WARNINGS_MSVC (4365 6240 6326 6386 6385 28182 28183 6387 6011 6001)

namespace jpeglibNamespace
{
#if DRX_INCLUDE_JPEGLIB_CODE || ! defined (DRX_INCLUDE_JPEGLIB_CODE)
     DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wconversion",
                                          "-Wdeprecated-register",
                                          "-Wdeprecated-declarations",
                                          "-Wsign-conversion",
                                          "-Wcast-align",
                                          "-Wswitch-enum",
                                          "-Wswitch-default",
                                          "-Wimplicit-fallthrough",
                                          "-Wzero-as-null-pointer-constant",
                                          "-Wshift-negative-value",
                                          "-Wcomma")

    #define JPEG_INTERNALS
    #undef FAR
    #include "jpglib/jpeglib.h"

    #include "jpglib/jcapimin.c"
    #include "jpglib/jcapistd.c"
    #include "jpglib/jccoefct.c"
    #include "jpglib/jccolor.c"
    #undef FIX
    #include "jpglib/jcdctmgr.c"
    #undef CONST_BITS
    #include "jpglib/jchuff.c"
    #undef emit_byte
    #include "jpglib/jcinit.c"
    #include "jpglib/jcmainct.c"
    #include "jpglib/jcmarker.c"
    #include "jpglib/jcmaster.c"
    #include "jpglib/jcomapi.c"
    #include "jpglib/jcparam.c"
    #include "jpglib/jcphuff.c"
    #include "jpglib/jcprepct.c"
    #include "jpglib/jcsample.c"
    #include "jpglib/jctrans.c"
    #include "jpglib/jdapistd.c"
    #include "jpglib/jdapimin.c"
    #include "jpglib/jdatasrc.c"
    #include "jpglib/jdcoefct.c"
    #undef FIX
    #include "jpglib/jdcolor.c"
    #undef FIX
    #include "jpglib/jddctmgr.c"
    #undef CONST_BITS
    #undef ASSIGN_STATE
    #include "jpglib/jdhuff.c"
    #include "jpglib/jdinput.c"
    #include "jpglib/jdmainct.c"
    #include "jpglib/jdmarker.c"
    #include "jpglib/jdmaster.c"
    #undef FIX
    #include "jpglib/jdmerge.c"
    #undef ASSIGN_STATE
    #include "jpglib/jdphuff.c"
    #include "jpglib/jdpostct.c"
    #undef FIX
    #include "jpglib/jdsample.c"
    #include "jpglib/jdtrans.c"
    #include "jpglib/jfdctflt.c"
    #include "jpglib/jfdctint.c"
    #undef CONST_BITS
    #undef MULTIPLY
    #undef FIX_0_541196100
    #include "jpglib/jfdctfst.c"
    #undef FIX_0_541196100
    #include "jpglib/jidctflt.c"
    #undef CONST_BITS
    #undef FIX_1_847759065
    #undef MULTIPLY
    #undef DEQUANTIZE
    #undef DESCALE
    #include "jpglib/jidctfst.c"
    #undef CONST_BITS
    #undef FIX_1_847759065
    #undef MULTIPLY
    #undef DEQUANTIZE
    #include "jpglib/jidctint.c"
    #include "jpglib/jidctred.c"
    #include "jpglib/jmemmgr.c"
    #include "jpglib/jmemnobs.c"
    #include "jpglib/jquant1.c"
    #include "jpglib/jquant2.c"
    #include "jpglib/jutils.c"
    #include "jpglib/transupp.c"

    DRX_END_IGNORE_WARNINGS_GCC_LIKE
#else
    #define JPEG_INTERNALS
    #undef FAR
    #include <jpeglib.h>
#endif
}

#undef max
#undef min

DRX_END_IGNORE_WARNINGS_MSVC

//==============================================================================
namespace JPEGHelpers
{
    using namespace jpeglibNamespace;

   #if ! (DRX_WINDOWS && (DRX_MSVC || DRX_CLANG))
    using jpeglibNamespace::boolean;
   #endif

    static z0 fatalErrorHandler (j_common_ptr p)          { *((b8*) (p->client_data)) = true; }
    static z0 silentErrorCallback1 (j_common_ptr)         {}
    static z0 silentErrorCallback2 (j_common_ptr, i32)    {}
    static z0 silentErrorCallback3 (j_common_ptr, tuk)  {}

    static z0 setupSilentErrorHandler (struct jpeg_error_mgr& err)
    {
        zerostruct (err);

        err.error_exit      = fatalErrorHandler;
        err.emit_message    = silentErrorCallback2;
        err.output_message  = silentErrorCallback1;
        err.format_message  = silentErrorCallback3;
        err.reset_error_mgr = silentErrorCallback1;
    }

    //==============================================================================
   #if ! DRX_USING_COREIMAGE_LOADER
    static z0 dummyCallback1 (j_decompress_ptr) {}

    static z0 jpegSkip (j_decompress_ptr decompStruct, i64 num)
    {
        decompStruct->src->next_input_byte += num;

        num = jmin (num, (i64) decompStruct->src->bytes_in_buffer);
        decompStruct->src->bytes_in_buffer -= (size_t) num;
    }

    static boolean jpegFill (j_decompress_ptr)
    {
        return 0;
    }
   #endif

    //==============================================================================
    i32k jpegBufferSize = 512;

    struct DrxJpegDest final : public jpeg_destination_mgr
    {
        OutputStream* output;
        tuk buffer;
    };

    static z0 jpegWriteInit (j_compress_ptr) {}

    static z0 jpegWriteTerminate (j_compress_ptr cinfo)
    {
        DrxJpegDest* const dest = static_cast<DrxJpegDest*> (cinfo->dest);

        const size_t numToWrite = jpegBufferSize - dest->free_in_buffer;
        dest->output->write (dest->buffer, numToWrite);
    }

    static boolean jpegWriteFlush (j_compress_ptr cinfo)
    {
        DrxJpegDest* const dest = static_cast<DrxJpegDest*> (cinfo->dest);

        i32k numToWrite = jpegBufferSize;

        dest->next_output_byte = reinterpret_cast<JOCTET*> (dest->buffer);
        dest->free_in_buffer = jpegBufferSize;

        return (boolean) dest->output->write (dest->buffer, (size_t) numToWrite);
    }
}

//==============================================================================
JPEGImageFormat::JPEGImageFormat()
    : quality (-1.0f)
{
}

JPEGImageFormat::~JPEGImageFormat() {}

z0 JPEGImageFormat::setQuality (const f32 newQuality)
{
    quality = newQuality;
}

Txt JPEGImageFormat::getFormatName()                   { return "JPEG"; }
b8 JPEGImageFormat::usesFileExtension (const File& f)   { return f.hasFileExtension ("jpeg;jpg"); }

b8 JPEGImageFormat::canUnderstand (InputStream& in)
{
    i32k bytesNeeded = 24;
    u8 header [bytesNeeded];

    if (in.read (header, bytesNeeded) == bytesNeeded
            && header[0] == 0xff
            && header[1] == 0xd8
            && header[2] == 0xff)
        return true;

   #if DRX_USING_COREIMAGE_LOADER
    return header[20] == 'j'
        && header[21] == 'p'
        && header[22] == '2'
        && header[23] == ' ';
   #endif

    return false;
}

#if DRX_USING_COREIMAGE_LOADER
 Image drx_loadWithCoreImage (InputStream& input);
#endif

Image JPEGImageFormat::decodeImage (InputStream& in)
{
   #if DRX_USING_COREIMAGE_LOADER
    return drx_loadWithCoreImage (in);
   #else
    using namespace jpeglibNamespace;
    using namespace JPEGHelpers;

    MemoryOutputStream mb;
    mb << in;

    Image image;

    if (mb.getDataSize() > 16)
    {
        struct jpeg_decompress_struct jpegDecompStruct;

        struct jpeg_error_mgr jerr;
        setupSilentErrorHandler (jerr);
        jpegDecompStruct.err = &jerr;

        jpeg_create_decompress (&jpegDecompStruct);

        jpegDecompStruct.src = (jpeg_source_mgr*)(jpegDecompStruct.mem->alloc_small)
            ((j_common_ptr)(&jpegDecompStruct), JPOOL_PERMANENT, sizeof (jpeg_source_mgr));

        b8 hasFailed = false;
        jpegDecompStruct.client_data = &hasFailed;

        jpegDecompStruct.src->init_source       = dummyCallback1;
        jpegDecompStruct.src->fill_input_buffer = jpegFill;
        jpegDecompStruct.src->skip_input_data   = jpegSkip;
        jpegDecompStruct.src->resync_to_restart = jpeg_resync_to_restart;
        jpegDecompStruct.src->term_source       = dummyCallback1;

        jpegDecompStruct.src->next_input_byte   = static_cast<u8k*> (mb.getData());
        jpegDecompStruct.src->bytes_in_buffer   = mb.getDataSize();

        jpeg_read_header (&jpegDecompStruct, TRUE);

        if (! hasFailed)
        {
            jpeg_calc_output_dimensions (&jpegDecompStruct);

            if (! hasFailed)
            {
                i32k width  = (i32) jpegDecompStruct.output_width;
                i32k height = (i32) jpegDecompStruct.output_height;

                jpegDecompStruct.out_color_space = JCS_RGB;

                JSAMPARRAY buffer
                    = (*jpegDecompStruct.mem->alloc_sarray) ((j_common_ptr) &jpegDecompStruct,
                                                             JPOOL_IMAGE,
                                                             (JDIMENSION) width * 3, 1);

                if (jpeg_start_decompress (&jpegDecompStruct) && ! hasFailed)
                {
                    image = Image (Image::RGB, width, height, false);
                    image.getProperties()->set ("originalImageHadAlpha", false);
                    const b8 hasAlphaChan = image.hasAlphaChannel(); // (the native image creator may not give back what we expect)

                    const Image::BitmapData destData (image, Image::BitmapData::writeOnly);

                    for (i32 y = 0; y < height; ++y)
                    {
                        jpeg_read_scanlines (&jpegDecompStruct, buffer, 1);

                        if (hasFailed)
                            break;

                        u8k* src = *buffer;
                        u8* dest = destData.getLinePointer (y);

                        if (hasAlphaChan)
                        {
                            for (i32 i = width; --i >= 0;)
                            {
                                ((PixelARGB*) dest)->setARGB (0xff, src[0], src[1], src[2]);
                                ((PixelARGB*) dest)->premultiply();
                                dest += destData.pixelStride;
                                src += 3;
                            }
                        }
                        else
                        {
                            for (i32 i = width; --i >= 0;)
                            {
                                ((PixelRGB*) dest)->setARGB (0xff, src[0], src[1], src[2]);
                                dest += destData.pixelStride;
                                src += 3;
                            }
                        }
                    }

                    if (! hasFailed)
                        jpeg_finish_decompress (&jpegDecompStruct);

                    in.setPosition (((tuk) jpegDecompStruct.src->next_input_byte) - (tuk) mb.getData());
                }
            }
        }

        jpeg_destroy_decompress (&jpegDecompStruct);
    }

    return image;
   #endif
}

b8 JPEGImageFormat::writeImageToStream (const Image& image, OutputStream& out)
{
    using namespace jpeglibNamespace;
    using namespace JPEGHelpers;

    jpeg_compress_struct jpegCompStruct;
    zerostruct (jpegCompStruct);
    jpeg_create_compress (&jpegCompStruct);

    struct jpeg_error_mgr jerr;
    setupSilentErrorHandler (jerr);
    jpegCompStruct.err = &jerr;

    DrxJpegDest dest;
    jpegCompStruct.dest = &dest;

    dest.output = &out;
    HeapBlock<t8> tempBuffer (jpegBufferSize);
    dest.buffer = tempBuffer;
    dest.next_output_byte = (JOCTET*) dest.buffer;
    dest.free_in_buffer = jpegBufferSize;
    dest.init_destination = jpegWriteInit;
    dest.empty_output_buffer = jpegWriteFlush;
    dest.term_destination = jpegWriteTerminate;

    jpegCompStruct.image_width  = (JDIMENSION) image.getWidth();
    jpegCompStruct.image_height = (JDIMENSION) image.getHeight();
    jpegCompStruct.input_components = 3;
    jpegCompStruct.in_color_space = JCS_RGB;
    jpegCompStruct.write_JFIF_header = 1;

    jpegCompStruct.X_density = 72;
    jpegCompStruct.Y_density = 72;

    jpeg_set_defaults (&jpegCompStruct);

    jpegCompStruct.dct_method = JDCT_FLOAT;
    jpegCompStruct.optimize_coding = 1;

    if (quality < 0.0f)
        quality = 0.85f;

    jpeg_set_quality (&jpegCompStruct, jlimit (0, 100, roundToInt (quality * 100.0f)), TRUE);

    jpeg_start_compress (&jpegCompStruct, TRUE);

    i32k strideBytes = (i32) (jpegCompStruct.image_width * (u32) jpegCompStruct.input_components);

    JSAMPARRAY buffer = (*jpegCompStruct.mem->alloc_sarray) ((j_common_ptr) &jpegCompStruct,
                                                             JPOOL_IMAGE, (JDIMENSION) strideBytes, 1);

    const Image::BitmapData srcData (image, Image::BitmapData::readOnly);

    while (jpegCompStruct.next_scanline < jpegCompStruct.image_height)
    {
        u8* dst = *buffer;

        if (srcData.pixelFormat == Image::RGB)
        {
            u8k* src = srcData.getLinePointer ((i32) jpegCompStruct.next_scanline);

            for (i32 i = srcData.width; --i >= 0;)
            {
                *dst++ = ((const PixelRGB*) src)->getRed();
                *dst++ = ((const PixelRGB*) src)->getGreen();
                *dst++ = ((const PixelRGB*) src)->getBlue();
                src += srcData.pixelStride;
            }
        }
        else
        {
            for (i32 x = 0; x < srcData.width; ++x)
            {
                const Color pixel (srcData.getPixelColor (x, (i32) jpegCompStruct.next_scanline));
                *dst++ = pixel.getRed();
                *dst++ = pixel.getGreen();
                *dst++ = pixel.getBlue();
            }
        }

        jpeg_write_scanlines (&jpegCompStruct, buffer, 1);
    }

    jpeg_finish_compress (&jpegCompStruct);
    jpeg_destroy_compress (&jpegCompStruct);

    return true;
}

} // namespace drx
