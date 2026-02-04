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

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             CryptographyDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Encrypts and decrypts data.

 dependencies:     drx_core, drx_cryptography, drx_data_structures, drx_events,
                   drx_graphics, drx_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        CryptographyDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class RSAComponent final : public Component
{
public:
    RSAComponent()
    {
        addAndMakeVisible (rsaGroup);

        addAndMakeVisible (bitSize);
        bitSize.setText (Txt (256));
        bitSizeLabel.attachToComponent (&bitSize, true);

        addAndMakeVisible (generateRSAButton);
        generateRSAButton.onClick = [this] { createRSAKey(); };

        addAndMakeVisible (rsaResultBox);
        rsaResultBox.setReadOnly (true);
        rsaResultBox.setMultiLine (true);
    }

    z0 resized() override
    {
        auto area = getLocalBounds();
        rsaGroup.setBounds (area);
        area.removeFromTop (10);
        area.reduce (5, 5);

        auto topArea = area.removeFromTop (34);
        topArea.removeFromLeft (110);
        bitSize.setBounds (topArea.removeFromLeft (topArea.getWidth() / 2).reduced (5));
        generateRSAButton.setBounds (topArea.reduced (5));

        rsaResultBox.setBounds (area.reduced (5));
    }

private:
    z0 createRSAKey()
    {
        auto bits = jlimit (32, 1024, bitSize.getText().getIntValue());
        bitSize.setText (Txt (bits), dontSendNotification);

        // Create a key-pair...
        RSAKey publicKey, privateKey;
        RSAKey::createKeyPair (publicKey, privateKey, bits);

        // Test the new key on a piece of data...
        BigInteger testValue;
        testValue.parseString ("1234567890abcdef", 16);

        auto encodedValue = testValue;
        publicKey.applyToValue (encodedValue);

        auto decodedValue = encodedValue;
        privateKey.applyToValue (decodedValue);

        // ..and show the results..
        Txt message;
        message << "Number of bits: " << bits << newLine
                << "Public Key: "  << publicKey .toString() << newLine
                << "Private Key: " << privateKey.toString() << newLine
                << newLine
                << "Test input: " << testValue.toString (16) << newLine
                << "Encoded: " << encodedValue.toString (16) << newLine
                << "Decoded: " << decodedValue.toString (16) << newLine;

        rsaResultBox.setText (message, false);
    }

    GroupComponent rsaGroup       { {}, "RSA Encryption" };
    TextButton generateRSAButton  { "Generate RSA" };
    Label bitSizeLabel            { {}, "Num Bits to Use:" };
    TextEditor bitSize, rsaResultBox;

    z0 lookAndFeelChanged() override
    {
        rsaGroup.setColor (GroupComponent::outlineColorId,
                            getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::outline,
                                                    Colors::grey));
        rsaGroup.setColor (GroupComponent::textColorId,
                            getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::defaultText,
                                                    Colors::white));
        rsaResultBox.setColor (TextEditor::backgroundColorId,
                                getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::widgetBackground,
                                                        Colors::white.withAlpha (0.5f)));

        bitSize.applyFontToAllText (bitSize.getFont());
        rsaResultBox.applyFontToAllText (rsaResultBox.getFont());
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSAComponent)
};

//==============================================================================
class HashesComponent final : public Component
{
public:
    HashesComponent()
    {
        addAndMakeVisible (hashGroup);

        addAndMakeVisible (hashEntryBox);
        hashEntryBox.setMultiLine (true);

        hashEntryBox.setReturnKeyStartsNewLine (true);
        hashEntryBox.setText ("Type some text in this box and the resulting MD5, SHA and Whirlpool hashes will update below");

        auto updateHashes = [this]
        {
            auto text = hashEntryBox.getText();

            updateMD5Result       (text.toUTF8());
            updateSHA256Result    (text.toUTF8());
            updateWhirlpoolResult (text.toUTF8());
        };

        hashEntryBox.onTextChange = updateHashes;
        hashEntryBox.onReturnKey  = updateHashes;

        hashLabel1.attachToComponent (&hashEntryBox,    true);
        hashLabel2.attachToComponent (&md5Result,       true);
        hashLabel3.attachToComponent (&shaResult,       true);
        hashLabel4.attachToComponent (&whirlpoolResult, true);

        addAndMakeVisible (md5Result);
        addAndMakeVisible (shaResult);
        addAndMakeVisible (whirlpoolResult);

        updateHashes();
    }

    z0 updateMD5Result (CharPointer_UTF8 text)
    {
        md5Result.setText (MD5 (text).toHexString(), dontSendNotification);
    }

    z0 updateSHA256Result (CharPointer_UTF8 text)
    {
        shaResult.setText (SHA256 (text).toHexString(), dontSendNotification);
    }

    z0 updateWhirlpoolResult (CharPointer_UTF8 text)
    {
        whirlpoolResult.setText (Whirlpool (text).toHexString(), dontSendNotification);
    }

    z0 resized() override
    {
        auto area = getLocalBounds();

        hashGroup.setBounds (area);

        area.removeFromLeft (120);
        area.removeFromTop (10);
        area.reduce (5, 5);

        whirlpoolResult.setBounds (area.removeFromBottom (34));
        shaResult      .setBounds (area.removeFromBottom (34));
        md5Result      .setBounds (area.removeFromBottom (34));
        hashEntryBox   .setBounds (area.reduced (5));
    }

private:
    GroupComponent hashGroup { {}, "Hashes" };
    TextEditor hashEntryBox;
    Label md5Result, shaResult, whirlpoolResult;

    Label hashLabel1  { {}, "Text to Hash:" };
    Label hashLabel2  { {}, "MD5 Result:" };
    Label hashLabel3  { {}, "SHA Result:" };
    Label hashLabel4  { {}, "Whirlpool Result:" };

    z0 lookAndFeelChanged() override
    {
        hashGroup.setColor (GroupComponent::outlineColorId,
                             getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::outline,
                                                     Colors::grey));
        hashGroup.setColor (GroupComponent::textColorId,
                             getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::defaultText,
                                                     Colors::white));
        hashEntryBox.setColor (TextEditor::backgroundColorId,
                                getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::widgetBackground,
                                                        Colors::white.withAlpha (0.5f)));

        hashEntryBox.applyFontToAllText (hashEntryBox.getFont());
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HashesComponent)
};

//==============================================================================
class CryptographyDemo final : public Component
{
public:
    CryptographyDemo()
    {
        addAndMakeVisible (rsaDemo);
        addAndMakeVisible (hashDemo);

        setSize (750, 750);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Color::greyLevel (0.4f)));
    }

    z0 resized() override
    {
        auto area = getLocalBounds();

        rsaDemo .setBounds (area.removeFromTop (getHeight() / 2).reduced (5));
        hashDemo.setBounds (area.reduced (5));
    }

private:
    RSAComponent rsaDemo;
    HashesComponent hashDemo;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CryptographyDemo)
};
