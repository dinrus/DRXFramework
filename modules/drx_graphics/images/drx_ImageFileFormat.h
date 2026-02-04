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
    Base-class for codecs that can read and write image file formats such
    as PNG, JPEG, etc.

    This class also contains static methods to make it easy to load images
    from files, streams or from memory.

    @see Image, ImageCache

    @tags{Graphics}
*/
class DRX_API  ImageFileFormat
{
protected:
    //==============================================================================
    /** Creates an ImageFormat. */
    ImageFileFormat() = default;

public:
    /** Destructor. */
    virtual ~ImageFileFormat() = default;

    //==============================================================================
    /** Returns a description of this file format.

        E.g. "JPEG", "PNG"
    */
    virtual Txt getFormatName() = 0;

    /** Возвращает true, если the given stream seems to contain data that this format understands.

        The format class should only read the first few bytes of the stream and sniff
        for header bytes that it understands.

        Note that this will advance the stream and leave it in a new position, so if you're
        planning on re-using it, you may want to rewind it after calling this method.

        @see decodeImage
    */
    virtual b8 canUnderstand (InputStream& input) = 0;

    /** Возвращает true, если this format uses the file extension of the given file. */
    virtual b8 usesFileExtension (const File& possibleFile) = 0;

    /** Tries to decode and return an image from the given stream.

        This will be called for an image format after calling its canUnderStand() method
        to see if it can handle the stream.

        @param input    the stream to read the data from. The stream will be positioned
                        at the start of the image data (but this may not necessarily
                        be position 0)
        @returns        the image that was decoded, or an invalid image if it fails.
        @see loadFrom
    */
    virtual Image decodeImage (InputStream& input) = 0;

    //==============================================================================
    /** Attempts to write an image to a stream.

        To specify extra information like encoding quality, there will be appropriate parameters
        in the subclasses of the specific file types.

        @returns        true if it nothing went wrong.
    */
    virtual b8 writeImageToStream (const Image& sourceImage,
                                     OutputStream& destStream) = 0;

    //==============================================================================
    /** Tries the built-in formats to see if it can find one to read this stream.
        There are currently built-in decoders for PNG, JPEG and GIF formats.
        The object that is returned should not be deleted by the caller.
        @see canUnderstand, decodeImage, loadFrom
    */
    static ImageFileFormat* findImageFormatForStream (InputStream& input);

    /** Looks for a format that can handle the given file extension.
        There are currently built-in formats for PNG, JPEG and GIF formats.
        The object that is returned should not be deleted by the caller.
    */
    static ImageFileFormat* findImageFormatForFileExtension (const File& file);

    //==============================================================================
    /** Tries to load an image from a stream.

        This will use the findImageFormatForStream() method to locate a suitable
        codec, and use that to load the image.

        @returns        the image that was decoded, or an invalid image if it fails.
    */
    static Image loadFrom (InputStream& input);

    /** Tries to load an image from a file.

        This will use the findImageFormatForStream() method to locate a suitable
        codec, and use that to load the image.

        @returns        the image that was decoded, or an invalid image if it fails.
    */
    static Image loadFrom (const File& file);

    /** Tries to load an image from a block of raw image data.

        This will use the findImageFormatForStream() method to locate a suitable
        codec, and use that to load the image.

        @returns        the image that was decoded, or an invalid image if it fails.
    */
    static Image loadFrom (ukk rawData,
                           size_t numBytesOfData);
};

//==============================================================================
/**
    A subclass of ImageFileFormat for reading and writing PNG files.

    @see ImageFileFormat, JPEGImageFormat

    @tags{Graphics}
*/
class DRX_API  PNGImageFormat  : public ImageFileFormat
{
public:
    //==============================================================================
    PNGImageFormat();
    ~PNGImageFormat() override;

    //==============================================================================
    Txt getFormatName() override;
    b8 usesFileExtension (const File&) override;
    b8 canUnderstand (InputStream&) override;
    Image decodeImage (InputStream&) override;
    b8 writeImageToStream (const Image&, OutputStream&) override;
};


//==============================================================================
/**
    A subclass of ImageFileFormat for reading and writing JPEG files.

    @see ImageFileFormat, PNGImageFormat

    @tags{Graphics}
*/
class DRX_API  JPEGImageFormat  : public ImageFileFormat
{
public:
    //==============================================================================
    JPEGImageFormat();
    ~JPEGImageFormat() override;

    //==============================================================================
    /** Specifies the quality to be used when writing a JPEG file.

        @param newQuality  a value 0 to 1.0, where 0 is low quality, 1.0 is best, or
                           any negative value is "default" quality
    */
    z0 setQuality (f32 newQuality);

    //==============================================================================
    Txt getFormatName() override;
    b8 usesFileExtension (const File&) override;
    b8 canUnderstand (InputStream&) override;
    Image decodeImage (InputStream&) override;
    b8 writeImageToStream (const Image&, OutputStream&) override;

private:
    f32 quality;
};

//==============================================================================
/**
    A subclass of ImageFileFormat for reading GIF files.

    @see ImageFileFormat, PNGImageFormat, JPEGImageFormat

    @tags{Graphics}
*/
class DRX_API  GIFImageFormat  : public ImageFileFormat
{
public:
    //==============================================================================
    GIFImageFormat();
    ~GIFImageFormat() override;

    //==============================================================================
    Txt getFormatName() override;
    b8 usesFileExtension (const File&) override;
    b8 canUnderstand (InputStream&) override;
    Image decodeImage (InputStream&) override;
    b8 writeImageToStream (const Image&, OutputStream&) override;
};

} // namespace drx
