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

::Window drx_createKeyProxyWindow (ComponentPeer*);
z0 drx_deleteKeyProxyWindow (::Window);

//==============================================================================
enum
{
    maxXEmbedVersionToSupport = 0
};

enum
{
    XEMBED_MAPPED  = (1<<0)
};

enum
{
    XEMBED_EMBEDDED_NOTIFY        = 0,
    XEMBED_WINDOW_ACTIVATE        = 1,
    XEMBED_WINDOW_DEACTIVATE      = 2,
    XEMBED_REQUEST_FOCUS          = 3,
    XEMBED_FOCUS_IN               = 4,
    XEMBED_FOCUS_OUT              = 5,
    XEMBED_FOCUS_NEXT             = 6,
    XEMBED_FOCUS_PREV             = 7,
    XEMBED_MODALITY_ON            = 10,
    XEMBED_MODALITY_OFF           = 11,
    XEMBED_REGISTER_ACCELERATOR   = 12,
    XEMBED_UNREGISTER_ACCELERATOR = 13,
    XEMBED_ACTIVATE_ACCELERATOR   = 14
};

enum
{
    XEMBED_FOCUS_CURRENT = 0,
    XEMBED_FOCUS_FIRST   = 1,
    XEMBED_FOCUS_LAST    = 2
};

//==============================================================================
class XEmbedComponent::Pimpl  : private ComponentListener
{
public:
    //==============================================================================
    struct SharedKeyWindow final : public ReferenceCountedObject
    {
        SharedKeyWindow (ComponentPeer* peerToUse)
            : keyPeer (peerToUse),
              keyProxy (drx_createKeyProxyWindow (keyPeer)),
              association (peerToUse, keyProxy)
        {}

        ~SharedKeyWindow()
        {
            association = {};
            drx_deleteKeyProxyWindow (keyProxy);

            auto& keyWindows = getKeyWindows();
            keyWindows.remove (keyPeer);
        }

        using Ptr = ReferenceCountedObjectPtr<SharedKeyWindow>;

        //==============================================================================
        Window getHandle()    { return keyProxy; }

        static Window getCurrentFocusWindow (ComponentPeer* peerToLookFor)
        {
            auto& keyWindows = getKeyWindows();

            if (peerToLookFor != nullptr)
                if (auto* foundKeyWindow = keyWindows[peerToLookFor])
                    return foundKeyWindow->keyProxy;

            return {};
        }

        static SharedKeyWindow::Ptr getKeyWindowForPeer (ComponentPeer* peerToLookFor)
        {
            jassert (peerToLookFor != nullptr);

            auto& keyWindows = getKeyWindows();
            auto foundKeyWindow = keyWindows[peerToLookFor];

            if (foundKeyWindow == nullptr)
            {
                foundKeyWindow = new SharedKeyWindow (peerToLookFor);
                keyWindows.set (peerToLookFor, foundKeyWindow);
            }

            return foundKeyWindow;
        }

    private:
        //==============================================================================
        ComponentPeer* keyPeer;
        Window keyProxy;
        ScopedWindowAssociation association;

        static HashMap<ComponentPeer*, SharedKeyWindow*>& getKeyWindows()
        {
            // store a weak reference to the shared key windows
            static HashMap<ComponentPeer*, SharedKeyWindow*> keyWindows;
            return keyWindows;
        }
    };

public:
    //==============================================================================
    Pimpl (XEmbedComponent& parent, Window x11Window,
           b8 wantsKeyboardFocus, b8 isClientInitiated, b8 shouldAllowResize)
        : owner (parent),
          infoAtom (XWindowSystem::getInstance()->getAtoms().XembedInfo),
          messageTypeAtom (XWindowSystem::getInstance()->getAtoms().XembedMsgType),
          clientInitiated (isClientInitiated),
          wantsFocus (wantsKeyboardFocus),
          allowResize (shouldAllowResize)
    {
        getWidgets().add (this);

        createHostWindow();

        if (clientInitiated)
            setClient (x11Window, true);

        owner.setWantsKeyboardFocus (wantsFocus);
        owner.addComponentListener (this);
    }

    ~Pimpl() override
    {
        owner.removeComponentListener (this);
        setClient (0, true);

        if (host != 0)
        {
            auto dpy = getDisplay();

            X11Symbols::getInstance()->xDestroyWindow (dpy, host);
            X11Symbols::getInstance()->xSync (dpy, false);

            auto mask = NoEventMask | KeyPressMask | KeyReleaseMask
                      | EnterWindowMask | LeaveWindowMask | PointerMotionMask
                      | KeymapStateMask | ExposureMask | StructureNotifyMask
                      | FocusChangeMask;

            XEvent event;
            while (X11Symbols::getInstance()->xCheckWindowEvent (dpy, host, mask, &event) == True)
            {}

            host = 0;
        }

        getWidgets().removeAllInstancesOf (this);
    }

    //==============================================================================
    z0 setClient (Window xembedClient, b8 shouldReparent)
    {
        removeClient();

        if (xembedClient != 0)
        {
            auto dpy = getDisplay();

            client = xembedClient;

            // if the client has initiated the component then keep the clients size
            // otherwise the client should use the host's window' size
            if (clientInitiated)
            {
                configureNotify();
            }
            else
            {
                auto newBounds = getX11BoundsFromDrx();
                X11Symbols::getInstance()->xResizeWindow (dpy, client, static_cast<u32> (newBounds.getWidth()),
                                                          static_cast<u32> (newBounds.getHeight()));
            }

            auto eventMask = StructureNotifyMask | PropertyChangeMask | FocusChangeMask;

            XWindowAttributes clientAttr;
            X11Symbols::getInstance()->xGetWindowAttributes (dpy, client, &clientAttr);

            if ((eventMask & clientAttr.your_event_mask) != eventMask)
                X11Symbols::getInstance()->xSelectInput (dpy, client, clientAttr.your_event_mask | eventMask);

            getXEmbedMappedFlag();

            if (shouldReparent)
                X11Symbols::getInstance()->xReparentWindow (dpy, client, host, 0, 0);

            if (supportsXembed)
                sendXEmbedEvent (CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, (i64) host, xembedVersion);

            updateMapping();
        }
    }

    z0 focusGained (FocusChangeType changeType, FocusChangeDirection direction)
    {
        if (client != 0 && supportsXembed && wantsFocus)
        {
            updateKeyFocus();

            const auto xembedDirection = [&]
            {
                if (direction == FocusChangeDirection::forward)
                    return XEMBED_FOCUS_FIRST;

                if (direction == FocusChangeDirection::backward)
                    return XEMBED_FOCUS_LAST;

                return XEMBED_FOCUS_CURRENT;
            }();

            sendXEmbedEvent (CurrentTime, XEMBED_FOCUS_IN,
                             (changeType == focusChangedByTabKey ? xembedDirection : XEMBED_FOCUS_CURRENT));
        }
    }

    z0 focusLost (FocusChangeType)
    {
        if (client != 0 && supportsXembed && wantsFocus)
        {
            sendXEmbedEvent (CurrentTime, XEMBED_FOCUS_OUT);
            updateKeyFocus();
        }
    }

    z0 broughtToFront()
    {
        if (client != 0 && supportsXembed)
            sendXEmbedEvent (CurrentTime, XEMBED_WINDOW_ACTIVATE);
    }

    u64 getHostWindowID()
    {
        // You are using the client initiated version of the protocol. You cannot
        // retrieve the window id of the host. Please read the documentation for
        // the XEmebedComponent class.
        jassert (! clientInitiated);

        return host;
    }

    z0 updateEmbeddedBounds()
    {
        componentMovedOrResized (owner, true, true);
    }

private:
    //==============================================================================
    XEmbedComponent& owner;
    Window client = 0, host = 0;
    Atom infoAtom, messageTypeAtom;

    b8 clientInitiated;
    b8 wantsFocus        = false;
    b8 allowResize       = false;
    b8 supportsXembed    = false;
    b8 hasBeenMapped     = false;
    i32 xembedVersion      = maxXEmbedVersionToSupport;

    ComponentPeer* lastPeer = nullptr;
    SharedKeyWindow::Ptr keyWindow;

    //==============================================================================
    z0 componentParentHierarchyChanged (Component&) override   { peerChanged (owner.getPeer()); }
    z0 componentMovedOrResized (Component&, b8, b8) override
    {
        if (host != 0 && lastPeer != nullptr)
        {
            auto dpy = getDisplay();
            auto newBounds = getX11BoundsFromDrx();
            XWindowAttributes attr;

            if (X11Symbols::getInstance()->xGetWindowAttributes (dpy, host, &attr))
            {
                Rectangle<i32> currentBounds (attr.x, attr.y, attr.width, attr.height);
                if (currentBounds != newBounds)
                {
                    X11Symbols::getInstance()->xMoveResizeWindow (dpy, host, newBounds.getX(), newBounds.getY(),
                                                                  static_cast<u32> (newBounds.getWidth()),
                                                                  static_cast<u32> (newBounds.getHeight()));
                }
            }

            if (client != 0 && X11Symbols::getInstance()->xGetWindowAttributes (dpy, client, &attr))
            {
                Rectangle<i32> currentBounds (attr.x, attr.y, attr.width, attr.height);

                if ((currentBounds.getWidth() != newBounds.getWidth()
                     || currentBounds.getHeight() != newBounds.getHeight()))
                {
                    X11Symbols::getInstance()->xMoveResizeWindow (dpy, client, 0, 0,
                                                                  static_cast<u32> (newBounds.getWidth()),
                                                                  static_cast<u32> (newBounds.getHeight()));
                }
            }
        }
    }

    //==============================================================================
    z0 createHostWindow()
    {
        auto dpy = getDisplay();
        i32 defaultScreen = X11Symbols::getInstance()->xDefaultScreen (dpy);
        Window root = X11Symbols::getInstance()->xRootWindow (dpy, defaultScreen);

        XSetWindowAttributes swa;
        swa.border_pixel = 0;
        swa.background_pixmap = None;
        swa.override_redirect = True;
        swa.event_mask = SubstructureNotifyMask | StructureNotifyMask | FocusChangeMask;

        host = X11Symbols::getInstance()->xCreateWindow (dpy, root, 0, 0, 1, 1, 0, CopyFromParent,
                                                         InputOutput, CopyFromParent,
                                                         CWEventMask | CWBorderPixel | CWBackPixmap | CWOverrideRedirect,
                                                         &swa);
    }

    z0 removeClient()
    {
        if (client != 0)
        {
            auto dpy = getDisplay();
            X11Symbols::getInstance()->xSelectInput (dpy, client, 0);

            keyWindow = nullptr;

            i32 defaultScreen = X11Symbols::getInstance()->xDefaultScreen (dpy);
            Window root = X11Symbols::getInstance()->xRootWindow (dpy, defaultScreen);

            if (hasBeenMapped)
            {
                X11Symbols::getInstance()->xUnmapWindow (dpy, client);
                hasBeenMapped = false;
            }

            X11Symbols::getInstance()->xReparentWindow (dpy, client, root, 0, 0);
            client = 0;

            X11Symbols::getInstance()->xSync (dpy, False);
        }
    }

    z0 updateMapping()
    {
        if (client != 0)
        {
            const b8 shouldBeMapped = getXEmbedMappedFlag();

            if (shouldBeMapped != hasBeenMapped)
            {
                hasBeenMapped = shouldBeMapped;

                if (shouldBeMapped)
                    X11Symbols::getInstance()->xMapWindow (getDisplay(), client);
                else
                    X11Symbols::getInstance()->xUnmapWindow (getDisplay(), client);
            }
        }
    }

    Window getParentX11Window()
    {
        if (auto* peer = owner.getPeer())
            return reinterpret_cast<Window> (peer->getNativeHandle());

        return {};
    }

    Display* getDisplay()   { return XWindowSystem::getInstance()->getDisplay(); }

    //==============================================================================
    b8 getXEmbedMappedFlag()
    {
        XWindowSystemUtilities::GetXProperty embedInfo (getDisplay(), client, infoAtom, 0, 2, false, infoAtom);

        if (embedInfo.success && embedInfo.actualFormat == 32
             && embedInfo.numItems >= 2 && embedInfo.data != nullptr)
        {
            i64 version;
            memcpy (&version, embedInfo.data, sizeof (i64));

            supportsXembed = true;
            xembedVersion = jmin ((i32) maxXEmbedVersionToSupport, (i32) version);

            i64 flags;
            memcpy (&flags, embedInfo.data + sizeof (i64), sizeof (i64));

            return ((flags & XEMBED_MAPPED) != 0);
        }
        else
        {
            supportsXembed = false;
            xembedVersion = maxXEmbedVersionToSupport;
        }

        return true;
    }

    //==============================================================================
    z0 propertyChanged (const Atom& a)
    {
        if (a == infoAtom)
            updateMapping();
    }

    z0 configureNotify()
    {
        XWindowAttributes attr;
        auto dpy = getDisplay();

        if (X11Symbols::getInstance()->xGetWindowAttributes (dpy, client, &attr))
        {
            XWindowAttributes hostAttr;

            if (X11Symbols::getInstance()->xGetWindowAttributes (dpy, host, &hostAttr))
                if (attr.width != hostAttr.width || attr.height != hostAttr.height)
                    X11Symbols::getInstance()->xResizeWindow (dpy, host, (u32) attr.width, (u32) attr.height);

            // as the client window is not on any screen yet, we need to guess
            // on which screen it might appear to get a scaling factor :-(
            auto& displays = Desktop::getInstance().getDisplays();
            auto* peer = owner.getPeer();
            const f64 scale = (peer != nullptr ? peer->getPlatformScaleFactor()
                                                  : displays.getPrimaryDisplay()->scale);

            Point<i32> topLeftInPeer
                = (peer != nullptr ? peer->getComponent().getLocalPoint (&owner, Point<i32> (0, 0))
                   : owner.getBounds().getTopLeft());

            Rectangle<i32> newBounds (topLeftInPeer.getX(), topLeftInPeer.getY(),
                                      static_cast<i32> (static_cast<f64> (attr.width)  / scale),
                                      static_cast<i32> (static_cast<f64> (attr.height) / scale));


            if (peer != nullptr)
                newBounds = owner.getLocalArea (&peer->getComponent(), newBounds);

            jassert (newBounds.getX() == 0 && newBounds.getY() == 0);

            if (newBounds != owner.getLocalBounds())
                owner.setSize (newBounds.getWidth(), newBounds.getHeight());
        }
    }

    z0 peerChanged (ComponentPeer* newPeer)
    {
        if (newPeer != lastPeer)
        {
            if (lastPeer != nullptr)
                keyWindow = nullptr;

            auto dpy = getDisplay();
            Window rootWindow = X11Symbols::getInstance()->xRootWindow (dpy, DefaultScreen (dpy));
            Rectangle<i32> newBounds = getX11BoundsFromDrx();

            if (newPeer == nullptr)
                X11Symbols::getInstance()->xUnmapWindow (dpy, host);

            Window newParent = (newPeer != nullptr ? getParentX11Window() : rootWindow);
            X11Symbols::getInstance()->xReparentWindow (dpy, host, newParent, newBounds.getX(), newBounds.getY());

            lastPeer = newPeer;

            if (newPeer != nullptr)
            {
                if (wantsFocus)
                {
                    keyWindow = SharedKeyWindow::getKeyWindowForPeer (newPeer);
                    updateKeyFocus();
                }

                componentMovedOrResized (owner, true, true);
                X11Symbols::getInstance()->xMapWindow (dpy, host);

                broughtToFront();
            }
        }
    }

    z0 updateKeyFocus()
    {
        if (lastPeer != nullptr && lastPeer->isFocused())
            X11Symbols::getInstance()->xSetInputFocus (getDisplay(), getCurrentFocusWindow (lastPeer), RevertToParent, CurrentTime);
    }

    //==============================================================================
    z0 handleXembedCmd (const ::Time& /*xTime*/, i64 opcode, i64 /*detail*/, i64 /*data1*/, i64 /*data2*/)
    {
        if (auto* peer = owner.getPeer())
            peer->getCurrentModifiersRealtime();

        switch (opcode)
        {
            case XEMBED_REQUEST_FOCUS:
                if (wantsFocus)
                    owner.grabKeyboardFocus();
                break;

            case XEMBED_FOCUS_NEXT:
                if (wantsFocus)
                    owner.moveKeyboardFocusToSibling (true);
                break;

            case XEMBED_FOCUS_PREV:
                if (wantsFocus)
                    owner.moveKeyboardFocusToSibling (false);
                break;

            default:
                break;
        }
    }

    b8 handleX11Event (const XEvent& e)
    {
        if (e.xany.window == client && client != 0)
        {
            switch (e.type)
            {
                case PropertyNotify:
                    propertyChanged (e.xproperty.atom);
                    return true;

                case ConfigureNotify:
                    if (allowResize)
                        configureNotify();
                    else
                        MessageManager::callAsync ([this] {componentMovedOrResized (owner, true, true);});

                    return true;

                default:
                    break;
            }
        }
        else if (e.xany.window == host && host != 0)
        {
            switch (e.type)
            {
                case ReparentNotify:
                    if (e.xreparent.parent == host && e.xreparent.window != client)
                    {
                        setClient (e.xreparent.window, false);
                        return true;
                    }
                    break;

                case CreateNotify:
                    if (e.xcreatewindow.parent != e.xcreatewindow.window && e.xcreatewindow.parent == host && e.xcreatewindow.window != client)
                    {
                        setClient (e.xcreatewindow.window, false);
                        return true;
                    }
                    break;

                case GravityNotify:
                    componentMovedOrResized (owner, true, true);
                    return true;

                case ClientMessage:
                    if (e.xclient.message_type == messageTypeAtom && e.xclient.format == 32)
                    {
                        handleXembedCmd ((::Time) e.xclient.data.l[0], e.xclient.data.l[1],
                                         e.xclient.data.l[2], e.xclient.data.l[3],
                                         e.xclient.data.l[4]);

                        return true;
                    }
                    break;

                default:
                    break;
            }
        }

        return false;
    }

    z0 sendXEmbedEvent (const ::Time& xTime, i64 opcode,
                          i64 opcodeMinor = 0, i64 data1 = 0, i64 data2 = 0)
    {
        XClientMessageEvent msg;
        auto dpy = getDisplay();

        ::memset (&msg, 0, sizeof (XClientMessageEvent));
        msg.window = client;
        msg.type = ClientMessage;
        msg.message_type = messageTypeAtom;
        msg.format = 32;
        msg.data.l[0] = (i64) xTime;
        msg.data.l[1] = opcode;
        msg.data.l[2] = opcodeMinor;
        msg.data.l[3] = data1;
        msg.data.l[4] = data2;

        X11Symbols::getInstance()->xSendEvent (dpy, client, False, NoEventMask, (XEvent*) &msg);
        X11Symbols::getInstance()->xSync (dpy, False);
    }

    Rectangle<i32> getX11BoundsFromDrx()
    {
        if (auto* peer = owner.getPeer())
        {
            auto r = peer->getComponent().getLocalArea (&owner, owner.getLocalBounds());
            return r * peer->getPlatformScaleFactor() * peer->getComponent().getDesktopScaleFactor();
        }

        return owner.getLocalBounds();
    }

    //==============================================================================
    friend b8 drx::drx_handleXEmbedEvent (ComponentPeer*, uk);
    friend u64 drx::drx_getCurrentFocusWindow (ComponentPeer*);

    static Array<Pimpl*>& getWidgets()
    {
        static Array<Pimpl*> i;
        return i;
    }

    static b8 dispatchX11Event (ComponentPeer* p, const XEvent* eventArg)
    {
        if (eventArg != nullptr)
        {
            auto& e = *eventArg;

            if (auto w = e.xany.window)
                for (auto* widget : getWidgets())
                    if (w == widget->host || w == widget->client)
                        return widget->handleX11Event (e);
        }
        else
        {
            for (auto* widget : getWidgets())
                if (widget->owner.getPeer() == p)
                    widget->peerChanged (nullptr);
        }

        return false;
    }

    static Window getCurrentFocusWindow (ComponentPeer* p)
    {
        if (p != nullptr)
        {
            for (auto* widget : getWidgets())
                if (widget->owner.getPeer() == p && widget->owner.hasKeyboardFocus (false))
                    return widget->client;
        }

        return SharedKeyWindow::getCurrentFocusWindow (p);
    }
};

//==============================================================================
XEmbedComponent::XEmbedComponent (b8 wantsKeyboardFocus, b8 allowForeignWidgetToResizeComponent)
    : pimpl (new Pimpl (*this, 0, wantsKeyboardFocus, false, allowForeignWidgetToResizeComponent))
{
    setOpaque (true);
}

XEmbedComponent::XEmbedComponent (u64 wID, b8 wantsKeyboardFocus, b8 allowForeignWidgetToResizeComponent)
    : pimpl (new Pimpl (*this, wID, wantsKeyboardFocus, true, allowForeignWidgetToResizeComponent))
{
    setOpaque (true);
}

XEmbedComponent::~XEmbedComponent() {}

z0 XEmbedComponent::paint (Graphics& g)
{
    g.fillAll (Colors::lightgrey);
}

z0 XEmbedComponent::focusGainedWithDirection (FocusChangeType changeType, FocusChangeDirection direction)
{
    pimpl->focusGained (changeType, direction);
}

z0 XEmbedComponent::focusLost   (FocusChangeType changeType)     { pimpl->focusLost   (changeType); }
z0 XEmbedComponent::broughtToFront()                             { pimpl->broughtToFront(); }
u64 XEmbedComponent::getHostWindowID()                   { return pimpl->getHostWindowID(); }
z0 XEmbedComponent::removeClient()                               { pimpl->setClient (0, true); }
z0 XEmbedComponent::updateEmbeddedBounds()                       { pimpl->updateEmbeddedBounds(); }

//==============================================================================
b8 drx_handleXEmbedEvent (ComponentPeer* p, uk e)
{
    return XEmbedComponent::Pimpl::dispatchX11Event (p, reinterpret_cast<const XEvent*> (e));
}

u64 drx_getCurrentFocusWindow (ComponentPeer* peer)
{
    return (u64) XEmbedComponent::Pimpl::getCurrentFocusWindow (peer);
}

} // namespace drx
