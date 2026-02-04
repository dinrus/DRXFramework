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

extern HWND drx_messageWindowHandle;

#if DRX_MODULE_AVAILABLE_drx_gui_extra
 LRESULT drx_offerEventToActiveXControl (::MSG&);
#endif

using CheckEventBlockedByModalComps = b8 (*)(const MSG&);
CheckEventBlockedByModalComps isEventBlockedByModalComps = nullptr;

using SettingChangeCallbackFunc = z0 (*)(z0);
SettingChangeCallbackFunc settingChangeCallback = nullptr;

//==============================================================================
class InternalMessageQueue
{
public:
    InternalMessageQueue()
    {
        messageWindow = std::make_unique<HiddenMessageWindow> (messageWindowName, (WNDPROC) messageWndProc);
        drx_messageWindowHandle = messageWindow->getHWND();
    }

    ~InternalMessageQueue()
    {
        drx_messageWindowHandle = nullptr;
        clearSingletonInstance();
    }

    DRX_DECLARE_SINGLETON_INLINE (InternalMessageQueue, false)

    //==============================================================================
    z0 broadcastMessage (const Txt& message)
    {
        auto localCopy = message;

        Array<HWND> windows;
        EnumWindows (&broadcastEnumWindowProc, (LPARAM) &windows);

        for (i32 i = windows.size(); --i >= 0;)
        {
            COPYDATASTRUCT data;
            data.dwData = broadcastMessageMagicNumber;
            data.cbData = (DWORD) (((size_t) localCopy.length() + 1) * sizeof (CharPointer_UTF32::CharType));
            data.lpData = (uk) localCopy.toUTF32().getAddress();

            DWORD_PTR result;
            SendMessageTimeout (windows.getUnchecked (i), WM_COPYDATA,
                                (WPARAM) drx_messageWindowHandle,
                                (LPARAM) &data,
                                SMTO_BLOCK | SMTO_ABORTIFHUNG, 8000, &result);
        }
    }

    z0 postMessage (MessageManager::MessageBase* message)
    {
        b8 shouldTriggerMessageQueueDispatch = false;

        {
            const ScopedLock sl (lock);

            shouldTriggerMessageQueueDispatch = messageQueue.isEmpty();
            messageQueue.add (message);
        }

        if (! shouldTriggerMessageQueueDispatch)
            return;

        if (detail::RunningInUnity::state)
        {
            SendNotifyMessage (drx_messageWindowHandle, customMessageID, 0, 0);
            return;
        }

        PostMessage (drx_messageWindowHandle, customMessageID, 0, 0);
    }

    b8 dispatchNextMessage (b8 returnIfNoPendingMessages)
    {
        MSG m;

        if (returnIfNoPendingMessages && ! PeekMessage (&m, nullptr, 0, 0, PM_NOREMOVE))
            return false;

        if (GetMessage (&m, nullptr, 0, 0) >= 0)
        {
           #if DRX_MODULE_AVAILABLE_drx_gui_extra
            if (drx_offerEventToActiveXControl (m) != S_FALSE)
                return true;
           #endif

            if (m.message == customMessageID && m.hwnd == drx_messageWindowHandle)
            {
                dispatchMessages();
            }
            else if (m.message == WM_QUIT)
            {
                if (auto* app = DRXApplicationBase::getInstance())
                    app->systemRequestedQuit();
            }
            else if (isEventBlockedByModalComps == nullptr || ! isEventBlockedByModalComps (m))
            {
                if ((m.message == WM_LBUTTONDOWN || m.message == WM_RBUTTONDOWN)
                      && ! DrxWindowIdentifier::isDRXWindow (m.hwnd))
                {
                    // if it's someone else's window being clicked on, and the focus is
                    // currently on a drx window, pass the kb focus over..
                    auto currentFocus = GetFocus();

                    if (currentFocus == nullptr || DrxWindowIdentifier::isDRXWindow (currentFocus))
                        SetFocus (m.hwnd);
                }

                TranslateMessage (&m);
                DispatchMessage (&m);
            }
        }

        return true;
    }

private:
    //==============================================================================
    static LRESULT CALLBACK messageWndProc (HWND h, UINT message, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (h == drx_messageWindowHandle)
        {
            if (message == customMessageID)
            {
                if (auto* queue = InternalMessageQueue::getInstanceWithoutCreating())
                    queue->dispatchMessages();

                return 0;
            }

            if (message == WM_COPYDATA)
            {
                handleBroadcastMessage (reinterpret_cast<const COPYDATASTRUCT*> (lParam));
                return 0;
            }

            if (message == WM_SETTINGCHANGE)
                NullCheckedInvocation::invoke (settingChangeCallback);
        }

        return DefWindowProc (h, message, wParam, lParam);
    }

    static BOOL CALLBACK broadcastEnumWindowProc (HWND hwnd, LPARAM lParam)
    {
        if (hwnd != drx_messageWindowHandle)
        {
            TCHAR windowName[64] = { 0 }; // no need to read longer strings than this
            GetWindowText (hwnd, windowName, 63);

            if (Txt (windowName) == messageWindowName)
                reinterpret_cast<Array<HWND>*> (lParam)->add (hwnd);
        }

        return TRUE;
    }

    static z0 dispatchMessage (MessageManager::MessageBase* message)
    {
        DRX_TRY
        {
            message->messageCallback();
        }
        DRX_CATCH_EXCEPTION

        message->decReferenceCount();
    }

    static z0 handleBroadcastMessage (const COPYDATASTRUCT* data)
    {
        if (data != nullptr && data->dwData == broadcastMessageMagicNumber)
        {
            struct BroadcastMessage final : public CallbackMessage
            {
                BroadcastMessage (CharPointer_UTF32 text, size_t length) : message (text, length) {}
                z0 messageCallback() override { MessageManager::getInstance()->deliverBroadcastMessage (message); }

                Txt message;
            };

            (new BroadcastMessage (CharPointer_UTF32 ((const CharPointer_UTF32::CharType*) data->lpData),
                                   data->cbData / sizeof (CharPointer_UTF32::CharType)))
                ->post();
        }
    }

    z0 dispatchMessages()
    {
        ReferenceCountedArray<MessageManager::MessageBase> messagesToDispatch;

        {
            const ScopedLock sl (lock);

            if (messageQueue.isEmpty())
                return;

            messagesToDispatch.swapWith (messageQueue);
        }

        for (i32 i = 0; i < messagesToDispatch.size(); ++i)
        {
            auto message = messagesToDispatch.getUnchecked (i);
            message->incReferenceCount();
            dispatchMessage (message.get());
        }
    }

    //==============================================================================
    static constexpr u32 customMessageID = WM_USER + 123;
    static constexpr u32 broadcastMessageMagicNumber = 0xc403;
    static const TCHAR messageWindowName[];

    std::unique_ptr<HiddenMessageWindow> messageWindow;

    CriticalSection lock;
    ReferenceCountedArray<MessageManager::MessageBase> messageQueue;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InternalMessageQueue)
};

const TCHAR InternalMessageQueue::messageWindowName[] = _T("DRXWindow");

//==============================================================================
namespace detail
{

b8 dispatchNextMessageOnSystemQueue (b8 returnIfNoPendingMessages)
{
    if (auto* queue = InternalMessageQueue::getInstanceWithoutCreating())
        return queue->dispatchNextMessage (returnIfNoPendingMessages);

    return false;
}

} // namespace detail

b8 MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    if (auto* queue = InternalMessageQueue::getInstanceWithoutCreating())
    {
        queue->postMessage (message);
        return true;
    }

    return false;
}

z0 MessageManager::broadcastMessage (const Txt& value)
{
    if (auto* queue = InternalMessageQueue::getInstanceWithoutCreating())
        queue->broadcastMessage (value);
}

//==============================================================================
z0 MessageManager::doPlatformSpecificInitialisation()
{
    [[maybe_unused]] const auto result = OleInitialize (nullptr);
    InternalMessageQueue::getInstance();
}

z0 MessageManager::doPlatformSpecificShutdown()
{
    InternalMessageQueue::deleteInstance();
    OleUninitialize();
}

//==============================================================================
struct MountedVolumeListChangeDetector::Pimpl
{
    explicit Pimpl (MountedVolumeListChangeDetector& d)
        : owner (d)
    {
        File::findFileSystemRoots (lastVolumeList);
    }

    z0 systemDeviceChanged()
    {
        Array<File> newList;
        File::findFileSystemRoots (newList);

        if (std::exchange (lastVolumeList, newList) != newList)
            owner.mountedVolumeListChanged();
    }

    DeviceChangeDetector detector { L"MountedVolumeList", [this] { systemDeviceChanged(); } };
    MountedVolumeListChangeDetector& owner;
    Array<File> lastVolumeList;
};

MountedVolumeListChangeDetector::MountedVolumeListChangeDetector()  { pimpl.reset (new Pimpl (*this)); }
MountedVolumeListChangeDetector::~MountedVolumeListChangeDetector() {}

} // namespace drx
