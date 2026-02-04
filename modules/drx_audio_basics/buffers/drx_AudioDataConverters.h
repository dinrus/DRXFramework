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
    This class a container which holds all the classes pertaining to the AudioData::Pointer
    audio sample format class.

    @see AudioData::Pointer.

    @tags{Audio}
*/
class DRX_API  AudioData
{
public:
    //==============================================================================
    // These types can be used as the SampleFormat template parameter for the AudioData::Pointer class.

    class Int8;       /**< Used as a template parameter for AudioData::Pointer. Indicates an 8-bit integer packed data format. */
    class UInt8;      /**< Used as a template parameter for AudioData::Pointer. Indicates an 8-bit u32 integer packed data format. */
    class Int16;      /**< Used as a template parameter for AudioData::Pointer. Indicates an 16-bit integer packed data format. */
    class Int24;      /**< Used as a template parameter for AudioData::Pointer. Indicates an 24-bit integer packed data format. */
    class Int32;      /**< Used as a template parameter for AudioData::Pointer. Indicates an 32-bit integer packed data format. */
    class Float32;    /**< Used as a template parameter for AudioData::Pointer. Indicates an 32-bit f32 data format. */

    //==============================================================================
    // These types can be used as the Endianness template parameter for the AudioData::Pointer class.

    class BigEndian;      /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are stored in big-endian order. */
    class LittleEndian;   /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are stored in little-endian order. */
    class NativeEndian;   /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are stored in the CPU's native endianness. */

    //==============================================================================
    // These types can be used as the InterleavingType template parameter for the AudioData::Pointer class.

    class NonInterleaved; /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are stored contiguously. */
    class Interleaved;    /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples are interleaved with a number of other channels. */

    //==============================================================================
    // These types can be used as the Constness template parameter for the AudioData::Pointer class.

    class NonConst; /**< Used as a template parameter for AudioData::Pointer. Indicates that the pointer can be used for non-const data. */
    class Const;    /**< Used as a template parameter for AudioData::Pointer. Indicates that the samples can only be used for const data.. */

  #ifndef DOXYGEN
    //==============================================================================
    class BigEndian
    {
    public:
        template <class SampleFormatType> static f32 getAsFloat (SampleFormatType& s) noexcept                         { return s.getAsFloatBE(); }
        template <class SampleFormatType> static z0  setAsFloat (SampleFormatType& s, f32 newValue) noexcept         { s.setAsFloatBE (newValue); }
        template <class SampleFormatType> static i32 getAsInt32 (SampleFormatType& s) noexcept                         { return s.getAsInt32BE(); }
        template <class SampleFormatType> static z0  setAsInt32 (SampleFormatType& s, i32 newValue) noexcept         { s.setAsInt32BE (newValue); }
        template <class SourceType, class DestType> static z0 copyFrom (DestType& dest, SourceType& source) noexcept   { dest.copyFromBE (source); }
        enum { isBigEndian = 1 };
    };

    class LittleEndian
    {
    public:
        template <class SampleFormatType> static f32 getAsFloat (SampleFormatType& s) noexcept                         { return s.getAsFloatLE(); }
        template <class SampleFormatType> static z0  setAsFloat (SampleFormatType& s, f32 newValue) noexcept         { s.setAsFloatLE (newValue); }
        template <class SampleFormatType> static i32 getAsInt32 (SampleFormatType& s) noexcept                         { return s.getAsInt32LE(); }
        template <class SampleFormatType> static z0  setAsInt32 (SampleFormatType& s, i32 newValue) noexcept         { s.setAsInt32LE (newValue); }
        template <class SourceType, class DestType> static z0 copyFrom (DestType& dest, SourceType& source) noexcept   { dest.copyFromLE (source); }
        enum { isBigEndian = 0 };
    };

    #if DRX_BIG_ENDIAN
     class NativeEndian   : public BigEndian  {};
    #else
     class NativeEndian   : public LittleEndian  {};
    #endif

    //==============================================================================
    class Int8
    {
    public:
        inline Int8 (uk d) noexcept  : data (static_cast<i8*> (d))  {}

        inline z0 advance() noexcept                          { ++data; }
        inline z0 skip (i32 numSamples) noexcept              { data += numSamples; }
        inline f32 getAsFloatLE() const noexcept              { return (f32) (*data * (1.0 / (1.0 + (f64) maxValue))); }
        inline f32 getAsFloatBE() const noexcept              { return getAsFloatLE(); }
        inline z0 setAsFloatLE (f32 newValue) noexcept      { *data = (i8) jlimit ((i32) -maxValue, (i32) maxValue, roundToInt (newValue * (1.0 + (f64) maxValue))); }
        inline z0 setAsFloatBE (f32 newValue) noexcept      { setAsFloatLE (newValue); }
        inline i32 getAsInt32LE() const noexcept              { return (i32) (*((u8*) data) << 24); }
        inline i32 getAsInt32BE() const noexcept              { return getAsInt32LE(); }
        inline z0 setAsInt32LE (i32 newValue) noexcept        { *data = (i8) (newValue >> 24); }
        inline z0 setAsInt32BE (i32 newValue) noexcept        { setAsInt32LE (newValue); }
        inline z0 clear() noexcept                            { *data = 0; }
        inline z0 clearMultiple (i32 num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline z0 copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline z0 copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline z0 copyFromSameType (Int8& source) noexcept    { *data = *source.data; }

        i8* data;
        enum { bytesPerSample = 1, maxValue = 0x7f, resolution = (1 << 24), isFloat = 0 };
    };

    class UInt8
    {
    public:
        inline UInt8 (uk d) noexcept  : data (static_cast<u8*> (d))  {}

        inline z0 advance() noexcept                          { ++data; }
        inline z0 skip (i32 numSamples) noexcept              { data += numSamples; }
        inline f32 getAsFloatLE() const noexcept              { return (f32) ((*data - 128) * (1.0 / (1.0 + (f64) maxValue))); }
        inline f32 getAsFloatBE() const noexcept              { return getAsFloatLE(); }
        inline z0 setAsFloatLE (f32 newValue) noexcept      { *data = (u8) jlimit (0, 255, 128 + roundToInt (newValue * (1.0 + (f64) maxValue))); }
        inline z0 setAsFloatBE (f32 newValue) noexcept      { setAsFloatLE (newValue); }
        inline i32 getAsInt32LE() const noexcept              { return (i32) (((u8) (*data - 128)) << 24); }
        inline i32 getAsInt32BE() const noexcept              { return getAsInt32LE(); }
        inline z0 setAsInt32LE (i32 newValue) noexcept        { *data = (u8) (128 + (newValue >> 24)); }
        inline z0 setAsInt32BE (i32 newValue) noexcept        { setAsInt32LE (newValue); }
        inline z0 clear() noexcept                            { *data = 128; }
        inline z0 clearMultiple (i32 num) noexcept            { memset (data, 128, (size_t) num) ;}
        template <class SourceType> inline z0 copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline z0 copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline z0 copyFromSameType (UInt8& source) noexcept   { *data = *source.data; }

        u8* data;
        enum { bytesPerSample = 1, maxValue = 0x7f, resolution = (1 << 24), isFloat = 0 };
    };

    class Int16
    {
    public:
        inline Int16 (uk d) noexcept  : data (static_cast<u16*> (d))  {}

        inline z0 advance() noexcept                          { ++data; }
        inline z0 skip (i32 numSamples) noexcept              { data += numSamples; }
        inline f32 getAsFloatLE() const noexcept              { return (f32) ((1.0 / (1.0 + (f64) maxValue)) * (i16) ByteOrder::swapIfBigEndian    (*data)); }
        inline f32 getAsFloatBE() const noexcept              { return (f32) ((1.0 / (1.0 + (f64) maxValue)) * (i16) ByteOrder::swapIfLittleEndian (*data)); }
        inline z0 setAsFloatLE (f32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((u16) jlimit ((i32) -maxValue, (i32) maxValue, roundToInt (newValue * (1.0 + (f64) maxValue)))); }
        inline z0 setAsFloatBE (f32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((u16) jlimit ((i32) -maxValue, (i32) maxValue, roundToInt (newValue * (1.0 + (f64) maxValue)))); }
        inline i32 getAsInt32LE() const noexcept              { return (i32) (ByteOrder::swapIfBigEndian    ((u16) *data) << 16); }
        inline i32 getAsInt32BE() const noexcept              { return (i32) (ByteOrder::swapIfLittleEndian ((u16) *data) << 16); }
        inline z0 setAsInt32LE (i32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((u16) (newValue >> 16)); }
        inline z0 setAsInt32BE (i32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((u16) (newValue >> 16)); }
        inline z0 clear() noexcept                            { *data = 0; }
        inline z0 clearMultiple (i32 num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline z0 copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline z0 copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline z0 copyFromSameType (Int16& source) noexcept   { *data = *source.data; }

        u16* data;
        enum { bytesPerSample = 2, maxValue = 0x7fff, resolution = (1 << 16), isFloat = 0 };
    };

    class Int24
    {
    public:
        inline Int24 (uk d) noexcept  : data (static_cast<tuk> (d))  {}

        inline z0 advance() noexcept                          { data += 3; }
        inline z0 skip (i32 numSamples) noexcept              { data += 3 * numSamples; }
        inline f32 getAsFloatLE() const noexcept              { return (f32) (ByteOrder::littleEndian24Bit (data) * (1.0 / (1.0 + (f64) maxValue))); }
        inline f32 getAsFloatBE() const noexcept              { return (f32) (ByteOrder::bigEndian24Bit    (data) * (1.0 / (1.0 + (f64) maxValue))); }
        inline z0 setAsFloatLE (f32 newValue) noexcept      { ByteOrder::littleEndian24BitToChars (jlimit ((i32) -maxValue, (i32) maxValue, roundToInt (newValue * (1.0 + (f64) maxValue))), data); }
        inline z0 setAsFloatBE (f32 newValue) noexcept      { ByteOrder::bigEndian24BitToChars (jlimit    ((i32) -maxValue, (i32) maxValue, roundToInt (newValue * (1.0 + (f64) maxValue))), data); }
        inline i32 getAsInt32LE() const noexcept              { return (i32) (((u32) ByteOrder::littleEndian24Bit (data)) << 8); }
        inline i32 getAsInt32BE() const noexcept              { return (i32) (((u32) ByteOrder::bigEndian24Bit    (data)) << 8); }
        inline z0 setAsInt32LE (i32 newValue) noexcept      { ByteOrder::littleEndian24BitToChars (newValue >> 8, data); }
        inline z0 setAsInt32BE (i32 newValue) noexcept      { ByteOrder::bigEndian24BitToChars    (newValue >> 8, data); }
        inline z0 clear() noexcept                            { data[0] = 0; data[1] = 0; data[2] = 0; }
        inline z0 clearMultiple (i32 num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline z0 copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline z0 copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline z0 copyFromSameType (Int24& source) noexcept   { data[0] = source.data[0]; data[1] = source.data[1]; data[2] = source.data[2]; }

        tuk data;
        enum { bytesPerSample = 3, maxValue = 0x7fffff, resolution = (1 << 8), isFloat = 0 };
    };

    class Int32
    {
    public:
        inline Int32 (uk d) noexcept  : data (static_cast<u32*> (d))  {}

        inline z0 advance() noexcept                          { ++data; }
        inline z0 skip (i32 numSamples) noexcept              { data += numSamples; }
        inline f32 getAsFloatLE() const noexcept              { return (f32) ((1.0 / (1.0 + (f64) maxValue)) * (i32) ByteOrder::swapIfBigEndian    (*data)); }
        inline f32 getAsFloatBE() const noexcept              { return (f32) ((1.0 / (1.0 + (f64) maxValue)) * (i32) ByteOrder::swapIfLittleEndian (*data)); }
        inline z0 setAsFloatLE (f32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((u32) (i32) ((f64) maxValue * jlimit (-1.0, 1.0, (f64) newValue))); }
        inline z0 setAsFloatBE (f32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((u32) (i32) ((f64) maxValue * jlimit (-1.0, 1.0, (f64) newValue))); }
        inline i32 getAsInt32LE() const noexcept              { return (i32) ByteOrder::swapIfBigEndian    (*data); }
        inline i32 getAsInt32BE() const noexcept              { return (i32) ByteOrder::swapIfLittleEndian (*data); }
        inline z0 setAsInt32LE (i32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((u32) newValue); }
        inline z0 setAsInt32BE (i32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((u32) newValue); }
        inline z0 clear() noexcept                            { *data = 0; }
        inline z0 clearMultiple (i32 num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline z0 copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline z0 copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline z0 copyFromSameType (Int32& source) noexcept   { *data = *source.data; }

        u32* data;
        enum { bytesPerSample = 4, maxValue = 0x7fffffff, resolution = 1, isFloat = 0 };
    };

    /** A 32-bit integer type, of which only the bottom 24 bits are used. */
    class Int24in32  : public Int32
    {
    public:
        inline Int24in32 (uk d) noexcept  : Int32 (d)  {}

        inline f32 getAsFloatLE() const noexcept              { return (f32) ((1.0 / (1.0 + (f64) maxValue)) * (i32) ByteOrder::swapIfBigEndian (*data)); }
        inline f32 getAsFloatBE() const noexcept              { return (f32) ((1.0 / (1.0 + (f64) maxValue)) * (i32) ByteOrder::swapIfLittleEndian (*data)); }
        inline z0 setAsFloatLE (f32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((u32) ((f64) maxValue * jlimit (-1.0, 1.0, (f64) newValue))); }
        inline z0 setAsFloatBE (f32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((u32) ((f64) maxValue * jlimit (-1.0, 1.0, (f64) newValue))); }
        inline i32 getAsInt32LE() const noexcept              { return (i32) ByteOrder::swapIfBigEndian    (*data) << 8; }
        inline i32 getAsInt32BE() const noexcept              { return (i32) ByteOrder::swapIfLittleEndian (*data) << 8; }
        inline z0 setAsInt32LE (i32 newValue) noexcept      { *data = ByteOrder::swapIfBigEndian    ((u32) newValue >> 8); }
        inline z0 setAsInt32BE (i32 newValue) noexcept      { *data = ByteOrder::swapIfLittleEndian ((u32) newValue >> 8); }
        template <class SourceType> inline z0 copyFromLE (SourceType& source) noexcept    { setAsInt32LE (source.getAsInt32()); }
        template <class SourceType> inline z0 copyFromBE (SourceType& source) noexcept    { setAsInt32BE (source.getAsInt32()); }
        inline z0 copyFromSameType (Int24in32& source) noexcept { *data = *source.data; }

        enum { bytesPerSample = 4, maxValue = 0x7fffff, resolution = (1 << 8), isFloat = 0 };
    };

    class Float32
    {
    public:
        inline Float32 (uk d) noexcept  : data (static_cast<f32*> (d))  {}

        inline z0 advance() noexcept                          { ++data; }
        inline z0 skip (i32 numSamples) noexcept              { data += numSamples; }
       #if DRX_BIG_ENDIAN
        inline f32 getAsFloatBE() const noexcept              { return *data; }
        inline z0 setAsFloatBE (f32 newValue) noexcept      { *data = newValue; }
        inline f32 getAsFloatLE() const noexcept              { union { u32 asInt; f32 asFloat; } n; n.asInt = ByteOrder::swap (*(u32*) data); return n.asFloat; }
        inline z0 setAsFloatLE (f32 newValue) noexcept      { union { u32 asInt; f32 asFloat; } n; n.asFloat = newValue; *(u32*) data = ByteOrder::swap (n.asInt); }
       #else
        inline f32 getAsFloatLE() const noexcept              { return *data; }
        inline z0 setAsFloatLE (f32 newValue) noexcept      { *data = newValue; }
        inline f32 getAsFloatBE() const noexcept              { union { u32 asInt; f32 asFloat; } n; n.asInt = ByteOrder::swap (*(u32*) data); return n.asFloat; }
        inline z0 setAsFloatBE (f32 newValue) noexcept      { union { u32 asInt; f32 asFloat; } n; n.asFloat = newValue; *(u32*) data = ByteOrder::swap (n.asInt); }
       #endif
        inline i32 getAsInt32LE() const noexcept              { return (i32) roundToInt (jlimit (-1.0, 1.0, (f64) getAsFloatLE()) * (f64) maxValue); }
        inline i32 getAsInt32BE() const noexcept              { return (i32) roundToInt (jlimit (-1.0, 1.0, (f64) getAsFloatBE()) * (f64) maxValue); }
        inline z0 setAsInt32LE (i32 newValue) noexcept      { setAsFloatLE ((f32) (newValue * (1.0 / (1.0 + (f64) maxValue)))); }
        inline z0 setAsInt32BE (i32 newValue) noexcept      { setAsFloatBE ((f32) (newValue * (1.0 / (1.0 + (f64) maxValue)))); }
        inline z0 clear() noexcept                            { *data = 0; }
        inline z0 clearMultiple (i32 num) noexcept            { zeromem (data, (size_t) (num * bytesPerSample)) ;}
        template <class SourceType> inline z0 copyFromLE (SourceType& source) noexcept    { setAsFloatLE (source.getAsFloat()); }
        template <class SourceType> inline z0 copyFromBE (SourceType& source) noexcept    { setAsFloatBE (source.getAsFloat()); }
        inline z0 copyFromSameType (Float32& source) noexcept { *data = *source.data; }

        f32* data;
        enum { bytesPerSample = 4, maxValue = 0x7fffffff, resolution = (1 << 8), isFloat = 1 };
    };

    //==============================================================================
    class NonInterleaved
    {
    public:
        inline NonInterleaved() = default;
        inline NonInterleaved (const NonInterleaved&) = default;
        inline NonInterleaved (i32) noexcept {}
        inline z0 copyFrom (const NonInterleaved&) noexcept {}
        template <class SampleFormatType> inline z0 advanceData (SampleFormatType& s) noexcept                    { s.advance(); }
        template <class SampleFormatType> inline z0 advanceDataBy (SampleFormatType& s, i32 numSamples) noexcept  { s.skip (numSamples); }
        template <class SampleFormatType> inline z0 clear (SampleFormatType& s, i32 numSamples) noexcept          { s.clearMultiple (numSamples); }
        template <class SampleFormatType> static i32 getNumBytesBetweenSamples (const SampleFormatType&) noexcept   { return SampleFormatType::bytesPerSample; }

        enum { isInterleavedType = 0, numInterleavedChannels = 1 };
    };

    class Interleaved
    {
    public:
        inline Interleaved() noexcept  {}
        inline Interleaved (const Interleaved& other) = default;
        inline Interleaved (i32k numInterleavedChans) noexcept : numInterleavedChannels (numInterleavedChans) {}
        inline z0 copyFrom (const Interleaved& other) noexcept { numInterleavedChannels = other.numInterleavedChannels; }
        template <class SampleFormatType> inline z0 advanceData (SampleFormatType& s) noexcept                    { s.skip (numInterleavedChannels); }
        template <class SampleFormatType> inline z0 advanceDataBy (SampleFormatType& s, i32 numSamples) noexcept  { s.skip (numInterleavedChannels * numSamples); }
        template <class SampleFormatType> inline z0 clear (SampleFormatType& s, i32 numSamples) noexcept          { while (--numSamples >= 0) { s.clear(); s.skip (numInterleavedChannels); } }
        template <class SampleFormatType> inline i32 getNumBytesBetweenSamples (const SampleFormatType&) const noexcept { return numInterleavedChannels * SampleFormatType::bytesPerSample; }
        i32 numInterleavedChannels = 1;
        enum { isInterleavedType = 1 };
    };

    //==============================================================================
    class NonConst
    {
    public:
        using VoidType = z0;
        static uk toVoidPtr (VoidType* v) noexcept { return v; }
        enum { isConst = 0 };
    };

    class Const
    {
    public:
        using VoidType = const z0;
        static uk toVoidPtr (VoidType* v) noexcept { return const_cast<uk> (v); }
        enum { isConst = 1 };
    };
  #endif

    //==============================================================================
    /**
        A pointer to a block of audio data with a particular encoding.

        This object can be used to read and write from blocks of encoded audio samples. To create one, you specify
        the audio format as a series of template parameters, e.g.
        @code
        // this creates a pointer for reading from a const array of 16-bit little-endian packed samples.
        AudioData::Pointer <AudioData::Int16,
                            AudioData::LittleEndian,
                            AudioData::NonInterleaved,
                            AudioData::Const> pointer (someRawAudioData);

        // These methods read the sample that is being pointed to
        f32 firstSampleAsFloat = pointer.getAsFloat();
        i32 firstSampleAsInt = pointer.getAsInt32();
        ++pointer; // moves the pointer to the next sample.
        pointer += 3; // skips the next 3 samples.
        @endcode

        The convertSamples() method lets you copy a range of samples from one format to another, automatically
        converting its format.

        @see AudioData::Converter
    */
    template <typename SampleFormat,
              typename Endianness,
              typename InterleavingType,
              typename Constness>
    class Pointer  : private InterleavingType  // (inherited for EBCO)
    {
    public:
        //==============================================================================
        /** Creates a non-interleaved pointer from some raw data in the appropriate format.
            This constructor is only used if you've specified the AudioData::NonInterleaved option -
            for interleaved formats, use the constructor that also takes a number of channels.
        */
        Pointer (typename Constness::VoidType* sourceData) noexcept
            : data (Constness::toVoidPtr (sourceData))
        {
            // If you're using interleaved data, call the other constructor! If you're using non-interleaved data,
            // you should pass NonInterleaved as the template parameter for the interleaving type!
            static_assert (InterleavingType::isInterleavedType == 0, "Incorrect constructor for interleaved data");
        }

        /** Creates a pointer from some raw data in the appropriate format with the specified number of interleaved channels.
            For non-interleaved data, use the other constructor.
        */
        Pointer (typename Constness::VoidType* sourceData, i32 numInterleaved) noexcept
            : InterleavingType (numInterleaved), data (Constness::toVoidPtr (sourceData))
        {
        }

        /** Creates a copy of another pointer. */
        Pointer (const Pointer& other) noexcept
            : InterleavingType (other), data (other.data)
        {
        }

        Pointer& operator= (const Pointer& other) noexcept
        {
            InterleavingType::operator= (other);
            data = other.data;
            return *this;
        }

        //==============================================================================
        /** Returns the value of the first sample as a floating point value.
            The value will be in the range -1.0 to 1.0 for integer formats. For floating point
            formats, the value could be outside that range, although -1 to 1 is the standard range.
        */
        inline f32 getAsFloat() const noexcept                { return Endianness::getAsFloat (data); }

        /** Sets the value of the first sample as a floating point value.

            (This method can only be used if the AudioData::NonConst option was used).
            The value should be in the range -1.0 to 1.0 - for integer formats, values outside that
            range will be clipped. For floating point formats, any value passed in here will be
            written directly, although -1 to 1 is the standard range.
        */
        inline z0 setAsFloat (f32 newValue) noexcept
        {
            // trying to write to a const pointer! For a writeable one, use AudioData::NonConst instead!
            static_assert (Constness::isConst == 0, "Attempt to write to a const pointer");
            Endianness::setAsFloat (data, newValue);
        }

        /** Returns the value of the first sample as a 32-bit integer.
            The value returned will be in the range 0x80000000 to 0x7fffffff, and shorter values will be
            shifted to fill this range (e.g. if you're reading from 24-bit data, the values will be shifted up
            by 8 bits when returned here). If the source data is floating point, values beyond -1.0 to 1.0 will
            be clipped so that -1.0 maps onto -0x7fffffff and 1.0 maps to 0x7fffffff.
        */
        inline i32 getAsInt32() const noexcept                { return Endianness::getAsInt32 (data); }

        /** Sets the value of the first sample as a 32-bit integer.
            This will be mapped to the range of the format that is being written - see getAsInt32().
        */
        inline z0 setAsInt32 (i32 newValue) noexcept
        {
             // trying to write to a const pointer! For a writeable one, use AudioData::NonConst instead!
            static_assert (Constness::isConst == 0, "Attempt to write to a const pointer");
            Endianness::setAsInt32 (data, newValue);
        }

        /** Moves the pointer along to the next sample. */
        inline Pointer& operator++() noexcept                   { advance(); return *this; }

        /** Moves the pointer back to the previous sample. */
        inline Pointer& operator--() noexcept                   { this->advanceDataBy (data, -1); return *this; }

        /** Adds a number of samples to the pointer's position. */
        Pointer& operator+= (i32 samplesToJump) noexcept        { this->advanceDataBy (data, samplesToJump); return *this; }

        /** Returns a new pointer with the specified offset from this pointer's position. */
        Pointer operator+ (i32 samplesToJump) const             { return Pointer { *this } += samplesToJump; }

        /** Writes a stream of samples into this pointer from another pointer.
            This will copy the specified number of samples, converting between formats appropriately.
        */
        z0 convertSamples (Pointer source, i32 numSamples) const noexcept
        {
            // trying to write to a const pointer! For a writeable one, use AudioData::NonConst instead!
            static_assert (Constness::isConst == 0, "Attempt to write to a const pointer");

            for (Pointer dest (*this); --numSamples >= 0;)
            {
                dest.data.copyFromSameType (source.data);
                dest.advance();
                source.advance();
            }
        }

        /** Writes a stream of samples into this pointer from another pointer.
            This will copy the specified number of samples, converting between formats appropriately.
        */
        template <class OtherPointerType>
        z0 convertSamples (OtherPointerType source, i32 numSamples) const noexcept
        {
            // trying to write to a const pointer! For a writeable one, use AudioData::NonConst instead!
            static_assert (Constness::isConst == 0, "Attempt to write to a const pointer");

            Pointer dest (*this);

            if (source.getRawData() != getRawData() || source.getNumBytesBetweenSamples() >= getNumBytesBetweenSamples())
            {
                while (--numSamples >= 0)
                {
                    Endianness::copyFrom (dest.data, source);
                    dest.advance();
                    ++source;
                }
            }
            else // copy backwards if we're increasing the sample width..
            {
                dest += numSamples;
                source += numSamples;

                while (--numSamples >= 0)
                    Endianness::copyFrom ((--dest).data, --source);
            }
        }

        /** Sets a number of samples to zero. */
        z0 clearSamples (i32 numSamples) const noexcept
        {
            Pointer dest (*this);
            dest.clear (dest.data, numSamples);
        }

        /** Scans a block of data, returning the lowest and highest levels as floats */
        Range<f32> findMinAndMax (size_t numSamples) const noexcept
        {
            if (numSamples == 0)
                return Range<f32>();

            Pointer dest (*this);

            if (isFloatingPoint())
            {
                f32 mn = dest.getAsFloat();
                dest.advance();
                f32 mx = mn;

                while (--numSamples > 0)
                {
                    const f32 v = dest.getAsFloat();
                    dest.advance();

                    if (mx < v)  mx = v;
                    if (v < mn)  mn = v;
                }

                return Range<f32> (mn, mx);
            }

            i32 mn = dest.getAsInt32();
            dest.advance();
            i32 mx = mn;

            while (--numSamples > 0)
            {
                i32k v = dest.getAsInt32();
                dest.advance();

                if (mx < v)  mx = v;
                if (v < mn)  mn = v;
            }

            return Range<f32> ((f32) mn * (f32) (1.0 / (1.0 + (f64) Int32::maxValue)),
                                 (f32) mx * (f32) (1.0 / (1.0 + (f64) Int32::maxValue)));
        }

        /** Scans a block of data, returning the lowest and highest levels as floats */
        z0 findMinAndMax (size_t numSamples, f32& minValue, f32& maxValue) const noexcept
        {
            Range<f32> r (findMinAndMax (numSamples));
            minValue = r.getStart();
            maxValue = r.getEnd();
        }

        /** Возвращает true, если the pointer is using a floating-point format. */
        static b8 isFloatingPoint() noexcept                  { return (b8) SampleFormat::isFloat; }

        /** Возвращает true, если the format is big-endian. */
        static b8 isBigEndian() noexcept                      { return (b8) Endianness::isBigEndian; }

        /** Returns the number of bytes in each sample (ignoring the number of interleaved channels). */
        static i32 getBytesPerSample() noexcept                 { return (i32) SampleFormat::bytesPerSample; }

        /** Returns the number of interleaved channels in the format. */
        i32 getNumInterleavedChannels() const noexcept          { return (i32) this->numInterleavedChannels; }

        /** Returns the number of bytes between the start address of each sample. */
        i32 getNumBytesBetweenSamples() const noexcept          { return InterleavingType::getNumBytesBetweenSamples (data); }

        /** Returns the accuracy of this format when represented as a 32-bit integer.
            This is the smallest number above 0 that can be represented in the sample format, converted to
            a 32-bit range. E,g. if the format is 8-bit, its resolution is 0x01000000; if the format is 24-bit,
            its resolution is 0x100.
        */
        static i32 get32BitResolution() noexcept                { return (i32) SampleFormat::resolution; }

        /** Returns a pointer to the underlying data. */
        ukk getRawData() const noexcept                 { return data.data; }

    private:
        //==============================================================================
        SampleFormat data;

        inline z0 advance() noexcept                          { this->advanceData (data); }

        Pointer operator++ (i32); // private to force you to use the more efficient pre-increment!
        Pointer operator-- (i32);
    };

    //==============================================================================
    /** A base class for objects that are used to convert between two different sample formats.

        The AudioData::ConverterInstance implements this base class and can be templated, so
        you can create an instance that converts between two particular formats, and then
        store this in the abstract base class.

        @see AudioData::ConverterInstance
    */
    class Converter
    {
    public:
        virtual ~Converter() = default;

        /** Converts a sequence of samples from the converter's source format into the dest format. */
        virtual z0 convertSamples (uk destSamples, ukk sourceSamples, i32 numSamples) const = 0;

        /** Converts a sequence of samples from the converter's source format into the dest format.
            This method takes sub-channel indexes, which can be used with interleaved formats in order to choose a
            particular sub-channel of the data to be used.
        */
        virtual z0 convertSamples (uk destSamples, i32 destSubChannel,
                                     ukk sourceSamples, i32 sourceSubChannel, i32 numSamples) const = 0;
    };

    //==============================================================================
    /**
        A class that converts between two templated AudioData::Pointer types, and which
        implements the AudioData::Converter interface.

        This can be used as a concrete instance of the AudioData::Converter abstract class.

        @see AudioData::Converter
    */
    template <class SourceSampleType, class DestSampleType>
    class ConverterInstance  : public Converter
    {
    public:
        ConverterInstance (i32 numSourceChannels = 1, i32 numDestChannels = 1)
            : sourceChannels (numSourceChannels), destChannels (numDestChannels)
        {}

        z0 convertSamples (uk dest, ukk source, i32 numSamples) const override
        {
            SourceSampleType s (source, sourceChannels);
            DestSampleType d (dest, destChannels);
            d.convertSamples (s, numSamples);
        }

        z0 convertSamples (uk dest, i32 destSubChannel,
                             ukk source, i32 sourceSubChannel, i32 numSamples) const override
        {
            jassert (destSubChannel < destChannels && sourceSubChannel < sourceChannels);

            SourceSampleType s (addBytesToPointer (source, sourceSubChannel * SourceSampleType::getBytesPerSample()), sourceChannels);
            DestSampleType d (addBytesToPointer (dest, destSubChannel * DestSampleType::getBytesPerSample()), destChannels);
            d.convertSamples (s, numSamples);
        }

    private:
        DRX_DECLARE_NON_COPYABLE (ConverterInstance)

        i32k sourceChannels, destChannels;
    };

    //==============================================================================
    /** A struct that contains a SampleFormat and Endianness to be used with the source and
        destination types when calling the interleaveSamples() and deinterleaveSamples() helpers.

        @see interleaveSamples, deinterleaveSamples
    */
    template <typename DataFormatIn, typename EndiannessIn>
    struct Format
    {
        using DataFormat = DataFormatIn;
        using Endianness = EndiannessIn;
    };

private:
    template <b8 IsInterleaved, b8 IsConst, typename...>
    struct ChannelDataSubtypes;

    template <b8 IsInterleaved, b8 IsConst, typename DataFormat, typename Endianness>
    struct ChannelDataSubtypes<IsInterleaved, IsConst, DataFormat, Endianness>
    {
        using ElementType = std::remove_pointer_t<decltype (DataFormat::data)>;
        using ChannelType = std::conditional_t<IsConst, const ElementType*, ElementType*>;
        using DataType = std::conditional_t<IsInterleaved, ChannelType, ChannelType const*>;
        using PointerType = Pointer<DataFormat,
                                    Endianness,
                                    std::conditional_t<IsInterleaved, Interleaved, NonInterleaved>,
                                    std::conditional_t<IsConst, Const, NonConst>>;
    };

    template <b8 IsInterleaved, b8 IsConst, typename DataFormat, typename Endianness>
    struct ChannelDataSubtypes<IsInterleaved, IsConst, Format<DataFormat, Endianness>>
    {
        using Subtypes    = ChannelDataSubtypes<IsInterleaved, IsConst, DataFormat, Endianness>;
        using DataType    = typename Subtypes::DataType;
        using PointerType = typename Subtypes::PointerType;
    };

    template <b8 IsInterleaved, b8 IsConst, typename... Format>
    struct ChannelData
    {
        using Subtypes    = ChannelDataSubtypes<IsInterleaved, IsConst, Format...>;
        using DataType    = typename Subtypes::DataType;
        using PointerType = typename Subtypes::PointerType;

        DataType data;
        i32 channels;
    };

public:
    //==============================================================================
    /** A sequence of interleaved samples used as the source for the deinterleaveSamples() method. */
    template <typename... Format> using InterleavedSource     = ChannelData<true,  true,   Format...>;
    /** A sequence of interleaved samples used as the destination for the interleaveSamples() method. */
    template <typename... Format> using InterleavedDest       = ChannelData<true,  false,  Format...>;
    /** A sequence of non-interleaved samples used as the source for the interleaveSamples() method. */
    template <typename... Format> using NonInterleavedSource  = ChannelData<false, true,   Format...>;
    /** A sequence of non-interleaved samples used as the destination for the deinterleaveSamples() method. */
    template <typename... Format> using NonInterleavedDest    = ChannelData<false, false,  Format...>;

    /** A helper function for converting a sequence of samples from a non-interleaved source
        to an interleaved destination.

        When calling this method you need to specify the source and destination data format and endianness
        from the AudioData SampleFormat and Endianness types and provide the data and number of channels
        for each. For example, to convert a floating-point stream of big endian samples to an interleaved,
        native endian stream of 16-bit integer samples you would do the following:

        @code
        using SourceFormat = AudioData::Format<AudioData::Float32, AudioData::BigEndian>;
        using DestFormat   = AudioData::Format<AudioData::Int16,   AudioData::NativeEndian>;

        AudioData::interleaveSamples (AudioData::NonInterleavedSource<SourceFormat> { sourceData, numSourceChannels },
                                      AudioData::InterleavedDest<DestFormat>        { destData,   numDestChannels },
                                      numSamples);
        @endcode
    */
    template <typename... SourceFormat, typename... DestFormat>
    static z0 interleaveSamples (NonInterleavedSource<SourceFormat...> source,
                                   InterleavedDest<DestFormat...> dest,
                                   i32 numSamples)
    {
        using SourceType = typename decltype (source)::PointerType;
        using DestType   = typename decltype (dest)  ::PointerType;

        for (i32 i = 0; i < dest.channels; ++i)
        {
            const DestType destType (addBytesToPointer (dest.data, i * DestType::getBytesPerSample()), dest.channels);

            if (i < source.channels)
            {
                if (*source.data != nullptr)
                {
                    destType.convertSamples (SourceType { *source.data }, numSamples);
                    ++source.data;
                }
            }
            else
            {
                destType.clearSamples (numSamples);
            }
        }
    }

    /** A helper function for converting a sequence of samples from an interleaved source
        to a non-interleaved destination.

        When calling this method you need to specify the source and destination data format and endianness
        from the AudioData SampleFormat and Endianness types and provide the data and number of channels
        for each. For example, to convert a floating-point stream of big endian samples to an non-interleaved,
        native endian stream of 16-bit integer samples you would do the following:

        @code
        using SourceFormat = AudioData::Format<AudioData::Float32, AudioData::BigEndian>;
        using DestFormat   = AudioData::Format<AudioData::Int16,   AudioData::NativeEndian>;

        AudioData::deinterleaveSamples (AudioData::InterleavedSource<SourceFormat> { sourceData, numSourceChannels },
                                        AudioData::NonInterleavedDest<DestFormat>  { destData,   numDestChannels },
                                        numSamples);
        @endcode
    */
    template <typename... SourceFormat, typename... DestFormat>
    static z0 deinterleaveSamples (InterleavedSource<SourceFormat...> source,
                                     NonInterleavedDest<DestFormat...> dest,
                                     i32 numSamples)
    {
        using SourceType = typename decltype (source)::PointerType;
        using DestType   = typename decltype (dest)  ::PointerType;

        for (i32 i = 0; i < dest.channels; ++i)
        {
            if (auto* targetChan = dest.data[i])
            {
                const DestType destType (targetChan);

                if (i < source.channels)
                    destType.convertSamples (SourceType (addBytesToPointer (source.data, i * SourceType::getBytesPerSample()), source.channels), numSamples);
                else
                    destType.clearSamples (numSamples);
            }
        }
    }
};

//==============================================================================
#ifndef DOXYGEN
/**
    A set of routines to convert buffers of 32-bit floating point data to and from
    various integer formats.

    Note that these functions are deprecated - the AudioData class provides a much more
    flexible set of conversion classes now.

    @tags{Audio}
*/
class [[deprecated]] DRX_API  AudioDataConverters
{
public:
    //==============================================================================
    static z0 convertFloatToInt16LE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample = 2);
    static z0 convertFloatToInt16BE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample = 2);

    static z0 convertFloatToInt24LE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample = 3);
    static z0 convertFloatToInt24BE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample = 3);

    static z0 convertFloatToInt32LE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample = 4);
    static z0 convertFloatToInt32BE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample = 4);

    static z0 convertFloatToFloat32LE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample = 4);
    static z0 convertFloatToFloat32BE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample = 4);

    //==============================================================================
    static z0 convertInt16LEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample = 2);
    static z0 convertInt16BEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample = 2);

    static z0 convertInt24LEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample = 3);
    static z0 convertInt24BEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample = 3);

    static z0 convertInt32LEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample = 4);
    static z0 convertInt32BEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample = 4);

    static z0 convertFloat32LEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample = 4);
    static z0 convertFloat32BEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample = 4);

    //==============================================================================
    enum DataFormat
    {
        int16LE,
        int16BE,
        int24LE,
        int24BE,
        int32LE,
        int32BE,
        float32LE,
        float32BE,
    };

    static z0 convertFloatToFormat (DataFormat destFormat,
                                      const f32* source, uk dest, i32 numSamples);

    static z0 convertFormatToFloat (DataFormat sourceFormat,
                                      ukk source, f32* dest, i32 numSamples);

    //==============================================================================
    static z0 interleaveSamples (const f32** source, f32* dest,
                                   i32 numSamples, i32 numChannels);

    static z0 deinterleaveSamples (const f32* source, f32** dest,
                                     i32 numSamples, i32 numChannels);

private:
    AudioDataConverters();
};
#endif

} // namespace drx
