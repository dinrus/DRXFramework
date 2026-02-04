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

DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS

z0 AudioDataConverters::convertFloatToInt16LE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample)
{
    auto maxVal = (f64) 0x7fff;
    auto intData = static_cast<tuk> (dest);

    if (dest != (uk) source || destBytesPerSample <= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            *unalignedPointerCast<u16*> (intData) = ByteOrder::swapIfBigEndian ((u16) (short) roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            *unalignedPointerCast<u16*> (intData) = ByteOrder::swapIfBigEndian ((u16) (short) roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
        }
    }
}

z0 AudioDataConverters::convertFloatToInt16BE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample)
{
    auto maxVal = (f64) 0x7fff;
    auto intData = static_cast<tuk> (dest);

    if (dest != (uk) source || destBytesPerSample <= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            *unalignedPointerCast<u16*> (intData) = ByteOrder::swapIfLittleEndian ((u16) (short) roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            *unalignedPointerCast<u16*> (intData) = ByteOrder::swapIfLittleEndian ((u16) (short) roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
        }
    }
}

z0 AudioDataConverters::convertFloatToInt24LE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample)
{
    auto maxVal = (f64) 0x7fffff;
    auto intData = static_cast<tuk> (dest);

    if (dest != (uk) source || destBytesPerSample <= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            ByteOrder::littleEndian24BitToChars (roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            ByteOrder::littleEndian24BitToChars (roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
        }
    }
}

z0 AudioDataConverters::convertFloatToInt24BE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample)
{
    auto maxVal = (f64) 0x7fffff;
    auto intData = static_cast<tuk> (dest);

    if (dest != (uk) source || destBytesPerSample <= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            ByteOrder::bigEndian24BitToChars (roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            ByteOrder::bigEndian24BitToChars (roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])), intData);
        }
    }
}

z0 AudioDataConverters::convertFloatToInt32LE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample)
{
    auto maxVal = (f64) 0x7fffffff;
    auto intData = static_cast<tuk> (dest);

    if (dest != (uk) source || destBytesPerSample <= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            *unalignedPointerCast<u32*> (intData) = ByteOrder::swapIfBigEndian ((u32) roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            *unalignedPointerCast<u32*> (intData) = ByteOrder::swapIfBigEndian ((u32) roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
        }
    }
}

z0 AudioDataConverters::convertFloatToInt32BE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample)
{
    auto maxVal = (f64) 0x7fffffff;
    auto intData = static_cast<tuk> (dest);

    if (dest != (uk) source || destBytesPerSample <= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            *unalignedPointerCast<u32*> (intData) = ByteOrder::swapIfLittleEndian ((u32) roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
            intData += destBytesPerSample;
        }
    }
    else
    {
        intData += destBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= destBytesPerSample;
            *unalignedPointerCast<u32*> (intData) = ByteOrder::swapIfLittleEndian ((u32) roundToInt (jlimit (-maxVal, maxVal, maxVal * source[i])));
        }
    }
}

z0 AudioDataConverters::convertFloatToFloat32LE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample)
{
    jassert (dest != (uk) source || destBytesPerSample <= 4); // This op can't be performed on in-place data!

    tuk d = static_cast<tuk> (dest);

    for (i32 i = 0; i < numSamples; ++i)
    {
        *unalignedPointerCast<f32*> (d) = source[i];

       #if DRX_BIG_ENDIAN
        *unalignedPointerCast<u32*> (d) = ByteOrder::swap (*unalignedPointerCast<u32*> (d));
       #endif

        d += destBytesPerSample;
    }
}

z0 AudioDataConverters::convertFloatToFloat32BE (const f32* source, uk dest, i32 numSamples, i32 destBytesPerSample)
{
    jassert (dest != (uk) source || destBytesPerSample <= 4); // This op can't be performed on in-place data!

    auto d = static_cast<tuk> (dest);

    for (i32 i = 0; i < numSamples; ++i)
    {
        *unalignedPointerCast<f32*> (d) = source[i];

       #if DRX_LITTLE_ENDIAN
        *unalignedPointerCast<u32*> (d) = ByteOrder::swap (*unalignedPointerCast<u32*> (d));
       #endif

        d += destBytesPerSample;
    }
}

//==============================================================================
z0 AudioDataConverters::convertInt16LEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample)
{
    const f32 scale = 1.0f / 0x7fff;
    auto intData = static_cast<tukk> (source);

    if (source != (uk) dest || srcBytesPerSample >= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (short) ByteOrder::swapIfBigEndian (*unalignedPointerCast<u16k*> (intData));
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (short) ByteOrder::swapIfBigEndian (*unalignedPointerCast<u16k*> (intData));
        }
    }
}

z0 AudioDataConverters::convertInt16BEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample)
{
    const f32 scale = 1.0f / 0x7fff;
    auto intData = static_cast<tukk> (source);

    if (source != (uk) dest || srcBytesPerSample >= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (short) ByteOrder::swapIfLittleEndian (*unalignedPointerCast<u16k*> (intData));
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (short) ByteOrder::swapIfLittleEndian (*unalignedPointerCast<u16k*> (intData));
        }
    }
}

z0 AudioDataConverters::convertInt24LEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample)
{
    const f32 scale = 1.0f / 0x7fffff;
    auto intData = static_cast<tukk> (source);

    if (source != (uk) dest || srcBytesPerSample >= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (short) ByteOrder::littleEndian24Bit (intData);
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (short) ByteOrder::littleEndian24Bit (intData);
        }
    }
}

z0 AudioDataConverters::convertInt24BEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample)
{
    const f32 scale = 1.0f / 0x7fffff;
    auto intData = static_cast<tukk> (source);

    if (source != (uk) dest || srcBytesPerSample >= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (short) ByteOrder::bigEndian24Bit (intData);
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (short) ByteOrder::bigEndian24Bit (intData);
        }
    }
}

z0 AudioDataConverters::convertInt32LEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample)
{
    const f32 scale = 1.0f / (f32) 0x7fffffff;
    auto intData = static_cast<tukk> (source);

    if (source != (uk) dest || srcBytesPerSample >= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (f32) ByteOrder::swapIfBigEndian (*unalignedPointerCast<u32k*> (intData));
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (f32) ByteOrder::swapIfBigEndian (*unalignedPointerCast<u32k*> (intData));
        }
    }
}

z0 AudioDataConverters::convertInt32BEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample)
{
    const f32 scale = 1.0f / (f32) 0x7fffffff;
    auto intData = static_cast<tukk> (source);

    if (source != (uk) dest || srcBytesPerSample >= 4)
    {
        for (i32 i = 0; i < numSamples; ++i)
        {
            dest[i] = scale * (f32) ByteOrder::swapIfLittleEndian (*unalignedPointerCast<u32k*> (intData));
            intData += srcBytesPerSample;
        }
    }
    else
    {
        intData += srcBytesPerSample * numSamples;

        for (i32 i = numSamples; --i >= 0;)
        {
            intData -= srcBytesPerSample;
            dest[i] = scale * (f32) ByteOrder::swapIfLittleEndian (*unalignedPointerCast<u32k*> (intData));
        }
    }
}

z0 AudioDataConverters::convertFloat32LEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample)
{
    auto s = static_cast<tukk> (source);

    for (i32 i = 0; i < numSamples; ++i)
    {
        dest[i] = *unalignedPointerCast<const f32*> (s);

       #if DRX_BIG_ENDIAN
        auto d = unalignedPointerCast<u32*> (dest + i);
        *d = ByteOrder::swap (*d);
       #endif

        s += srcBytesPerSample;
    }
}

z0 AudioDataConverters::convertFloat32BEToFloat (ukk source, f32* dest, i32 numSamples, i32 srcBytesPerSample)
{
    auto s = static_cast<tukk> (source);

    for (i32 i = 0; i < numSamples; ++i)
    {
        dest[i] = *unalignedPointerCast<const f32*> (s);

       #if DRX_LITTLE_ENDIAN
        auto d = unalignedPointerCast<u32*> (dest + i);
        *d = ByteOrder::swap (*d);
       #endif

        s += srcBytesPerSample;
    }
}


//==============================================================================
z0 AudioDataConverters::convertFloatToFormat (DataFormat destFormat, const f32* source, uk dest, i32 numSamples)
{
    switch (destFormat)
    {
        case int16LE:       convertFloatToInt16LE   (source, dest, numSamples); break;
        case int16BE:       convertFloatToInt16BE   (source, dest, numSamples); break;
        case int24LE:       convertFloatToInt24LE   (source, dest, numSamples); break;
        case int24BE:       convertFloatToInt24BE   (source, dest, numSamples); break;
        case int32LE:       convertFloatToInt32LE   (source, dest, numSamples); break;
        case int32BE:       convertFloatToInt32BE   (source, dest, numSamples); break;
        case float32LE:     convertFloatToFloat32LE (source, dest, numSamples); break;
        case float32BE:     convertFloatToFloat32BE (source, dest, numSamples); break;
        default:            jassertfalse; break;
    }
}

z0 AudioDataConverters::convertFormatToFloat (DataFormat sourceFormat, ukk source, f32* dest, i32 numSamples)
{
    switch (sourceFormat)
    {
        case int16LE:       convertInt16LEToFloat   (source, dest, numSamples); break;
        case int16BE:       convertInt16BEToFloat   (source, dest, numSamples); break;
        case int24LE:       convertInt24LEToFloat   (source, dest, numSamples); break;
        case int24BE:       convertInt24BEToFloat   (source, dest, numSamples); break;
        case int32LE:       convertInt32LEToFloat   (source, dest, numSamples); break;
        case int32BE:       convertInt32BEToFloat   (source, dest, numSamples); break;
        case float32LE:     convertFloat32LEToFloat (source, dest, numSamples); break;
        case float32BE:     convertFloat32BEToFloat (source, dest, numSamples); break;
        default:            jassertfalse; break;
    }
}

//==============================================================================
z0 AudioDataConverters::interleaveSamples (const f32** source, f32* dest, i32 numSamples, i32 numChannels)
{
    using Format = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

    AudioData::interleaveSamples (AudioData::NonInterleavedSource<Format> { source, numChannels },
                                  AudioData::InterleavedDest<Format>      { dest,   numChannels },
                                  numSamples);
}

z0 AudioDataConverters::deinterleaveSamples (const f32* source, f32** dest, i32 numSamples, i32 numChannels)
{
    using Format = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

    AudioData::deinterleaveSamples (AudioData::InterleavedSource<Format>  { source, numChannels },
                                    AudioData::NonInterleavedDest<Format> { dest,   numChannels },
                                    numSamples);
}

//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class AudioConversionTests final : public UnitTest
{
public:
    AudioConversionTests()
        : UnitTest ("Audio data conversion", UnitTestCategories::audio)
    {}

    template <class F1, class E1, class F2, class E2>
    struct Test5
    {
        static z0 test (UnitTest& unitTest, Random& r)
        {
            test (unitTest, false, r);
            test (unitTest, true, r);
        }

        DRX_BEGIN_IGNORE_WARNINGS_MSVC (6262)
        static z0 test (UnitTest& unitTest, b8 inPlace, Random& r)
        {
            i32k numSamples = 2048;
            i32 original [(size_t) numSamples],
                  converted[(size_t) numSamples],
                  reversed [(size_t) numSamples];

            {
                AudioData::Pointer<F1, E1, AudioData::NonInterleaved, AudioData::NonConst> d (original);
                b8 clippingFailed = false;

                for (i32 i = 0; i < numSamples / 2; ++i)
                {
                    d.setAsFloat (r.nextFloat() * 2.2f - 1.1f);

                    if (! d.isFloatingPoint())
                        clippingFailed = d.getAsFloat() > 1.0f || d.getAsFloat() < -1.0f || clippingFailed;

                    ++d;
                    d.setAsInt32 (r.nextInt());
                    ++d;
                }

                unitTest.expect (! clippingFailed);
            }

            // convert data from the source to dest format..
            std::unique_ptr<AudioData::Converter> conv (new AudioData::ConverterInstance<AudioData::Pointer<F1, E1, AudioData::NonInterleaved, AudioData::Const>,
                                                                                         AudioData::Pointer<F2, E2, AudioData::NonInterleaved, AudioData::NonConst>>());
            conv->convertSamples (inPlace ? reversed : converted, original, numSamples);

            // ..and back again..
            conv.reset (new AudioData::ConverterInstance<AudioData::Pointer<F2, E2, AudioData::NonInterleaved, AudioData::Const>,
                                                         AudioData::Pointer<F1, E1, AudioData::NonInterleaved, AudioData::NonConst>>());
            if (! inPlace)
                zeromem (reversed, sizeof (reversed));

            conv->convertSamples (reversed, inPlace ? reversed : converted, numSamples);

            {
                i32 biggestDiff = 0;
                AudioData::Pointer<F1, E1, AudioData::NonInterleaved, AudioData::Const> d1 (original);
                AudioData::Pointer<F1, E1, AudioData::NonInterleaved, AudioData::Const> d2 (reversed);

                i32k errorMargin = 2 * AudioData::Pointer<F1, E1, AudioData::NonInterleaved, AudioData::Const>::get32BitResolution()
                                          + AudioData::Pointer<F2, E2, AudioData::NonInterleaved, AudioData::Const>::get32BitResolution();

                for (i32 i = 0; i < numSamples; ++i)
                {
                    biggestDiff = jmax (biggestDiff, std::abs (d1.getAsInt32() - d2.getAsInt32()));
                    ++d1;
                    ++d2;
                }

                unitTest.expect (biggestDiff <= errorMargin);
            }
        }
        DRX_END_IGNORE_WARNINGS_MSVC
    };

    template <class F1, class E1, class FormatType>
    struct Test3
    {
        static z0 test (UnitTest& unitTest, Random& r)
        {
            Test5 <F1, E1, FormatType, AudioData::BigEndian>::test (unitTest, r);
            Test5 <F1, E1, FormatType, AudioData::LittleEndian>::test (unitTest, r);
        }
    };

    template <class FormatType, class Endianness>
    struct Test2
    {
        static z0 test (UnitTest& unitTest, Random& r)
        {
            Test3 <FormatType, Endianness, AudioData::Int8>::test (unitTest, r);
            Test3 <FormatType, Endianness, AudioData::UInt8>::test (unitTest, r);
            Test3 <FormatType, Endianness, AudioData::Int16>::test (unitTest, r);
            Test3 <FormatType, Endianness, AudioData::Int24>::test (unitTest, r);
            Test3 <FormatType, Endianness, AudioData::Int32>::test (unitTest, r);
            Test3 <FormatType, Endianness, AudioData::Float32>::test (unitTest, r);
        }
    };

    template <class FormatType>
    struct Test1
    {
        static z0 test (UnitTest& unitTest, Random& r)
        {
            Test2 <FormatType, AudioData::BigEndian>::test (unitTest, r);
            Test2 <FormatType, AudioData::LittleEndian>::test (unitTest, r);
        }
    };

    z0 runTest() override
    {
        auto r = getRandom();
        beginTest ("Round-trip conversion: Int8");
        Test1 <AudioData::Int8>::test (*this, r);
        beginTest ("Round-trip conversion: Int16");
        Test1 <AudioData::Int16>::test (*this, r);
        beginTest ("Round-trip conversion: Int24");
        Test1 <AudioData::Int24>::test (*this, r);
        beginTest ("Round-trip conversion: Int32");
        Test1 <AudioData::Int32>::test (*this, r);
        beginTest ("Round-trip conversion: Float32");
        Test1 <AudioData::Float32>::test (*this, r);

        using Format = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

        beginTest ("Interleaving");
        {
            constexpr auto numChannels = 4;
            constexpr auto numSamples = 512;

            AudioBuffer<f32> sourceBuffer { numChannels, numSamples },
                               destBuffer   { 1, numChannels * numSamples };

            for (i32 ch = 0; ch < numChannels; ++ch)
                for (i32 i = 0; i < numSamples; ++i)
                    sourceBuffer.setSample (ch, i, r.nextFloat());

            AudioData::interleaveSamples (AudioData::NonInterleavedSource<Format> { sourceBuffer.getArrayOfReadPointers(), numChannels },
                                          AudioData::InterleavedDest<Format>      { destBuffer.getWritePointer (0),        numChannels },
                                          numSamples);

            for (i32 ch = 0; ch < numChannels; ++ch)
                for (i32 i = 0; i < numSamples; ++i)
                    expectEquals (destBuffer.getSample (0, ch + (i * numChannels)), sourceBuffer.getSample (ch, i));
        }

        beginTest ("Deinterleaving");
        {
            constexpr auto numChannels = 4;
            constexpr auto numSamples = 512;

            AudioBuffer<f32> sourceBuffer { 1, numChannels * numSamples },
                               destBuffer   { numChannels, numSamples };

            for (i32 ch = 0; ch < numChannels; ++ch)
                for (i32 i = 0; i < numSamples; ++i)
                    sourceBuffer.setSample (0, ch + (i * numChannels), r.nextFloat());

            AudioData::deinterleaveSamples (AudioData::InterleavedSource<Format>  { sourceBuffer.getReadPointer (0),      numChannels },
                                            AudioData::NonInterleavedDest<Format> { destBuffer.getArrayOfWritePointers(), numChannels },
                                            numSamples);

            for (i32 ch = 0; ch < numChannels; ++ch)
                for (i32 i = 0; i < numSamples; ++i)
                    expectEquals (sourceBuffer.getSample (0, ch + (i * numChannels)), destBuffer.getSample (ch, i));
        }
    }
};

static AudioConversionTests audioConversionUnitTests;

#endif

DRX_END_IGNORE_DEPRECATION_WARNINGS

} // namespace drx
