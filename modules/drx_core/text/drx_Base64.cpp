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

b8 Base64::convertToBase64 (OutputStream& base64Result, ukk sourceData, size_t sourceDataSize)
{
    static const t8 lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    auto* source = static_cast<u8k*> (sourceData);

    while (sourceDataSize > 0)
    {
        t8 frame[4];
        auto byte0 = *source++;
        frame[0] = lookup [(byte0 & 0xfcu) >> 2];
        u32 bits = (byte0 & 0x03u) << 4;

        if (sourceDataSize > 1)
        {
            auto byte1 = *source++;
            frame[1] = lookup[bits | ((byte1 & 0xf0u) >> 4)];
            bits = (byte1 & 0x0fu) << 2;

            if (sourceDataSize > 2)
            {
                auto byte2 = *source++;
                frame[2] = lookup[bits | ((byte2 & 0xc0u) >> 6)];
                frame[3] = lookup[byte2 & 0x3fu];
                sourceDataSize -= 3;
            }
            else
            {
                frame[2] = lookup[bits];
                frame[3] = '=';
                sourceDataSize = 0;
            }
        }
        else
        {
            frame[1] = lookup[bits];
            frame[2] = '=';
            frame[3] = '=';
            sourceDataSize = 0;
        }

        if (! base64Result.write (frame, 4))
            return false;
    }

    return true;
}

b8 Base64::convertFromBase64 (OutputStream& binaryOutput, StringRef base64TextInput)
{
    for (auto s = base64TextInput.text; ! s.isEmpty();)
    {
        u8 data[4];

        for (i32 i = 0; i < 4; ++i)
        {
            auto c = (u32) s.getAndAdvance();

            if (c >= 'A' && c <= 'Z')         c -= 'A';
            else if (c >= 'a' && c <= 'z')    c -= 'a' - 26;
            else if (c >= '0' && c <= '9')    c += 52 - '0';
            else if (c == '+')                c = 62;
            else if (c == '/')                c = 63;
            else if (c == '=')                { c = 64; if (i <= 1) return false; }
            else                              return false;

            data[i] = (u8) c;
        }

        binaryOutput.writeByte ((t8) ((data[0] << 2) | (data[1] >> 4)));

        if (data[2] < 64)
        {
            binaryOutput.writeByte ((t8) ((data[1] << 4) | (data[2] >> 2)));

            if (data[3] < 64)
                binaryOutput.writeByte ((t8) ((data[2] << 6) | data[3]));
        }
    }

    return true;
}

Txt Base64::toBase64 (ukk sourceData, size_t sourceDataSize)
{
    MemoryOutputStream m ((sourceDataSize * 4) / 3 + 3);
    [[maybe_unused]] b8 ok = convertToBase64 (m, sourceData, sourceDataSize);
    jassert (ok); // should always succeed for this simple case
    return m.toString();
}

Txt Base64::toBase64 (const Txt& text)
{
    return toBase64 (text.toRawUTF8(), strlen (text.toRawUTF8()));
}


//==============================================================================
//==============================================================================
#if DRX_UNIT_TESTS

class Base64Tests final : public UnitTest
{
public:
    Base64Tests()
        : UnitTest ("Base64 class", UnitTestCategories::text)
    {}

    static MemoryBlock createRandomData (Random& r)
    {
        MemoryOutputStream m;

        for (i32 i = r.nextInt (400); --i >= 0;)
            m.writeByte ((t8) r.nextInt (256));

        return m.getMemoryBlock();
    }

    z0 runTest() override
    {
        beginTest ("Base64");

        auto r = getRandom();

        for (i32 i = 1000; --i >= 0;)
        {
            auto original = createRandomData (r);
            auto asBase64 = Base64::toBase64 (original.getData(), original.getSize());
            MemoryOutputStream out;
            expect (Base64::convertFromBase64 (out, asBase64));
            auto result = out.getMemoryBlock();
            expect (result == original);
        }
    }
};

static Base64Tests base64Tests;

#endif

} // namespace drx
