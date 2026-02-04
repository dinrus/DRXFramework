/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#pragma once


//==============================================================================
/* This component scrolls a continuous waveform showing the audio that's
   coming into whatever audio inputs this object is connected to.
*/
class LiveScrollingAudioDisplay final : public AudioVisualiserComponent,
                                        public AudioIODeviceCallback
{
public:
    LiveScrollingAudioDisplay()  : AudioVisualiserComponent (1)
    {
        setSamplesPerBlock (256);
        setBufferSize (1024);
    }

    //==============================================================================
    z0 audioDeviceAboutToStart (AudioIODevice*) override
    {
        clear();
    }

    z0 audioDeviceStopped() override
    {
        clear();
    }

    z0 audioDeviceIOCallbackWithContext (const f32* const* inputChannelData, i32 numInputChannels,
                                           f32* const* outputChannelData, i32 numOutputChannels,
                                           i32 numberOfSamples, const AudioIODeviceCallbackContext& context) override
    {
        ignoreUnused (context);

        for (i32 i = 0; i < numberOfSamples; ++i)
        {
            f32 inputSample = 0;

            for (i32 chan = 0; chan < numInputChannels; ++chan)
                if (const f32* inputChannel = inputChannelData[chan])
                    inputSample += inputChannel[i];  // find the sum of all the channels

            inputSample *= 10.0f; // boost the level to make it more easily visible.

            pushSample (&inputSample, 1);
        }

        // We need to clear the output buffers before returning, in case they're full of junk..
        for (i32 j = 0; j < numOutputChannels; ++j)
            if (f32* outputChannel = outputChannelData[j])
                zeromem (outputChannel, (size_t) numberOfSamples * sizeof (f32));
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveScrollingAudioDisplay)
};
