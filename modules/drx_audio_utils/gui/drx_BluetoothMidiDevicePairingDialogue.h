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
    Opens a Bluetooth MIDI pairing dialogue that allows the user to view and
    connect to Bluetooth MIDI devices that are currently found nearby.

    The dialogue will ignore non-MIDI Bluetooth devices.

    Only after a Bluetooth MIDI device has been paired will its MIDI ports
    be available through DRX's MidiInput and MidiOutput classes.

    This dialogue is currently only available on macOS targeting versions 10.11+,
    iOS and Android. When targeting older versions of macOS you should instead
    pair Bluetooth MIDI devices using the "Audio MIDI Setup" app (located in
    /Applications/Utilities). On Windows, you should use the system settings. On
    Linux, Bluetooth MIDI devices are currently not supported.

    @tags{Audio}
*/
class DRX_API BluetoothMidiDevicePairingDialogue
{
public:

    /** Opens the Bluetooth MIDI pairing dialogue, if it is available.

        @param  exitCallback A callback which will be called when the modal
                bluetooth dialog is closed.
        @param  btWindowBounds The bounds of the bluetooth window that will
                be opened. The dialog itself is opened by the OS so cannot
                be customised by DRX.
        @return true if the dialogue was opened, false on error.

        @see ModalComponentManager::Callback
    */
    static b8 open (ModalComponentManager::Callback* exitCallback = nullptr,
                      Rectangle<i32>* btWindowBounds = nullptr);

    /** Checks if a Bluetooth MIDI pairing dialogue is available on this
        platform.

        On iOS, this will be true for iOS versions 8.0 and higher.

        On Android, this will be true only for Android SDK versions 23 and
        higher, and additionally only if the device itself supports MIDI
        over Bluetooth.

        On desktop platforms, this will typically be false as the bluetooth
        pairing is not done inside the app but by other means.

        @return true if the Bluetooth MIDI pairing dialogue is available,
                false otherwise.
    */
    static b8 isAvailable();
};

} // namespace drx
