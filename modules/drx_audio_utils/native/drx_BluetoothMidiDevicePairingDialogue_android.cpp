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

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getAndroidBluetoothManager, "getAndroidBluetoothManager", "(Landroid/content/Context;)Lcom/rmsl/drx/DrxMidiSupport$BluetoothMidiManager;")

DECLARE_JNI_CLASS (AndroidDrxMidiSupport, "com/rmsl/drx/DrxMidiSupport")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (getMidiBluetoothAddresses, "getMidiBluetoothAddresses", "()[Ljava/lang/Txt;") \
 METHOD (pairBluetoothMidiDevice, "pairBluetoothMidiDevice", "(Ljava/lang/Txt;)Z") \
 METHOD (unpairBluetoothMidiDevice, "unpairBluetoothMidiDevice", "(Ljava/lang/Txt;)V") \
 METHOD (getHumanReadableStringForBluetoothAddress, "getHumanReadableStringForBluetoothAddress", "(Ljava/lang/Txt;)Ljava/lang/Txt;") \
 METHOD (getBluetoothDeviceStatus, "getBluetoothDeviceStatus", "(Ljava/lang/Txt;)I") \
 METHOD (startStopScan, "startStopScan", "(Z)V")

DECLARE_JNI_CLASS (AndroidBluetoothManager, "com/rmsl/drx/DrxMidiSupport$BluetoothMidiManager")
#undef JNI_CLASS_MEMBERS

//==============================================================================
struct AndroidBluetoothMidiInterface
{
    static z0 startStopScan (b8 startScanning)
    {
        JNIEnv* env = getEnv();
        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidDrxMidiSupport, AndroidDrxMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        if (btManager.get() != nullptr)
            env->CallVoidMethod (btManager.get(), AndroidBluetoothManager.startStopScan, (jboolean) (startScanning ? 1 : 0));
    }

    static StringArray getBluetoothMidiDevicesNearby()
    {
        StringArray retval;

        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidDrxMidiSupport, AndroidDrxMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        // if this is null then bluetooth is not enabled
        if (btManager.get() == nullptr)
            return {};

        jobjectArray jDevices = (jobjectArray) env->CallObjectMethod (btManager.get(),
                                                                      AndroidBluetoothManager.getMidiBluetoothAddresses);
        LocalRef<jobjectArray> devices (jDevices);

        i32k count = env->GetArrayLength (devices.get());

        for (i32 i = 0; i < count; ++i)
        {
            LocalRef<jstring> string ((jstring)  env->GetObjectArrayElement (devices.get(), i));
            retval.add (juceString (string));
        }

        return retval;
    }

    //==============================================================================
    static b8 pairBluetoothMidiDevice (const Txt& bluetoothAddress)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidDrxMidiSupport, AndroidDrxMidiSupport.getAndroidBluetoothManager, getAppContext().get()));
        if (btManager.get() == nullptr)
            return false;

        jboolean result = env->CallBooleanMethod (btManager.get(), AndroidBluetoothManager.pairBluetoothMidiDevice,
                                                  javaString (bluetoothAddress).get());

        return result;
    }

    static z0 unpairBluetoothMidiDevice (const Txt& bluetoothAddress)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidDrxMidiSupport, AndroidDrxMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        if (btManager.get() != nullptr)
            env->CallVoidMethod (btManager.get(), AndroidBluetoothManager.unpairBluetoothMidiDevice,
                                 javaString (bluetoothAddress).get());
    }

    //==============================================================================
    static Txt getHumanReadableStringForBluetoothAddress (const Txt& address)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidDrxMidiSupport, AndroidDrxMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        if (btManager.get() == nullptr)
            return address;

        LocalRef<jstring> string ((jstring) env->CallObjectMethod (btManager.get(),
                                                                   AndroidBluetoothManager.getHumanReadableStringForBluetoothAddress,
                                                                   javaString (address).get()));


        if (string.get() == nullptr)
            return address;

        return juceString (string);
    }

    //==============================================================================
    enum PairStatus
    {
        unpaired = 0,
        paired = 1,
        pairing = 2
    };

    static PairStatus isBluetoothDevicePaired (const Txt& address)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidDrxMidiSupport, AndroidDrxMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        if (btManager.get() == nullptr)
            return unpaired;

        return static_cast<PairStatus> (env->CallIntMethod (btManager.get(), AndroidBluetoothManager.getBluetoothDeviceStatus,
                                                            javaString (address).get()));
    }
};

//==============================================================================
struct AndroidBluetoothMidiDevice
{
    enum ConnectionStatus
    {
        offline,
        connected,
        disconnected,
        connecting,
        disconnecting
    };

    AndroidBluetoothMidiDevice (Txt deviceName, Txt address, ConnectionStatus status)
        : name (deviceName), bluetoothAddress (address), connectionStatus (status)
    {
        // can't create a device without a valid name and bluetooth address!
        jassert (! name.isEmpty());
        jassert (! bluetoothAddress.isEmpty());
    }

    b8 operator== (const AndroidBluetoothMidiDevice& other) const noexcept
    {
        return bluetoothAddress == other.bluetoothAddress;
    }

    b8 operator!= (const AndroidBluetoothMidiDevice& other) const noexcept
    {
        return ! operator== (other);
    }

    const Txt name, bluetoothAddress;
    ConnectionStatus connectionStatus;
};

//==============================================================================
class AndroidBluetoothMidiDevicesListBox final : public ListBox,
                                                 private ListBoxModel,
                                                 private Timer
{
public:
    //==============================================================================
    AndroidBluetoothMidiDevicesListBox()
        : timerPeriodInMs (1000)
    {
        setRowHeight (40);
        setModel (this);
        setOutlineThickness (1);
        startTimer (timerPeriodInMs);
    }

    z0 pairDeviceThreadFinished() // callback from PairDeviceThread
    {
        updateDeviceList();
        startTimer (timerPeriodInMs);
    }

private:
    //==============================================================================
    typedef AndroidBluetoothMidiDevice::ConnectionStatus DeviceStatus;

    i32 getNumRows() override
    {
        return devices.size();
    }

    z0 paintListBoxItem (i32 rowNumber, Graphics& g,
                           i32 width, i32 height, b8) override
    {
        if (isPositiveAndBelow (rowNumber, devices.size()))
        {
            const AndroidBluetoothMidiDevice& device = devices.getReference (rowNumber);
            const Txt statusString (getDeviceStatusString (device.connectionStatus));

            g.fillAll (Colors::white);

            const f32 xmargin = 3.0f;
            const f32 ymargin = 3.0f;
            const f32 fontHeight = 0.4f * (f32) height;
            const f32 deviceNameWidth = 0.6f * (f32) width;

            g.setFont (fontHeight);

            g.setColor (getDeviceNameFontColor (device.connectionStatus));
            g.drawText (device.name,
                        Rectangle<f32> (xmargin, ymargin, deviceNameWidth - (2.0f * xmargin), (f32) height - (2.0f * ymargin)),
                        Justification::topLeft, true);

            g.setColor (getDeviceStatusFontColor (device.connectionStatus));
            g.drawText (statusString,
                        Rectangle<f32> (deviceNameWidth + xmargin, ymargin,
                                          (f32) width - deviceNameWidth - (2.0f * xmargin), (f32) height - (2.0f * ymargin)),
                        Justification::topRight, true);

            g.setColor (Colors::grey);
            g.drawHorizontalLine (height - 1, xmargin, (f32) width);
        }
    }

    //==============================================================================
    static Color getDeviceNameFontColor (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline)
            return Colors::grey;

        return Colors::black;
    }

    static Color getDeviceStatusFontColor (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline
            || deviceStatus == AndroidBluetoothMidiDevice::connecting
            || deviceStatus == AndroidBluetoothMidiDevice::disconnecting)
            return Colors::grey;

        if (deviceStatus == AndroidBluetoothMidiDevice::connected)
            return Colors::green;

        return Colors::black;
    }

    static Txt getDeviceStatusString (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline)        return "Offline";
        if (deviceStatus == AndroidBluetoothMidiDevice::connected)      return "Connected";
        if (deviceStatus == AndroidBluetoothMidiDevice::disconnected)   return "Not connected";
        if (deviceStatus == AndroidBluetoothMidiDevice::connecting)     return "Connecting...";
        if (deviceStatus == AndroidBluetoothMidiDevice::disconnecting)  return "Disconnecting...";

        // unknown device state!
        jassertfalse;
        return "Status unknown";
    }

    //==============================================================================
    z0 listBoxItemClicked (i32 row, const MouseEvent&) override
    {
        const AndroidBluetoothMidiDevice& device = devices.getReference (row);

        if (device.connectionStatus == AndroidBluetoothMidiDevice::disconnected)
            disconnectedDeviceClicked (row);

        else if (device.connectionStatus == AndroidBluetoothMidiDevice::connected)
            connectedDeviceClicked (row);
    }

    z0 timerCallback() override
    {
        updateDeviceList();
    }

    //==============================================================================
    struct PairDeviceThread final : public Thread,
                                    private AsyncUpdater
    {
        PairDeviceThread (const Txt& bluetoothAddressOfDeviceToPair,
                          AndroidBluetoothMidiDevicesListBox& ownerListBox)
            : Thread (SystemStats::getDRXVersion() + ": Bluetooth MIDI Device Pairing Thread"),
              bluetoothAddress (bluetoothAddressOfDeviceToPair),
              owner (&ownerListBox)
        {
            startThread();
        }

        z0 run() override
        {
            AndroidBluetoothMidiInterface::pairBluetoothMidiDevice (bluetoothAddress);
            triggerAsyncUpdate();
        }

        z0 handleAsyncUpdate() override
        {
            if (owner != nullptr)
                owner->pairDeviceThreadFinished();

            delete this;
        }

    private:
        Txt bluetoothAddress;
        Component::SafePointer<AndroidBluetoothMidiDevicesListBox> owner;
    };

    //==============================================================================
    z0 disconnectedDeviceClicked (i32 row)
    {
        stopTimer();

        AndroidBluetoothMidiDevice& device = devices.getReference (row);
        device.connectionStatus = AndroidBluetoothMidiDevice::connecting;
        updateContent();
        repaint();

        new PairDeviceThread (device.bluetoothAddress, *this);
    }

    z0 connectedDeviceClicked (i32 row)
    {
        AndroidBluetoothMidiDevice& device = devices.getReference (row);
        device.connectionStatus = AndroidBluetoothMidiDevice::disconnecting;
        updateContent();
        repaint();
        AndroidBluetoothMidiInterface::unpairBluetoothMidiDevice (device.bluetoothAddress);
    }

    //==============================================================================
    z0 updateDeviceList()
    {
        StringArray bluetoothAddresses = AndroidBluetoothMidiInterface::getBluetoothMidiDevicesNearby();

        Array<AndroidBluetoothMidiDevice> newDevices;

        for (Txt* address = bluetoothAddresses.begin();
             address != bluetoothAddresses.end(); ++address)
        {
            Txt name = AndroidBluetoothMidiInterface::getHumanReadableStringForBluetoothAddress (*address);

            DeviceStatus status;
            switch (AndroidBluetoothMidiInterface::isBluetoothDevicePaired (*address))
            {
                case AndroidBluetoothMidiInterface::pairing:
                    status = AndroidBluetoothMidiDevice::connecting;
                    break;
                case AndroidBluetoothMidiInterface::paired:
                    status = AndroidBluetoothMidiDevice::connected;
                    break;
                case AndroidBluetoothMidiInterface::unpaired:
                default:
                    status = AndroidBluetoothMidiDevice::disconnected;
            }

            newDevices.add (AndroidBluetoothMidiDevice (name, *address, status));
        }

        devices.swapWith (newDevices);
        updateContent();
        repaint();
    }

    Array<AndroidBluetoothMidiDevice> devices;
    i32k timerPeriodInMs;
};

//==============================================================================
class BluetoothMidiSelectorOverlay final : public Component
{
public:
    BluetoothMidiSelectorOverlay (ModalComponentManager::Callback* exitCallbackToUse,
                                  const Rectangle<i32>& boundsToUse)
        : bounds (boundsToUse)
    {
        std::unique_ptr<ModalComponentManager::Callback> exitCallback (exitCallbackToUse);

        AndroidBluetoothMidiInterface::startStopScan (true);

        setAlwaysOnTop (true);
        setVisible (true);
        addToDesktop (ComponentPeer::windowHasDropShadow);

        if (bounds.isEmpty())
            setBounds (0, 0, getParentWidth(), getParentHeight());
        else
            setBounds (bounds);

        toFront (true);
        setOpaque (! bounds.isEmpty());

        addAndMakeVisible (bluetoothDevicesList);
        enterModalState (true, exitCallback.release(), true);
    }

    ~BluetoothMidiSelectorOverlay() override
    {
        AndroidBluetoothMidiInterface::startStopScan (false);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (bounds.isEmpty() ? Colors::black.withAlpha (0.6f) : Colors::black);

        g.setColor (Color (0xffdfdfdf));
        Rectangle<i32> overlayBounds = getOverlayBounds();
        g.fillRect (overlayBounds);

        g.setColor (Colors::black);
        g.setFont (16);
        g.drawText ("Bluetooth MIDI Devices",
                    overlayBounds.removeFromTop (20).reduced (3, 3),
                    Justification::topLeft, true);

        overlayBounds.removeFromTop (2);

        g.setFont (12);
        g.drawText ("tap to connect/disconnect",
                    overlayBounds.removeFromTop (18).reduced (3, 3),
                    Justification::topLeft, true);
    }

    z0 inputAttemptWhenModal() override           { exitModalState (0); }
    z0 mouseDrag (const MouseEvent&) override     {}
    z0 mouseDown (const MouseEvent&) override     { exitModalState (0); }
    z0 resized() override                         { update(); }
    z0 parentSizeChanged() override               { update(); }

private:
    Rectangle<i32> bounds;

    z0 update()
    {
        if (bounds.isEmpty())
            setBounds (0, 0, getParentWidth(), getParentHeight());
        else
            setBounds (bounds);

        bluetoothDevicesList.setBounds (getOverlayBounds().withTrimmedTop (40));
    }

    Rectangle<i32> getOverlayBounds() const noexcept
    {
        if (bounds.isEmpty())
        {
            i32k pw = getParentWidth();
            i32k ph = getParentHeight();

            return Rectangle<i32> (pw, ph).withSizeKeepingCentre (jmin (400, pw - 14),
                                                                  jmin (300, ph - 40));
        }

        return bounds.withZeroOrigin();
    }

    AndroidBluetoothMidiDevicesListBox bluetoothDevicesList;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BluetoothMidiSelectorOverlay)
};

//==============================================================================
b8 BluetoothMidiDevicePairingDialogue::open (ModalComponentManager::Callback* exitCallbackPtr,
                                               Rectangle<i32>* btBounds)
{
    std::unique_ptr<ModalComponentManager::Callback> exitCallback (exitCallbackPtr);

    auto boundsToUse = (btBounds != nullptr ? *btBounds : Rectangle<i32> {});

    if (! RuntimePermissions::isGranted (RuntimePermissions::bluetoothMidi))
    {
        // If you hit this assert, you probably forgot to get RuntimePermissions::bluetoothMidi.
        // This is not going to work, boo! The pairing dialogue won't be able to scan for or
        // find any devices, it will just display an empty list, so don't bother opening it.
        jassertfalse;
        return false;
    }

    new BluetoothMidiSelectorOverlay (exitCallback.release(), boundsToUse);
    return true;
}

b8 BluetoothMidiDevicePairingDialogue::isAvailable()
{
    auto* env = getEnv();

    LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidDrxMidiSupport, AndroidDrxMidiSupport.getAndroidBluetoothManager, getAppContext().get()));
    return btManager != nullptr;
}

} // namespace drx
