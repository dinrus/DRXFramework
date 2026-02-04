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

 name:             InAppPurchasesDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Showcases in-app purchases features. To run this demo you must enable the
                   "In-App Purchases Capability" option in the Projucer exporter.

 dependencies:     drx_audio_basics, drx_audio_devices, drx_audio_formats,
                   drx_audio_processors, drx_audio_utils, drx_core,
                   drx_cryptography, drx_data_structures, drx_events,
                   drx_graphics, drx_gui_basics, drx_gui_extra,
                   drx_product_unlocking
 exporters:        xcode_mac, xcode_iphone, androidstudio

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1
                   DRX_IN_APP_PURCHASES=1

 type:             Component
 mainClass:        InAppPurchasesDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

/*
    To finish the setup of this demo, do the following in the Projucer project:

    1. In the project settings, set the "Bundle Identifier" to com.rmsl.juceInAppPurchaseSample
    2. In the Android exporter settings, change the following settings:
         - "In-App Billing" - Enabled
         - "Key Signing: key.store" - path to InAppPurchase.keystore file in examples/Assets/Signing
         - "Key Signing: key.store.password" - amazingvoices
         - "Key Signing: key-alias" - InAppPurchase
         - "Key Signing: key.alias.password" - amazingvoices
    3. Re-save the project
*/

//==============================================================================
class VoicePurchases final : private InAppPurchases::Listener
{
public:
    //==============================================================================
    struct VoiceProduct
    {
        tukk identifier;
        tukk humanReadable;
        b8 isPurchased, priceIsKnown, purchaseInProgress;
        Txt purchasePrice;
    };

    //==============================================================================
    VoicePurchases (AsyncUpdater& asyncUpdater)
         : guiUpdater (asyncUpdater)
    {
        voiceProducts = Array<VoiceProduct>(
                        { VoiceProduct {"robot",  "Robot",  true,   true,  false, "Free" },
                          VoiceProduct {"jules",  "Jules",  false,  false, false, "Retrieving price..." },
                          VoiceProduct {"fabian", "Fabian", false,  false, false, "Retrieving price..." },
                          VoiceProduct {"ed",     "Ed",     false,  false, false, "Retrieving price..." },
                          VoiceProduct {"lukasz", "Lukasz", false,  false, false, "Retrieving price..." },
                          VoiceProduct {"jb",     "JB",     false,  false, false, "Retrieving price..." } });
    }

    ~VoicePurchases() override
    {
        InAppPurchases::getInstance()->removeListener (this);
    }

    //==============================================================================
    VoiceProduct getPurchase (i32 voiceIndex)
    {
        if (! havePurchasesBeenRestored)
        {
            havePurchasesBeenRestored = true;
            InAppPurchases::getInstance()->addListener (this);

            InAppPurchases::getInstance()->restoreProductsBoughtList (true);
        }

        return voiceProducts[voiceIndex];
    }

    z0 purchaseVoice (i32 voiceIndex)
    {
        if (havePricesBeenFetched && isPositiveAndBelow (voiceIndex, voiceProducts.size()))
        {
            auto& product = voiceProducts.getReference (voiceIndex);

            if (! product.isPurchased)
            {
                purchaseInProgress = true;

                product.purchaseInProgress = true;
                InAppPurchases::getInstance()->purchaseProduct (product.identifier);

                guiUpdater.triggerAsyncUpdate();
            }
        }
    }

    StringArray getVoiceNames() const
    {
        StringArray names;

        for (auto& voiceProduct : voiceProducts)
            names.add (voiceProduct.humanReadable);

        return names;
    }

    b8 isPurchaseInProgress() const noexcept { return purchaseInProgress; }

private:
    //==============================================================================
    z0 productsInfoReturned (const Array<InAppPurchases::Product>& products) override
    {
        if (! InAppPurchases::getInstance()->isInAppPurchasesSupported())
        {
            for (auto idx = 1; idx < voiceProducts.size(); ++idx)
            {
                auto& voiceProduct = voiceProducts.getReference (idx);

                voiceProduct.isPurchased  = false;
                voiceProduct.priceIsKnown = false;
                voiceProduct.purchasePrice = "In-App purchases unavailable";
            }

            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             "In-app purchase is unavailable!",
                                                             "In-App purchases are not available. This either means you are trying "
                                                             "to use IAP on a platform that does not support IAP or you haven't setup "
                                                             "your app correctly to work with IAP.",
                                                             "OK");
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
        else
        {
            for (auto product : products)
            {
                auto idx = findVoiceIndexFromIdentifier (product.identifier);

                if (isPositiveAndBelow (idx, voiceProducts.size()))
                {
                    auto& voiceProduct = voiceProducts.getReference (idx);

                    voiceProduct.priceIsKnown = true;
                    voiceProduct.purchasePrice = product.price;
                }
            }

            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             "Your credit card will be charged!",
                                                             "You are running the sample code for DRX In-App purchases. "
                                                             "Although this is only sample code, it will still CHARGE YOUR CREDIT CARD!",
                                                             "Understood!");
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }

        guiUpdater.triggerAsyncUpdate();
    }

    z0 productPurchaseFinished (const PurchaseInfo& info, b8 success, const Txt& error) override
    {
        purchaseInProgress = false;

        for (const auto& productId : info.purchase.productIds)
        {
            auto idx = findVoiceIndexFromIdentifier (productId);

            if (isPositiveAndBelow (idx, voiceProducts.size()))
            {
                auto& voiceProduct = voiceProducts.getReference (idx);

                voiceProduct.isPurchased = success;
                voiceProduct.purchaseInProgress = false;
            }
            else
            {
                // On failure Play Store will not tell us which purchase failed
                for (auto& voiceProduct : voiceProducts)
                    voiceProduct.purchaseInProgress = false;
            }
        }

        if (! success)
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon, "Purchase failed", error);
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }

        guiUpdater.triggerAsyncUpdate();
    }

    z0 purchasesListRestored (const Array<PurchaseInfo>& infos, b8 success, const Txt&) override
    {
        if (success)
        {
            for (const auto& info : infos)
            {
                for (const auto& productId : info.purchase.productIds)
                {
                    auto idx = findVoiceIndexFromIdentifier (productId);

                    if (isPositiveAndBelow (idx, voiceProducts.size()))
                    {
                        auto& voiceProduct = voiceProducts.getReference (idx);

                        voiceProduct.isPurchased = true;
                    }
                }
            }

            guiUpdater.triggerAsyncUpdate();
        }

        if (! havePricesBeenFetched)
        {
            havePricesBeenFetched = true;
            StringArray identifiers;

            for (const auto& voiceProduct : voiceProducts)
                identifiers.add (voiceProduct.identifier);

            InAppPurchases::getInstance()->getProductsInformation (identifiers);
        }
    }

    //==============================================================================
    i32 findVoiceIndexFromIdentifier (Txt identifier) const
    {
        identifier = identifier.toLowerCase();

        for (auto i = 0; i < voiceProducts.size(); ++i)
            if (Txt (voiceProducts.getReference (i).identifier) == identifier)
                return i;

        return -1;
    }

    //==============================================================================
    AsyncUpdater& guiUpdater;
    b8 havePurchasesBeenRestored = false, havePricesBeenFetched = false, purchaseInProgress = false;
    Array<VoiceProduct> voiceProducts;
    ScopedMessageBox messageBox;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoicePurchases)
};

//==============================================================================
class PhraseModel final : public ListBoxModel
{
public:
    PhraseModel() {}

    i32 getNumRows() override    { return phrases.size(); }

    z0 paintListBoxItem (i32 row, Graphics& g, i32 w, i32 h, b8 isSelected) override
    {
        Rectangle<i32> r (0, 0, w, h);

        auto& lf = Desktop::getInstance().getDefaultLookAndFeel();
        g.setColor (lf.findColor (isSelected ? (i32) TextEditor::highlightColorId : (i32) ListBox::backgroundColorId));
        g.fillRect (r);

        g.setColor (lf.findColor (ListBox::textColorId));

        g.setFont (18);

        Txt phrase = (isPositiveAndBelow (row, phrases.size()) ? phrases[row] : Txt{});
        g.drawText (phrase, 10, 0, w, h, Justification::centredLeft);
    }

private:
    StringArray phrases {"I love DRX!", "The five dimensions of touch", "Make it fast!"};

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhraseModel)
};

//==============================================================================
class VoiceModel final : public ListBoxModel
{
public:
    //==============================================================================
    class VoiceRow final : public Component,
                           private Timer
    {
    public:
        VoiceRow (VoicePurchases& voicePurchases) : purchases (voicePurchases)
        {
            addAndMakeVisible (nameLabel);
            addAndMakeVisible (purchaseButton);
            addAndMakeVisible (priceLabel);

            purchaseButton.onClick = [this] { clickPurchase(); };

            voices = purchases.getVoiceNames();

            setSize (600, 33);
        }

        z0 paint (Graphics& g) override
        {
            auto r = getLocalBounds().reduced (4);
            {
                auto voiceIconBounds = r.removeFromLeft (r.getHeight());
                g.setColor (Colors::black);
                g.drawRect (voiceIconBounds);

                voiceIconBounds.reduce (1, 1);
                g.setColor (hasBeenPurchased ? Colors::white : Colors::grey);
                g.fillRect (voiceIconBounds);

                g.drawImage (avatar, voiceIconBounds.toFloat());

                if (! hasBeenPurchased)
                {
                    g.setColor (Colors::white.withAlpha (0.8f));
                    g.fillRect (voiceIconBounds);

                    if (purchaseInProgress)
                        getLookAndFeel().drawSpinningWaitAnimation (g, Colors::darkgrey,
                                                                    voiceIconBounds.getX(),
                                                                    voiceIconBounds.getY(),
                                                                    voiceIconBounds.getWidth(),
                                                                    voiceIconBounds.getHeight());
                }
            }
        }

        z0 resized() override
        {
            auto r = getLocalBounds().reduced (4 + 8, 4);
            auto h = r.getHeight();
            auto w = static_cast<i32> (h * 1.5);

            r.removeFromLeft (h);
            purchaseButton.setBounds (r.removeFromRight (w).withSizeKeepingCentre (w, h / 2));

            nameLabel.setBounds (r.removeFromTop (18));
            priceLabel.setBounds (r.removeFromTop (18));
        }

        z0 update (i32 rowNumber, b8 rowIsSelected)
        {
            isSelected  = rowIsSelected;
            rowSelected = rowNumber;

            if (isPositiveAndBelow (rowNumber, voices.size()))
            {
                auto imageResourceName = voices[rowNumber] + ".png";

                nameLabel.setText (voices[rowNumber], NotificationType::dontSendNotification);

                auto purchase = purchases.getPurchase (rowNumber);
                hasBeenPurchased = purchase.isPurchased;
                purchaseInProgress = purchase.purchaseInProgress;

                if (purchaseInProgress)
                    startTimer (1000 / 50);
                else
                    stopTimer();

                nameLabel.setFont (FontOptions { 16.0f, Font::bold | (hasBeenPurchased ? 0 : Font::italic) });
                nameLabel.setColor (Label::textColorId, hasBeenPurchased ? Colors::white : Colors::grey);

                priceLabel.setFont (FontOptions { 10.0f, purchase.priceIsKnown ? 0 : Font::italic });
                priceLabel.setColor (Label::textColorId, hasBeenPurchased ? Colors::white : Colors::grey);
                priceLabel.setText (purchase.purchasePrice, NotificationType::dontSendNotification);

                if (rowNumber == 0)
                {
                    purchaseButton.setButtonText ("Internal");
                    purchaseButton.setEnabled (false);
                }
                else
                {
                    purchaseButton.setButtonText (hasBeenPurchased ? "Purchased" : "Purchase");
                    purchaseButton.setEnabled (! hasBeenPurchased && purchase.priceIsKnown);
                }

                setInterceptsMouseClicks (! hasBeenPurchased, ! hasBeenPurchased);

                if (auto fileStream = createAssetInputStream (Txt ("Purchases/" + Txt (imageResourceName)).toRawUTF8()))
                    avatar = PNGImageFormat().decodeImage (*fileStream);
            }
        }
    private:
        //==============================================================================
        z0 clickPurchase()
        {
            if (rowSelected >= 0)
            {
                if (! hasBeenPurchased)
                {
                    purchases.purchaseVoice (rowSelected);
                    purchaseInProgress = true;
                    startTimer (1000 / 50);
                }
            }
        }

        z0 timerCallback() override   { repaint(); }

        //==============================================================================
        b8 isSelected = false, hasBeenPurchased = false, purchaseInProgress = false;
        i32 rowSelected = -1;
        Image avatar;

        StringArray voices;

        VoicePurchases& purchases;

        Label nameLabel, priceLabel;
        TextButton purchaseButton {"Purchase"};

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceRow)
    };

    //==============================================================================
    VoiceModel (VoicePurchases& voicePurchases) : purchases (voicePurchases)
    {
        voiceProducts = purchases.getVoiceNames();
    }

    i32 getNumRows() override    { return voiceProducts.size(); }

    Component* refreshComponentForRow (i32 row, b8 selected, Component* existing) override
    {
        auto safePtr = rawToUniquePtr (existing);

        if (isPositiveAndBelow (row, voiceProducts.size()))
        {
            if (safePtr == nullptr)
                safePtr = std::make_unique<VoiceRow> (purchases);

            if (auto* voiceRow = dynamic_cast<VoiceRow*> (safePtr.get()))
                voiceRow->update (row, selected);

            return safePtr.release();
        }

        return nullptr;
    }

    z0 paintListBoxItem (i32, Graphics& g, i32 w, i32 h, b8 isSelected) override
    {
        auto r = Rectangle<i32> (0, 0, w, h).reduced (4);

        auto& lf = Desktop::getInstance().getDefaultLookAndFeel();
        g.setColor (lf.findColor (isSelected ? (i32) TextEditor::highlightColorId : (i32) ListBox::backgroundColorId));
        g.fillRect (r);
    }

private:
    StringArray voiceProducts;

    VoicePurchases& purchases;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceModel)
};

//==============================================================================
class InAppPurchasesDemo final : public Component,
                                 private AsyncUpdater
{
public:
    InAppPurchasesDemo()
    {
        manager.registerBasicFormats();

        Desktop::getInstance().getDefaultLookAndFeel().setUsingNativeAlertWindows (true);

        dm.addAudioCallback (&player);
        dm.initialiseWithDefaultDevices (0, 2);

        setOpaque (true);

        phraseListBox.setModel (phraseModel.get());
        voiceListBox .setModel (voiceModel.get());

        phraseListBox.setRowHeight (33);
        phraseListBox.selectRow (0);
        phraseListBox.updateContent();

        voiceListBox.setRowHeight (66);
        voiceListBox.selectRow (0);
        voiceListBox.updateContent();

        addAndMakeVisible (phraseLabel);
        addAndMakeVisible (phraseListBox);
        addAndMakeVisible (playStopButton);
        addAndMakeVisible (voiceLabel);
        addAndMakeVisible (voiceListBox);

        playStopButton.onClick = [this] { playStopPhrase(); };

        soundNames = purchases.getVoiceNames();

       #if DRX_ANDROID || DRX_IOS
        auto screenBounds = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
        setSize (screenBounds.getWidth(), screenBounds.getHeight());
       #else
        setSize (800, 600);
       #endif
    }

    ~InAppPurchasesDemo() override
    {
        dm.closeAudioDevice();
        dm.removeAudioCallback (&player);
    }

private:
    //==============================================================================
    z0 handleAsyncUpdate() override
    {
        voiceListBox.updateContent();
        voiceListBox.setEnabled (! purchases.isPurchaseInProgress());
        voiceListBox.repaint();
    }

    //==============================================================================
    z0 resized() override
    {
        auto r = getLocalBounds().reduced (20);

        {
            auto phraseArea = r.removeFromTop (r.getHeight() / 2);

            phraseLabel   .setBounds (phraseArea.removeFromTop (36).reduced (0, 10));
            playStopButton.setBounds (phraseArea.removeFromBottom (50).reduced (0, 10));
            phraseListBox .setBounds (phraseArea);
        }

        {
            auto voiceArea = r;

            voiceLabel  .setBounds (voiceArea.removeFromTop (36).reduced (0, 10));
            voiceListBox.setBounds (voiceArea);
        }
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (Desktop::getInstance().getDefaultLookAndFeel()
                      .findColor (ResizableWindow::backgroundColorId));
    }

    //==============================================================================
    z0 playStopPhrase()
    {
        auto idx = voiceListBox.getSelectedRow();
        if (isPositiveAndBelow (idx, soundNames.size()))
        {
            auto assetName = "Purchases/" + soundNames[idx] + Txt (phraseListBox.getSelectedRow()) + ".ogg";

            if (auto fileStream = createAssetInputStream (assetName.toRawUTF8()))
                if (auto* reader = manager.createReaderFor (std::move (fileStream)))
                    player.play (reader, true);
        }
    }

    //==============================================================================
    StringArray soundNames;

    Label phraseLabel                          { "phraseLabel", NEEDS_TRANS ("Phrases:") };
    ListBox phraseListBox                      { "phraseListBox" };
    std::unique_ptr<ListBoxModel> phraseModel  { new PhraseModel() };
    TextButton playStopButton                  { "Play" };

    SoundPlayer player;
    VoicePurchases purchases                   { *this };
    AudioDeviceManager dm;

    Label voiceLabel                           { "voiceLabel", NEEDS_TRANS ("Voices:") };
    ListBox voiceListBox                       { "voiceListBox" };
    std::unique_ptr<VoiceModel> voiceModel     { new VoiceModel (purchases) };

    AudioFormatManager manager;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InAppPurchasesDemo)
};
