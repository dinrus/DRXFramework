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

#if DRX_USE_MP3AUDIOFORMAT || DOXYGEN

//==============================================================================
/**
    Software-based MP3 decoding format (doesn't currently provide an encoder).

    IMPORTANT DISCLAIMER: By choosing to enable the DRX_USE_MP3AUDIOFORMAT flag and
    to compile the MP3 code into your software, you do so AT YOUR OWN RISK! By doing so,
    you are agreeing that DinrusPro is in no way responsible for any patent,
    copyright, or other legal issues that you may suffer as a result.

    The code in drx_MP3AudioFormat.cpp is NOT guaranteed to be free from infringements of 3rd-party
    intellectual property. If you wish to use it, please seek your own independent advice about the
    legality of doing so. If you are not willing to accept full responsibility for the consequences
    of using this code, then do not enable the DRX_USE_MP3AUDIOFORMAT setting.

    @tags{Audio}
*/
class MP3AudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    MP3AudioFormat();
    ~MP3AudioFormat() override;

    //==============================================================================
    Array<i32> getPossibleSampleRates() override;
    Array<i32> getPossibleBitDepths() override;
    b8 canDoStereo() override;
    b8 canDoMono() override;
    b8 isCompressed() override;
    StringArray getQualityOptions() override;

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream*, b8 deleteStreamIfOpeningFails) override;

    AudioFormatWriter* createWriterFor (OutputStream*, f64 sampleRateToUse,
                                        u32 numberOfChannels, i32 bitsPerSample,
                                        const StringPairArray& metadataValues, i32 qualityOptionIndex) override;
    using AudioFormat::createWriterFor;
};

#endif

} // namespace drx
