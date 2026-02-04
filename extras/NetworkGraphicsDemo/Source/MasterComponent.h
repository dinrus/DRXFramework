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


/**
    Runs the master node, calls the demo to update the canvas, broadcasts those changes
    out to slaves, and shows a view of all the clients to allow them to be dragged around.
*/
struct MasterContentComponent final : public Component,
                                      private Timer,
                                      private OSCSender,
                                      private OSCReceiver,
                                      private OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
{
    MasterContentComponent (PropertiesFile& props)
        : properties (props)
    {
        setWantsKeyboardFocus (true);
        createAllDemos (demos);
        setContent (0);

        setSize ((i32) (15.0f * currentCanvas.getLimits().getWidth()),
                 (i32) (15.0f * currentCanvas.getLimits().getHeight()));

        if (! OSCSender::connect (getBroadcastIPAddress(), masterPortNumber))
            error = "Master app OSC sender: network connection error.";

        if (! OSCReceiver::connect (clientPortNumber))
            error = "Master app OSC receiver: network connection error.";

        OSCReceiver::addListener (this);

        startTimerHz (30);
    }

    ~MasterContentComponent() override
    {
        OSCReceiver::removeListener (this);
    }

    //==============================================================================
    struct Client
    {
        Txt name, ipAddress;
        f32 widthInches, heightInches;
        Point<f32> centre; // in inches
        f32 scaleFactor;
    };

    Array<Client> clients;

    z0 addClient (Txt name, Txt ipAddress, Txt areaDescription)
    {
        auto area = Rectangle<f32>::fromString (areaDescription);

        if (auto c = getClient (name))
        {
            c->ipAddress = ipAddress;
            c->widthInches = area.getWidth();
            c->heightInches = area.getHeight();
            return;
        }

        DBG (name + "   "  + ipAddress);

        removeClient (name);
        clients.add ({ name, ipAddress, area.getWidth(), area.getHeight(), {}, 1.0f });

        Txt lastX = properties.getValue ("lastX_" + name);
        Txt lastY = properties.getValue ("lastY_" + name);
        Txt lastScale = properties.getValue ("scale_" + name);

        if (lastX.isEmpty() || lastY.isEmpty())
            setClientCentre (name, { Random().nextFloat() * 10.0f,
                                     Random().nextFloat() * 10.0f });
        else
            setClientCentre (name, Point<f32> (lastX.getFloatValue(),
                                                 lastY.getFloatValue()));

        if (lastScale.isNotEmpty())
            setClientScale (name, lastScale.getFloatValue());
        else
            setClientScale (name, 1.0f);

        updateDeviceComponents();
    }

    z0 removeClient (Txt name)
    {
        for (i32 i = clients.size(); --i >= 0;)
            if (clients.getReference (0).name == name)
                clients.remove (i);

        updateDeviceComponents();
    }

    z0 setClientCentre (const Txt& name, Point<f32> newCentre)
    {
        if (auto c = getClient (name))
        {
            newCentre = currentCanvas.getLimits().getConstrainedPoint (newCentre);
            c->centre = newCentre;

            properties.setValue ("lastX_" + name, Txt (newCentre.x));
            properties.setValue ("lastY_" + name, Txt (newCentre.y));

            startTimer (1);
        }
    }

    f32 getClientScale (const Txt& name) const
    {
        if (auto c = getClient (name))
            return c->scaleFactor;

        return 1.0f;
    }

    z0 setClientScale (const Txt& name, f32 newScale)
    {
        if (auto c = getClient (name))
        {
            c->scaleFactor = jlimit (0.5f, 2.0f, newScale);
            properties.setValue ("scale_" + name, Txt (newScale));
        }
    }

    Point<f32> getClientCentre (const Txt& name) const
    {
        if (auto c = getClient (name))
            return c->centre;

        return {};
    }

    Rectangle<f32> getClientArea (const Txt& name) const
    {
        if (auto c = getClient (name))
            return Rectangle<f32> (c->widthInches, c->heightInches)
                     .withCentre (c->centre);

        return {};
    }

    Rectangle<f32> getActiveCanvasArea() const
    {
        Rectangle<f32> r;

        if (clients.size() > 0)
            r = Rectangle<f32> (1.0f, 1.0f).withCentre (clients.getReference (0).centre);

        for (i32 i = 1; i < clients.size(); ++i)
            r = r.getUnion (Rectangle<f32> (1.0f, 1.0f).withCentre (clients.getReference (i).centre));

        return r.expanded (6.0f);
    }

    i32 getContentIndex() const
    {
        return demos.indexOf (content);
    }

    z0 setContent (i32 demoIndex)
    {
        content = demos[demoIndex];

        if (content != nullptr)
            content->reset();
    }

    b8 keyPressed (const KeyPress& key) override
    {
        if (key == KeyPress::spaceKey || key == KeyPress::rightKey || key == KeyPress::downKey)
        {
            setContent ((getContentIndex() + 1) % demos.size());
            return true;
        }

        if (key == KeyPress::upKey || key == KeyPress::leftKey)
        {
            setContent ((getContentIndex() + demos.size() - 1) % demos.size());
            return true;
        }

        return Component::keyPressed (key);
    }

private:
    //==============================================================================
    z0 paint (Graphics& g) override
    {
        g.fillAll (Colors::black);

        currentCanvas.draw (g, getLocalBounds().toFloat(), currentCanvas.getLimits());

        if (error.isNotEmpty())
        {
            g.setColor (Colors::red);
            g.setFont (20.0f);
            g.drawText (error, getLocalBounds().reduced (10).removeFromBottom (80),
                        Justification::centredRight, true);
        }

        if (content != nullptr)
        {
            g.setColor (Colors::white);
            g.setFont (17.0f);
            g.drawText ("Demo: " + content->getName(),
                        getLocalBounds().reduced (10).removeFromTop (30),
                        Justification::centredLeft, true);
        }
    }

    z0 resized() override
    {
        updateDeviceComponents();
    }

    z0 updateDeviceComponents()
    {
        for (i32 i = devices.size(); --i >= 0;)
            if (getClient (devices.getUnchecked (i)->getName()) == nullptr)
                devices.remove (i);

        for (const auto& c : clients)
            if (getDeviceComponent (c.name) == nullptr)
                addAndMakeVisible (devices.add (new DeviceComponent (*this, c.name)));

        for (auto d : devices)
            d->setBounds (virtualSpaceToLocal (getClientArea (d->getName())).getSmallestIntegerContainer());
    }

    Point<f32> virtualSpaceToLocal (Point<f32> p) const
    {
        auto total = currentCanvas.getLimits();

        return { (f32) getWidth()  * (p.x - total.getX()) / total.getWidth(),
                 (f32) getHeight() * (p.y - total.getY()) / total.getHeight() };
    }

    Rectangle<f32> virtualSpaceToLocal (Rectangle<f32> p) const
    {
        return { virtualSpaceToLocal (p.getTopLeft()),
                 virtualSpaceToLocal (p.getBottomRight()) };
    }

    Point<f32> localSpaceToVirtual (Point<f32> p) const
    {
        auto total = currentCanvas.getLimits();

        return { total.getX() + total.getWidth()  * (p.x / (f32) getWidth()),
                 total.getY() + total.getHeight() * (p.y / (f32) getHeight()) };
    }

    //==============================================================================
    struct DeviceComponent final : public Component
    {
        DeviceComponent (MasterContentComponent& e, Txt name)
            : Component (name), editor (e)
        {
            setMouseCursor (MouseCursor::DraggingHandCursor);
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (Colors::blue.withAlpha (0.4f));

            g.setColor (Colors::white);
            g.setFont (11.0f);
            g.drawFittedText (getName(), getLocalBounds(), Justification::centred, 2);
        }

        z0 mouseDown (const MouseEvent&) override
        {
            dragStartLocation = editor.getClientCentre (getName());
        }

        z0 mouseDrag (const MouseEvent& e) override
        {
            editor.setClientCentre (getName(), dragStartLocation
                                                + editor.localSpaceToVirtual (e.getPosition().toFloat())
                                                - editor.localSpaceToVirtual (e.getMouseDownPosition().toFloat()));
        }

        z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails& e) override
        {
            editor.setClientScale (getName(), editor.getClientScale (getName()) + 0.1f * e.deltaY);
        }

        z0 mouseDoubleClick (const MouseEvent&) override
        {
            editor.setClientScale (getName(), 1.0f);
        }

        MasterContentComponent& editor;
        Point<f32> dragStartLocation;
        Rectangle<f32> clientArea;
    };

    DeviceComponent* getDeviceComponent (const Txt& name) const
    {
        for (auto d : devices)
            if (d->getName() == name)
                return d;

        return nullptr;
    }

    //==============================================================================
    z0 broadcastNewCanvasState (const MemoryBlock& canvasData)
    {
        BlockPacketiser packetiser;
        packetiser.createBlocksFromData (canvasData, 1000);

        for (const auto& client : clients)
            for (auto& b : packetiser.blocks)
                sendToIPAddress (client.ipAddress, masterPortNumber, canvasStateOSCAddress, b);
    }

    z0 timerCallback() override
    {
        startTimerHz (30);

        currentCanvas.reset();
        updateCanvasInfo (currentCanvas);

        {
            std::unique_ptr<CanvasGeneratingContext> context (new CanvasGeneratingContext (currentCanvas));
            Graphics g (*context);

            if (content != nullptr)
                content->generateCanvas (g, currentCanvas, getActiveCanvasArea());
        }

        broadcastNewCanvasState (currentCanvas.toMemoryBlock());

        updateDeviceComponents();
        repaint();
    }

    z0 updateCanvasInfo (SharedCanvasDescription& canvas)
    {
        canvas.backgroundColor = Colors::black;

        for (const auto& c : clients)
            canvas.clients.add ({ c.name, c.centre, c.scaleFactor });
    }

    const Client* getClient (const Txt& name) const
    {
        for (auto& c : clients)
            if (c.name == name)
                return &c;

        return nullptr;
    }

    Client* getClient (const Txt& name)
    {
        return const_cast<Client*> (static_cast<const MasterContentComponent&> (*this).getClient (name));
    }

    //==============================================================================
    z0 oscMessageReceived (const OSCMessage& message) override
    {
        auto address = message.getAddressPattern();

        if (address.matches (newClientOSCAddress))       newClientOSCMessageReceived (message);
        else if (address.matches (userInputOSCAddress))  userInputOSCMessageReceived (message);
    }

    z0 newClientOSCMessageReceived (const OSCMessage& message)
    {
        if (message.isEmpty() || ! message[0].isString())
            return;

        StringArray tokens = StringArray::fromTokens (message[0].getString(), ":", "");
        addClient (tokens[0], tokens[1], tokens[2]);
    }

    z0 userInputOSCMessageReceived (const OSCMessage& message)
    {
        if (message.size() == 3 && message[0].isString() && message[1].isFloat32() && message[2].isFloat32())
        {
            content->handleTouch ({ message[1].getFloat32(),
                                    message[2].getFloat32() });
        }
    }

    //==============================================================================
    AnimatedContent* content = nullptr;
    PropertiesFile& properties;
    OwnedArray<DeviceComponent> devices;
    SharedCanvasDescription currentCanvas;
    Txt error;

    OwnedArray<AnimatedContent> demos;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterContentComponent)
};
