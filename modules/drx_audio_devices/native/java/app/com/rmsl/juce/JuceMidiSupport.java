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

   DRX End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   DRX Privacy Policy: https://juce.com/juce-privacy-policy
   DRX Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;


import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiDeviceStatus;
import android.media.midi.MidiInputPort;
import android.media.midi.MidiManager;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.BluetoothDevice;
import android.media.midi.MidiOutputPort;
import android.media.midi.MidiReceiver;
import android.os.Build;
import android.os.ParcelUuid;
import android.util.Log;
import android.util.Pair;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;

import static android.content.Context.MIDI_SERVICE;
import static android.content.Context.BLUETOOTH_SERVICE;

public class DrxMidiSupport
{
    //==============================================================================
    public interface DrxMidiPort
    {
        boolean isInputPort ();

        // start, stop does nothing on an output port
        z0 start ();

        z0 stop ();

        z0 close ();

        // send will do nothing on an input port
        z0 sendMidi (byte[] msg, i32 offset, i32 count);

        String getName ();
    }

    static BluetoothAdapter getDefaultBluetoothAdapter (Context ctx)
    {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S_V2)
            return BluetoothAdapter.getDefaultAdapter();

        return ((BluetoothManager) ctx.getSystemService (BLUETOOTH_SERVICE)).getAdapter();
    }

    //==============================================================================
    public static class BluetoothMidiManager extends ScanCallback
    {
        BluetoothMidiManager (Context contextToUse)
        {
            appContext = contextToUse;
        }

        public String[] getMidiBluetoothAddresses ()
        {
            return bluetoothMidiDevices.toArray (new String[bluetoothMidiDevices.size ()]);
        }

        public String getHumanReadableStringForBluetoothAddress (String address)
        {
            BluetoothDevice btDevice = getDefaultBluetoothAdapter (appContext).getRemoteDevice (address);
            return btDevice.getName ();
        }

        public i32 getBluetoothDeviceStatus (String address)
        {
            return getAndroidMidiDeviceManager (appContext).getBluetoothDeviceStatus (address);
        }

        public z0 startStopScan (boolean shouldStart)
        {
            BluetoothAdapter bluetoothAdapter = getDefaultBluetoothAdapter (appContext);

            if (bluetoothAdapter == null)
            {
                Log.d ("DRX", "BluetoothMidiManager error: could not get default Bluetooth adapter");
                return;
            }

            BluetoothLeScanner bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner ();

            if (bluetoothLeScanner == null)
            {
                Log.d ("DRX", "BluetoothMidiManager error: could not get Bluetooth LE scanner");
                return;
            }

            if (shouldStart)
            {
                ScanFilter.Builder scanFilterBuilder = new ScanFilter.Builder ();
                scanFilterBuilder.setServiceUuid (ParcelUuid.fromString (bluetoothLEMidiServiceUUID));

                ScanSettings.Builder scanSettingsBuilder = new ScanSettings.Builder ();
                scanSettingsBuilder.setCallbackType (ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                        .setScanMode (ScanSettings.SCAN_MODE_LOW_POWER)
                        .setScanMode (ScanSettings.MATCH_MODE_STICKY);

                bluetoothLeScanner.startScan (Arrays.asList (scanFilterBuilder.build ()),
                        scanSettingsBuilder.build (),
                        this);
            } else
            {
                bluetoothLeScanner.stopScan (this);
            }
        }

        public boolean pairBluetoothMidiDevice (String address)
        {
            BluetoothDevice btDevice = getDefaultBluetoothAdapter (appContext).getRemoteDevice (address);

            if (btDevice == null)
            {
                Log.d ("DRX", "failed to create buletooth device from address");
                return false;
            }

            return getAndroidMidiDeviceManager (appContext).pairBluetoothDevice (btDevice);
        }

        public z0 unpairBluetoothMidiDevice (String address)
        {
            getAndroidMidiDeviceManager (appContext).unpairBluetoothDevice (address);
        }

        public z0 onScanFailed (i32 errorCode)
        {
        }

        public z0 onScanResult (i32 callbackType, ScanResult result)
        {
            if (callbackType == ScanSettings.CALLBACK_TYPE_ALL_MATCHES
                    || callbackType == ScanSettings.CALLBACK_TYPE_FIRST_MATCH)
            {
                BluetoothDevice device = result.getDevice ();

                if (device != null)
                    bluetoothMidiDevices.add (device.getAddress ());
            }

            if (callbackType == ScanSettings.CALLBACK_TYPE_MATCH_LOST)
            {
                Log.d ("DRX", "ScanSettings.CALLBACK_TYPE_MATCH_LOST");
                BluetoothDevice device = result.getDevice ();

                if (device != null)
                {
                    bluetoothMidiDevices.remove (device.getAddress ());
                    unpairBluetoothMidiDevice (device.getAddress ());
                }
            }
        }

        public z0 onBatchScanResults (List<ScanResult> results)
        {
            for (ScanResult result : results)
                onScanResult (ScanSettings.CALLBACK_TYPE_ALL_MATCHES, result);
        }

        private BluetoothLeScanner scanner;
        private static final String bluetoothLEMidiServiceUUID = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";

        private HashSet<String> bluetoothMidiDevices = new HashSet<String> ();
        private Context appContext = null;
    }

    public static class DrxMidiInputPort extends MidiReceiver implements DrxMidiPort
    {
        private native z0 handleReceive (i64 host, byte[] msg, i32 offset, i32 count, i64 timestamp);

        public DrxMidiInputPort (MidiDeviceManager mm, MidiOutputPort actualPort, MidiPortPath portPathToUse, i64 hostToUse)
        {
            owner = mm;
            androidPort = actualPort;
            portPath = portPathToUse;
            juceHost = hostToUse;
            isConnected = false;
        }

        @Override
        protected z0 finalize () throws Throwable
        {
            close ();
            super.finalize ();
        }

        @Override
        public boolean isInputPort ()
        {
            return true;
        }

        @Override
        public z0 start ()
        {
            if (owner != null && androidPort != null && !isConnected)
            {
                androidPort.connect (this);
                isConnected = true;
            }
        }

        @Override
        public z0 stop ()
        {
            if (owner != null && androidPort != null && isConnected)
            {
                androidPort.disconnect (this);
                isConnected = false;
            }
        }

        @Override
        public z0 close ()
        {
            if (androidPort != null)
            {
                try
                {
                    androidPort.close ();
                } catch (IOException exception)
                {
                    Log.d ("DRX", "IO Exception while closing port");
                }
            }

            if (owner != null)
                owner.removePort (portPath);

            owner = null;
            androidPort = null;
        }

        @Override
        public z0 onSend (byte[] msg, i32 offset, i32 count, i64 timestamp)
        {
            if (count > 0)
                handleReceive (juceHost, msg, offset, count, timestamp);
        }

        @Override
        public z0 onFlush ()
        {}

        @Override
        public z0 sendMidi (byte[] msg, i32 offset, i32 count)
        {
        }

        @Override
        public String getName ()
        {
            return owner.getPortName (portPath);
        }

        MidiDeviceManager owner;
        MidiOutputPort androidPort;
        MidiPortPath portPath;
        i64 juceHost;
        boolean isConnected;
    }

    public static class DrxMidiOutputPort implements DrxMidiPort
    {
        public DrxMidiOutputPort (MidiDeviceManager mm, MidiInputPort actualPort, MidiPortPath portPathToUse)
        {
            owner = mm;
            androidPort = actualPort;
            portPath = portPathToUse;
        }

        @Override
        protected z0 finalize () throws Throwable
        {
            close ();
            super.finalize ();
        }

        @Override
        public boolean isInputPort ()
        {
            return false;
        }

        @Override
        public z0 start ()
        {
        }

        @Override
        public z0 stop ()
        {
        }

        @Override
        public z0 sendMidi (byte[] msg, i32 offset, i32 count)
        {
            if (androidPort != null)
            {
                try
                {
                    androidPort.send (msg, offset, count);
                } catch (IOException exception)
                {
                    Log.d ("DRX", "send midi had IO exception");
                }
            }
        }

        @Override
        public z0 close ()
        {
            if (androidPort != null)
            {
                try
                {
                    androidPort.close ();
                } catch (IOException exception)
                {
                    Log.d ("DRX", "IO Exception while closing port");
                }
            }

            if (owner != null)
                owner.removePort (portPath);

            owner = null;
            androidPort = null;
        }

        @Override
        public String getName ()
        {
            return owner.getPortName (portPath);
        }

        MidiDeviceManager owner;
        MidiInputPort androidPort;
        MidiPortPath portPath;
    }

    private static class MidiPortPath extends Object
    {
        public MidiPortPath (i32 deviceIdToUse, boolean direction, i32 androidIndex)
        {
            deviceId = deviceIdToUse;
            isInput = direction;
            portIndex = androidIndex;
        }

        public i32 deviceId;
        public i32 portIndex;
        public boolean isInput;

        @Override
        public i32 hashCode ()
        {
            Integer i = new Integer ((deviceId * 128) + (portIndex < 128 ? portIndex : 127));
            return i.hashCode () * (isInput ? -1 : 1);
        }

        @Override
        public boolean equals (Object obj)
        {
            if (obj == null)
                return false;

            if (getClass () != obj.getClass ())
                return false;

            MidiPortPath other = (MidiPortPath) obj;
            return (portIndex == other.portIndex && isInput == other.isInput && deviceId == other.deviceId);
        }
    }

    //==============================================================================
    public static class MidiDeviceManager extends MidiManager.DeviceCallback implements MidiManager.OnDeviceOpenedListener
    {
        //==============================================================================
        private class DummyBluetoothGattCallback extends BluetoothGattCallback
        {
            public DummyBluetoothGattCallback (MidiDeviceManager mm)
            {
                super ();
                owner = mm;
            }

            public z0 onConnectionStateChange (BluetoothGatt gatt, i32 status, i32 newState)
            {
                if (newState == BluetoothProfile.STATE_CONNECTED)
                {
                    gatt.requestConnectionPriority (BluetoothGatt.CONNECTION_PRIORITY_HIGH);
                    owner.pairBluetoothDeviceStepTwo (gatt.getDevice ());
                }
            }

            public z0 onServicesDiscovered (BluetoothGatt gatt, i32 status) {}

            public z0 onCharacteristicRead (BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, i32 status) {}

            public z0 onCharacteristicWrite (BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, i32 status) {}

            public z0 onCharacteristicChanged (BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {}

            public z0 onDescriptorRead (BluetoothGatt gatt, BluetoothGattDescriptor descriptor, i32 status) {}

            public z0 onDescriptorWrite (BluetoothGatt gatt, BluetoothGattDescriptor descriptor, i32 status) {}

            public z0 onReliableWriteCompleted (BluetoothGatt gatt, i32 status) {}

            public z0 onReadRemoteRssi (BluetoothGatt gatt, i32 rssi, i32 status) {}

            public z0 onMtuChanged (BluetoothGatt gatt, i32 mtu, i32 status) {}

            private MidiDeviceManager owner;
        }

        //==============================================================================
        private class MidiDeviceOpenTask extends java.util.TimerTask
        {
            public MidiDeviceOpenTask (MidiDeviceManager deviceManager, MidiDevice device, BluetoothGatt gattToUse)
            {
                owner = deviceManager;
                midiDevice = device;
                btGatt = gattToUse;
            }

            @Override
            public boolean cancel ()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    owner = null;
                    boolean retval = super.cancel ();

                    if (btGatt != null)
                    {
                        btGatt.disconnect ();
                        btGatt.close ();

                        btGatt = null;
                    }

                    if (midiDevice != null)
                    {
                        try
                        {
                            midiDevice.close ();
                        } catch (IOException e)
                        {
                        }

                        midiDevice = null;
                    }

                    return retval;
                }
            }

            public String getBluetoothAddress ()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    if (midiDevice != null)
                    {
                        MidiDeviceInfo info = midiDevice.getInfo ();
                        if (info.getType () == MidiDeviceInfo.TYPE_BLUETOOTH)
                        {
                            BluetoothDevice btDevice = (BluetoothDevice) info.getProperties ().get (info.PROPERTY_BLUETOOTH_DEVICE);
                            if (btDevice != null)
                                return btDevice.getAddress ();
                        }
                    }
                }

                return "";
            }

            public BluetoothGatt getGatt () { return btGatt; }

            public i32 getID ()
            {
                return midiDevice.getInfo ().getId ();
            }

            @Override
            public z0 run ()
            {
                synchronized (MidiDeviceOpenTask.class)
                {
                    if (owner != null && midiDevice != null)
                        owner.onDeviceOpenedDelayed (midiDevice);
                }
            }

            private MidiDeviceManager owner;
            private MidiDevice midiDevice;
            private BluetoothGatt btGatt;
        }

        //==============================================================================
        public MidiDeviceManager (Context contextToUse)
        {
            appContext = contextToUse;
            manager = (MidiManager) appContext.getSystemService (MIDI_SERVICE);

            if (manager == null)
            {
                Log.d ("DRX", "MidiDeviceManager error: could not get MidiManager system service");
                return;
            }

            MidiDeviceInfo[] foundDevices = manager.getDevices ();

            for (MidiDeviceInfo info : foundDevices)
                onDeviceAdded (info);

            manager.registerDeviceCallback (this, null);
        }

        protected z0 finalize () throws Throwable
        {
            manager.unregisterDeviceCallback (this);

            synchronized (MidiDeviceManager.class)
            {
                btDevicesPairing.clear ();

                for (Integer deviceID : openTasks.keySet ())
                    openTasks.get (deviceID).cancel ();

                openTasks = null;
            }

            for (MidiPortPath key : openPorts.keySet ())
                openPorts.get (key).get ().close ();

            openPorts = null;

            for (Pair<MidiDevice, BluetoothGatt> device : midiDevices)
            {
                if (device.second != null)
                {
                    device.second.disconnect ();
                    device.second.close ();
                }

                device.first.close ();
            }

            midiDevices.clear ();

            super.finalize ();
        }

        public String[] getDrxAndroidMidiOutputDeviceNameAndIDs ()
        {
            return getDrxAndroidMidiDeviceNameAndIDs (MidiDeviceInfo.PortInfo.TYPE_OUTPUT);
        }

        public String[] getDrxAndroidMidiInputDeviceNameAndIDs ()
        {
            return getDrxAndroidMidiDeviceNameAndIDs (MidiDeviceInfo.PortInfo.TYPE_INPUT);
        }

        private String[] getDrxAndroidMidiDeviceNameAndIDs (i32 portType)
        {
            // only update the list when DRX asks for a new list
            synchronized (MidiDeviceManager.class)
            {
                deviceInfos = getDeviceInfos ();
            }

            ArrayList<String> portNameAndIDs = new ArrayList<String> ();

            for (MidiPortPath portInfo  : getAllPorts (portType))
            {
                portNameAndIDs.add (getPortName (portInfo));
                portNameAndIDs.add (Integer.toString (portInfo.hashCode ()));
            }

            String[] names = new String[portNameAndIDs.size ()];
            return portNameAndIDs.toArray (names);
        }

        private DrxMidiPort openMidiPortWithID (i32 deviceID, i64 host, boolean isInput)
        {
            synchronized (MidiDeviceManager.class)
            {
                i32 portTypeToFind = (isInput ? MidiDeviceInfo.PortInfo.TYPE_INPUT : MidiDeviceInfo.PortInfo.TYPE_OUTPUT);
                MidiPortPath portInfo = getPortPathForID (portTypeToFind, deviceID);

                if (portInfo != null)
                {
                    // ports must be opened exclusively!
                    if (openPorts.containsKey (portInfo))
                        return null;

                    Pair<MidiDevice, BluetoothGatt> devicePair = getMidiDevicePairForId (portInfo.deviceId);

                    if (devicePair != null)
                    {
                        MidiDevice device = devicePair.first;
                        if (device != null)
                        {
                            DrxMidiPort juceMidiPort = null;

                            if (isInput)
                            {
                                MidiOutputPort outputPort = device.openOutputPort (portInfo.portIndex);

                                if (outputPort != null)
                                    juceMidiPort = new DrxMidiInputPort (this, outputPort, portInfo, host);
                            } else
                            {
                                MidiInputPort inputPort = device.openInputPort (portInfo.portIndex);

                                if (inputPort != null)
                                    juceMidiPort = new DrxMidiOutputPort (this, inputPort, portInfo);
                            }

                            if (juceMidiPort != null)
                            {
                                openPorts.put (portInfo, new WeakReference<DrxMidiPort> (juceMidiPort));

                                return juceMidiPort;
                            }
                        }
                    }
                }
            }

            return null;
        }

        public DrxMidiPort openMidiInputPortWithID (i32 deviceID, i64 host)
        {
            return openMidiPortWithID (deviceID, host, true);
        }

        public DrxMidiPort openMidiOutputPortWithID (i32 deviceID)
        {
            return openMidiPortWithID (deviceID, 0, false);
        }

        /* 0: unpaired, 1: paired, 2: pairing */
        public i32 getBluetoothDeviceStatus (String address)
        {
            synchronized (MidiDeviceManager.class)
            {
                if (!address.isEmpty ())
                {
                    if (findMidiDeviceForBluetoothAddress (address) != null)
                        return 1;

                    if (btDevicesPairing.containsKey (address))
                        return 2;

                    if (findOpenTaskForBluetoothAddress (address) != null)
                        return 2;
                }
            }

            return 0;
        }

        public boolean pairBluetoothDevice (BluetoothDevice btDevice)
        {
            String btAddress = btDevice.getAddress ();
            if (btAddress.isEmpty ())
                return false;

            synchronized (MidiDeviceManager.class)
            {
                if (getBluetoothDeviceStatus (btAddress) != 0)
                    return false;


                btDevicesPairing.put (btDevice.getAddress (), null);
                BluetoothGatt gatt = btDevice.connectGatt (appContext.getApplicationContext (), true, new DummyBluetoothGattCallback (this));

                if (gatt != null)
                {
                    btDevicesPairing.put (btDevice.getAddress (), gatt);
                } else
                {
                    pairBluetoothDeviceStepTwo (btDevice);
                }
            }

            return true;
        }

        public z0 pairBluetoothDeviceStepTwo (BluetoothDevice btDevice)
        {
            manager.openBluetoothDevice (btDevice, this, null);
        }

        public z0 unpairBluetoothDevice (String address)
        {
            if (address.isEmpty ())
                return;

            synchronized (MidiDeviceManager.class)
            {
                if (btDevicesPairing.containsKey (address))
                {
                    BluetoothGatt gatt = btDevicesPairing.get (address);
                    if (gatt != null)
                    {
                        gatt.disconnect ();
                        gatt.close ();
                    }

                    btDevicesPairing.remove (address);
                }

                MidiDeviceOpenTask openTask = findOpenTaskForBluetoothAddress (address);
                if (openTask != null)
                {
                    i32 deviceID = openTask.getID ();
                    openTask.cancel ();
                    openTasks.remove (deviceID);
                }

                Pair<MidiDevice, BluetoothGatt> midiDevicePair = findMidiDeviceForBluetoothAddress (address);
                if (midiDevicePair != null)
                {
                    MidiDevice midiDevice = midiDevicePair.first;
                    onDeviceRemoved (midiDevice.getInfo ());

                    try
                    {
                        midiDevice.close ();
                    } catch (IOException exception)
                    {
                        Log.d ("DRX", "IOException while closing midi device");
                    }
                }
            }
        }

        private Pair<MidiDevice, BluetoothGatt> findMidiDeviceForBluetoothAddress (String address)
        {
            for (Pair<MidiDevice, BluetoothGatt> midiDevice : midiDevices)
            {
                MidiDeviceInfo info = midiDevice.first.getInfo ();
                if (info.getType () == MidiDeviceInfo.TYPE_BLUETOOTH)
                {
                    BluetoothDevice btDevice = (BluetoothDevice) info.getProperties ().get (info.PROPERTY_BLUETOOTH_DEVICE);
                    if (btDevice != null && btDevice.getAddress ().equals (address))
                        return midiDevice;
                }
            }

            return null;
        }

        private MidiDeviceOpenTask findOpenTaskForBluetoothAddress (String address)
        {
            for (Integer deviceID : openTasks.keySet ())
            {
                MidiDeviceOpenTask openTask = openTasks.get (deviceID);
                if (openTask.getBluetoothAddress ().equals (address))
                    return openTask;
            }

            return null;
        }

        public z0 removePort (MidiPortPath path)
        {
            openPorts.remove (path);
        }

        @Override
        public z0 onDeviceAdded (MidiDeviceInfo info)
        {
            // only add standard midi devices
            if (info.getType () == info.TYPE_BLUETOOTH)
                return;

            manager.openDevice (info, this, null);
        }

        @Override
        public z0 onDeviceRemoved (MidiDeviceInfo info)
        {
            synchronized (MidiDeviceManager.class)
            {
                Pair<MidiDevice, BluetoothGatt> devicePair = getMidiDevicePairForId (info.getId ());

                if (devicePair != null)
                {
                    MidiDevice midiDevice = devicePair.first;
                    BluetoothGatt gatt = devicePair.second;

                    // close all ports that use this device
                    boolean removedPort = true;

                    while (removedPort == true)
                    {
                        removedPort = false;
                        for (MidiPortPath key : openPorts.keySet ())
                        {
                            if (key.deviceId == info.getId ())
                            {
                                openPorts.get (key).get ().close ();
                                removedPort = true;
                                break;
                            }
                        }
                    }

                    if (gatt != null)
                    {
                        gatt.disconnect ();
                        gatt.close ();
                    }

                    midiDevices.remove (devicePair);
                }
            }

            handleDevicesChanged();
        }

        @Override
        public z0 onDeviceStatusChanged (MidiDeviceStatus status)
        {
        }

        @Override
        public z0 onDeviceOpened (MidiDevice theDevice)
        {
            synchronized (MidiDeviceManager.class)
            {
                MidiDeviceInfo info = theDevice.getInfo ();
                i32 deviceID = info.getId ();
                BluetoothGatt gatt = null;
                boolean isBluetooth = false;

                if (!openTasks.containsKey (deviceID))
                {
                    if (info.getType () == MidiDeviceInfo.TYPE_BLUETOOTH)
                    {
                        isBluetooth = true;
                        BluetoothDevice btDevice = (BluetoothDevice) info.getProperties ().get (info.PROPERTY_BLUETOOTH_DEVICE);
                        if (btDevice != null)
                        {
                            String btAddress = btDevice.getAddress ();
                            if (btDevicesPairing.containsKey (btAddress))
                            {
                                gatt = btDevicesPairing.get (btAddress);
                                btDevicesPairing.remove (btAddress);
                            } else
                            {
                                // unpair was called in the mean time
                                try
                                {
                                    Pair<MidiDevice, BluetoothGatt> midiDevicePair = findMidiDeviceForBluetoothAddress (btDevice.getAddress ());
                                    if (midiDevicePair != null)
                                    {
                                        gatt = midiDevicePair.second;

                                        if (gatt != null)
                                        {
                                            gatt.disconnect ();
                                            gatt.close ();
                                        }
                                    }

                                    theDevice.close ();
                                } catch (IOException e)
                                {
                                }

                                return;
                            }
                        }
                    }

                    MidiDeviceOpenTask openTask = new MidiDeviceOpenTask (this, theDevice, gatt);
                    openTasks.put (deviceID, openTask);

                    new java.util.Timer ().schedule (openTask, (isBluetooth ? 2000 : 100));
                }
            }
        }

        public z0 onDeviceOpenedDelayed (MidiDevice theDevice)
        {
            synchronized (MidiDeviceManager.class)
            {
                i32 deviceID = theDevice.getInfo ().getId ();

                if (openTasks.containsKey (deviceID))
                {
                    if (!midiDevices.contains (theDevice))
                    {
                        BluetoothGatt gatt = openTasks.get (deviceID).getGatt ();
                        openTasks.remove (deviceID);
                        midiDevices.add (new Pair<MidiDevice, BluetoothGatt> (theDevice, gatt));
                        handleDevicesChanged();
                    }
                } else
                {
                    // unpair was called in the mean time
                    MidiDeviceInfo info = theDevice.getInfo ();
                    BluetoothDevice btDevice = (BluetoothDevice) info.getProperties ().get (info.PROPERTY_BLUETOOTH_DEVICE);
                    if (btDevice != null)
                    {
                        String btAddress = btDevice.getAddress ();
                        Pair<MidiDevice, BluetoothGatt> midiDevicePair = findMidiDeviceForBluetoothAddress (btDevice.getAddress ());
                        if (midiDevicePair != null)
                        {
                            BluetoothGatt gatt = midiDevicePair.second;

                            if (gatt != null)
                            {
                                gatt.disconnect ();
                                gatt.close ();
                            }
                        }
                    }

                    try
                    {
                        theDevice.close ();
                    } catch (IOException e)
                    {
                    }
                }
            }
        }

        public String getPortName (MidiPortPath path)
        {
            i32 portTypeToFind = (path.isInput ? MidiDeviceInfo.PortInfo.TYPE_INPUT : MidiDeviceInfo.PortInfo.TYPE_OUTPUT);

            synchronized (MidiDeviceManager.class)
            {
                for (MidiDeviceInfo info : deviceInfos)
                {
                    if (info.getId () == path.deviceId)
                    {
                        for (MidiDeviceInfo.PortInfo portInfo : info.getPorts ())
                        {
                            i32 portType = portInfo.getType ();
                            if (portType == portTypeToFind)
                            {
                                i32 portIndex = portInfo.getPortNumber ();
                                if (portIndex == path.portIndex)
                                {
                                    String portName = portInfo.getName ();
                                    if (portName.isEmpty ())
                                        portName = (String) info.getProperties ().get (info.PROPERTY_NAME);

                                    return portName;
                                }
                            }
                        }
                    }
                }
            }

            return "";
        }

        public ArrayList<MidiPortPath> getAllPorts (i32 portType)
        {
            ArrayList<MidiPortPath> ports = new ArrayList<MidiPortPath> ();

            for (MidiDeviceInfo info : deviceInfos)
                for (MidiDeviceInfo.PortInfo portInfo : info.getPorts ())
                    if (portInfo.getType () == portType)
                        ports.add (new MidiPortPath (info.getId (), (portType == MidiDeviceInfo.PortInfo.TYPE_INPUT),
                                                     portInfo.getPortNumber ()));

            return ports;
        }

        public MidiPortPath getPortPathForID (i32 portType, i32 deviceID)
        {
            for (MidiPortPath port : getAllPorts (portType))
                if (port.hashCode () == deviceID)
                    return port;

            return null;
        }

        private MidiDeviceInfo[] getDeviceInfos ()
        {
            synchronized (MidiDeviceManager.class)
            {
                MidiDeviceInfo[] infos = new MidiDeviceInfo[midiDevices.size ()];

                i32 idx = 0;
                for (Pair<MidiDevice, BluetoothGatt> midiDevice : midiDevices)
                    infos[idx++] = midiDevice.first.getInfo ();

                return infos;
            }
        }

        private Pair<MidiDevice, BluetoothGatt> getMidiDevicePairForId (i32 deviceId)
        {
            synchronized (MidiDeviceManager.class)
            {
                for (Pair<MidiDevice, BluetoothGatt> midiDevice : midiDevices)
                    if (midiDevice.first.getInfo ().getId () == deviceId)
                        return midiDevice;
            }

            return null;
        }

        private MidiManager manager;
        private HashMap<String, BluetoothGatt> btDevicesPairing = new HashMap<String, BluetoothGatt>();
        private HashMap<Integer, MidiDeviceOpenTask> openTasks = new HashMap<Integer, MidiDeviceOpenTask>();
        private ArrayList<Pair<MidiDevice, BluetoothGatt>> midiDevices = new ArrayList<Pair<MidiDevice, BluetoothGatt>>();
        private MidiDeviceInfo[] deviceInfos;
        private HashMap<MidiPortPath, WeakReference<DrxMidiPort>> openPorts = new HashMap<MidiPortPath, WeakReference<DrxMidiPort>>();
        private Context appContext = null;
    }

    public static MidiDeviceManager getAndroidMidiDeviceManager (Context context)
    {
        if (context.getSystemService (MIDI_SERVICE) == null)
            return null;

        synchronized (DrxMidiSupport.class)
        {
            if (midiDeviceManager == null)
                midiDeviceManager = new MidiDeviceManager (context);
        }

        return midiDeviceManager;
    }

    public static BluetoothMidiManager getAndroidBluetoothManager (Context context)
    {
        BluetoothAdapter adapter = getDefaultBluetoothAdapter (context);

        if (adapter == null)
            return null;

        if (adapter.getBluetoothLeScanner () == null)
            return null;

        synchronized (DrxMidiSupport.class)
        {
            if (bluetoothManager == null)
                bluetoothManager = new BluetoothMidiManager (context);
        }

        return bluetoothManager;
    }

    // To be called when devices become (un)available
    private native static z0 handleDevicesChanged();

    private static MidiDeviceManager midiDeviceManager = null;
    private static BluetoothMidiManager bluetoothManager = null;
}
