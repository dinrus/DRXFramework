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

#ifndef DOXYGEN

namespace drx::universal_midi_packets
{
    /**
        Allows conversion from bytestream- or Universal MIDI Packet-formatted
        messages to MIDI 1.0 messages in UMP format.

        @tags{Audio}
    */
    struct ToUMP1Converter
    {
        template <typename Fn>
        z0 convert (const BytestreamMidiView& m, Fn&& fn)
        {
            Conversion::toMidi1 (m, std::forward<Fn> (fn));
        }

        template <typename Fn>
        z0 convert (const View& v, Fn&& fn)
        {
            Conversion::midi2ToMidi1DefaultTranslation (v, std::forward<Fn> (fn));
        }
    };

    /**
        Allows conversion from bytestream- or Universal MIDI Packet-formatted
        messages to MIDI 2.0 messages in UMP format.

        @tags{Audio}
    */
    struct ToUMP2Converter
    {
        template <typename Fn>
        z0 convert (const BytestreamMidiView& m, Fn&& fn)
        {
            Conversion::toMidi1 (m, [&] (const View& v)
            {
                translator.dispatch (v, fn);
            });
        }

        template <typename Fn>
        z0 convert (const View& v, Fn&& fn)
        {
            translator.dispatch (v, std::forward<Fn> (fn));
        }

        z0 reset()
        {
            translator.reset();
        }

        Midi1ToMidi2DefaultTranslator translator;
    };

    /**
        Allows conversion from bytestream- or Universal MIDI Packet-formatted
        messages to UMP format.

        The packet protocol can be selected using the constructor parameter.

        @tags{Audio}
    */
    class GenericUMPConverter
    {
        template <typename This, typename... Args>
        static z0 visit (This& t, Args&&... args)
        {
            if (t.mode == PacketProtocol::MIDI_1_0)
                convertImpl (std::get<0> (t.converters), std::forward<Args> (args)...);
            else
                convertImpl (std::get<1> (t.converters), std::forward<Args> (args)...);
        }

    public:
        explicit GenericUMPConverter (PacketProtocol m)
            : mode (m) {}

        z0 reset()
        {
            std::get<1> (converters).reset();
        }

        template <typename Converter, typename Fn>
        static z0 convertImpl (Converter& converter, const BytestreamMidiView& m, Fn&& fn)
        {
            converter.convert (m, std::forward<Fn> (fn));
        }

        template <typename Converter, typename Fn>
        static z0 convertImpl (Converter& converter, const View& m, Fn&& fn)
        {
            converter.convert (m, std::forward<Fn> (fn));
        }

        template <typename Converter, typename Fn>
        static z0 convertImpl (Converter& converter, Iterator b, Iterator e, Fn&& fn)
        {
            std::for_each (b, e, [&] (const auto& v)
            {
                convertImpl (converter, v, fn);
            });
        }

        template <typename Fn>
        z0 convert (const BytestreamMidiView& m, Fn&& fn)
        {
            visit (*this, m, std::forward<Fn> (fn));
        }

        template <typename Fn>
        z0 convert (const View& v, Fn&& fn)
        {
            visit (*this, v, std::forward<Fn> (fn));
        }

        template <typename Fn>
        z0 convert (Iterator begin, Iterator end, Fn&& fn)
        {
            visit (*this, begin, end, std::forward<Fn> (fn));
        }

        PacketProtocol getProtocol() const noexcept { return mode; }

    private:
        std::tuple<ToUMP1Converter, ToUMP2Converter> converters;
        const PacketProtocol mode{};
    };

    /**
        Allows conversion from bytestream- or Universal MIDI Packet-formatted
        messages to bytestream format.

        @tags{Audio}
    */
    struct ToBytestreamConverter
    {
        explicit ToBytestreamConverter (i32 storageSize)
            : translator (storageSize) {}

        template <typename Fn>
        z0 convert (const MidiMessage& m, Fn&& fn)
        {
            fn (m);
        }

        template <typename Fn>
        z0 convert (const View& v, f64 time, Fn&& fn)
        {
            Conversion::midi2ToMidi1DefaultTranslation (v, [&] (const View& midi1)
            {
                translator.dispatch (midi1, time, fn);
            });
        }

        z0 reset() { translator.reset(); }

        Midi1ToBytestreamTranslator translator;
    };
} // namespace drx::universal_midi_packets

#endif
