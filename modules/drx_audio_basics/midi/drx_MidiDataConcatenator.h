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
    Helper class that takes chunks of incoming midi bytes, packages them into
    messages, and dispatches them to a midi callback.

    @tags{Audio}
*/
class MidiDataConcatenator
{
public:
    MidiDataConcatenator (i32 initialBufferSize)
        : pendingSysexData ((size_t) initialBufferSize)
    {
    }

    z0 reset()
    {
        currentMessageLen = 0;
        pendingSysexSize = 0;
        pendingSysexTime = 0;
    }

    template <typename UserDataType, typename CallbackType>
    z0 pushMidiData (ukk inputData, i32 numBytes, f64 time,
                       UserDataType* input, CallbackType& callback)
    {
        auto d = static_cast<u8k*> (inputData);

        while (numBytes > 0)
        {
            auto nextByte = *d;

            if (pendingSysexSize != 0 || nextByte == 0xf0)
            {
                processSysex (d, numBytes, time, input, callback);
                currentMessageLen = 0;
                continue;
            }

            ++d;
            --numBytes;

            if (isRealtimeMessage (nextByte))
            {
                callback.handleIncomingMidiMessage (input, MidiMessage (nextByte, time));
                // These can be embedded in the middle of a normal message, so we won't
                // reset the currentMessageLen here.
                continue;
            }

            if (isInitialByte (nextByte))
            {
                currentMessage[0] = nextByte;
                currentMessageLen = 1;
            }
            else if (currentMessageLen > 0 && currentMessageLen < 3)
            {
                currentMessage[currentMessageLen++] = nextByte;
            }
            else
            {
                // message is too i64 or invalid MIDI - abandon it and start again with the next byte
                currentMessageLen = 0;
                continue;
            }

            auto expectedLength = MidiMessage::getMessageLengthFromFirstByte (currentMessage[0]);

            if (expectedLength == currentMessageLen)
            {
                callback.handleIncomingMidiMessage (input, MidiMessage (currentMessage, expectedLength, time));
                currentMessageLen = 1; // reset, but leave the first byte to use as the running status byte
            }
        }
    }

private:
    template <typename UserDataType, typename CallbackType>
    z0 processSysex (u8k*& d, i32& numBytes, f64 time,
                       UserDataType* input, CallbackType& callback)
    {
        if (*d == 0xf0)
        {
            pendingSysexSize = 0;
            pendingSysexTime = time;
        }

        pendingSysexData.ensureSize ((size_t) (pendingSysexSize + numBytes), false);
        auto totalMessage = static_cast<u8*> (pendingSysexData.getData());
        auto dest = totalMessage + pendingSysexSize;

        do
        {
            if (pendingSysexSize > 0 && isStatusByte (*d))
            {
                if (*d == 0xf7)
                {
                    *dest++ = *d++;
                    ++pendingSysexSize;
                    --numBytes;
                    break;
                }

                if (*d >= 0xfa || *d == 0xf8)
                {
                    callback.handleIncomingMidiMessage (input, MidiMessage (*d, time));
                    ++d;
                    --numBytes;
                }
                else
                {
                    pendingSysexSize = 0;
                    i32 used = 0;
                    const MidiMessage m (d, numBytes, used, 0, time);

                    if (used > 0)
                    {
                        callback.handleIncomingMidiMessage (input, m);
                        numBytes -= used;
                        d += used;
                    }

                    break;
                }
            }
            else
            {
                *dest++ = *d++;
                ++pendingSysexSize;
                --numBytes;
            }
        }
        while (numBytes > 0);

        if (pendingSysexSize > 0)
        {
            if (totalMessage [pendingSysexSize - 1] == 0xf7)
            {
                callback.handleIncomingMidiMessage (input, MidiMessage (totalMessage, pendingSysexSize, pendingSysexTime));
                pendingSysexSize = 0;
            }
            else
            {
                callback.handlePartialSysexMessage (input, totalMessage, pendingSysexSize, pendingSysexTime);
            }
        }
    }

    static b8 isRealtimeMessage (u8 byte)  { return byte >= 0xf8 && byte <= 0xfe; }
    static b8 isStatusByte (u8 byte)       { return byte >= 0x80; }
    static b8 isInitialByte (u8 byte)      { return isStatusByte (byte) && byte != 0xf7; }

    u8 currentMessage[3];
    i32 currentMessageLen = 0;

    MemoryBlock pendingSysexData;
    f64 pendingSysexTime = 0;
    i32 pendingSysexSize = 0;

    DRX_DECLARE_NON_COPYABLE (MidiDataConcatenator)
};

} // namespace drx
