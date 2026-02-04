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

#if ! TARGET_IPHONE_SIMULATOR

namespace drx
{

//==============================================================================
class BluetoothMidiSelectorOverlay final : public Component
{
public:
    BluetoothMidiSelectorOverlay (ModalComponentManager::Callback* exitCallbackToUse,
                                  const Rectangle<i32>& boundsToUse)
        : bounds (boundsToUse)
    {
        std::unique_ptr<ModalComponentManager::Callback> exitCallback (exitCallbackToUse);

        setAlwaysOnTop (true);
        setVisible (true);
        addToDesktop (ComponentPeer::windowHasDropShadow);

        if (bounds.isEmpty())
            setBounds (0, 0, getParentWidth(), getParentHeight());
        else
            setBounds (bounds);

        toFront (true);
        setOpaque (true);

        controller = [[CABTMIDICentralViewController alloc] init];
        nativeSelectorComponent.setView ([controller view]);

        addAndMakeVisible (nativeSelectorComponent);

        enterModalState (true, exitCallback.release(), true);
    }

    ~BluetoothMidiSelectorOverlay() override
    {
        nativeSelectorComponent.setView (nullptr);
        [controller release];
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (bounds.isEmpty() ? Colors::black.withAlpha (0.5f) : Colors::black);
    }

    z0 inputAttemptWhenModal() override           { close(); }
    z0 mouseDrag (const MouseEvent&) override     {}
    z0 mouseDown (const MouseEvent&) override     { close(); }
    z0 resized() override                         { update(); }
    z0 parentSizeChanged() override               { update(); }

private:
    z0 update()
    {
        if (bounds.isEmpty())
        {
            i32k pw = getParentWidth();
            i32k ph = getParentHeight();

            nativeSelectorComponent.setBounds (Rectangle<i32> (pw, ph)
                                                 .withSizeKeepingCentre (jmin (400, pw),
                                                                         jmin (450, ph - 40)));
        }
        else
        {
            nativeSelectorComponent.setBounds (bounds.withZeroOrigin());
        }
    }

    z0 close()
    {
        exitModalState (0);
        setVisible (false);
    }

    CABTMIDICentralViewController* controller;
    UIViewComponent nativeSelectorComponent;
    Rectangle<i32> bounds;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BluetoothMidiSelectorOverlay)
};

b8 BluetoothMidiDevicePairingDialogue::open (ModalComponentManager::Callback* exitCallback,
                                               Rectangle<i32>* btBounds)
{
    std::unique_ptr<ModalComponentManager::Callback> cb (exitCallback);
    auto boundsToUse = (btBounds != nullptr ? *btBounds : Rectangle<i32> {});

    if (isAvailable())
    {
        new BluetoothMidiSelectorOverlay (cb.release(), boundsToUse);
        return true;
    }

    return false;
}

b8 BluetoothMidiDevicePairingDialogue::isAvailable()
{
    return NSClassFromString (@"CABTMIDICentralViewController") != nil;
}

} // namespace drx

//==============================================================================
#else

namespace drx
{
    b8 BluetoothMidiDevicePairingDialogue::open (ModalComponentManager::Callback* exitCallback,
                                                   Rectangle<i32>*)
    {
        std::unique_ptr<ModalComponentManager::Callback> cb (exitCallback);
        return false;
    }

    b8 BluetoothMidiDevicePairingDialogue::isAvailable()  { return false; }
}

#endif
