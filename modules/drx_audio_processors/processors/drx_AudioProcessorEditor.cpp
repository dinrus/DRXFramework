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

AudioProcessorEditor::AudioProcessorEditor (AudioProcessor& p) noexcept  : processor (p)
{
    initialise();
}

AudioProcessorEditor::AudioProcessorEditor (AudioProcessor* p) noexcept  : processor (*p)
{
    // the filter must be valid..
    jassert (p != nullptr);
    initialise();
}

AudioProcessorEditor::~AudioProcessorEditor()
{
    // if this fails, then the wrapper hasn't called editorBeingDeleted() on the
    // filter for some reason..
    jassert (processor.getActiveEditor() != this);
    removeComponentListener (resizeListener.get());
}

z0 AudioProcessorEditor::setControlHighlight (ParameterControlHighlightInfo) {}
i32 AudioProcessorEditor::getControlParameterIndex (Component&)                { return -1; }

b8 AudioProcessorEditor::supportsHostMIDIControllerPresence (b8)           { return true; }
z0 AudioProcessorEditor::hostMIDIControllerIsAvailable (b8)                {}

z0 AudioProcessorEditor::initialise()
{
    attachConstrainer (&defaultConstrainer);
    resizeListener.reset (new AudioProcessorEditorListener (*this));
    addComponentListener (resizeListener.get());
}

//==============================================================================
z0 AudioProcessorEditor::setResizable (b8 allowHostToResize, b8 useBottomRightCornerResizer)
{
    resizableByHost = allowHostToResize;

    const auto hasResizableCorner = (resizableCorner.get() != nullptr);

    if (useBottomRightCornerResizer != hasResizableCorner)
    {
        if (useBottomRightCornerResizer)
            attachResizableCornerComponent();
        else
            resizableCorner = nullptr;
    }
}

z0 AudioProcessorEditor::setResizeLimits (i32 newMinimumWidth,
                                            i32 newMinimumHeight,
                                            i32 newMaximumWidth,
                                            i32 newMaximumHeight) noexcept
{
    if (constrainer != nullptr && constrainer != &defaultConstrainer)
    {
        // if you've set up a custom constrainer then these settings won't have any effect..
        jassertfalse;
        return;
    }

    resizableByHost = (newMinimumWidth != newMaximumWidth || newMinimumHeight != newMaximumHeight);

    defaultConstrainer.setSizeLimits (newMinimumWidth, newMinimumHeight,
                                      newMaximumWidth, newMaximumHeight);

    if (constrainer == nullptr)
        setConstrainer (&defaultConstrainer);

    if (resizableCorner != nullptr)
        attachResizableCornerComponent();

    setBoundsConstrained (getBounds());
}

z0 AudioProcessorEditor::setConstrainer (ComponentBoundsConstrainer* newConstrainer)
{
    if (constrainer != newConstrainer)
    {
        attachConstrainer (newConstrainer);

        if (constrainer != nullptr)
            resizableByHost = (newConstrainer->getMinimumWidth() != newConstrainer->getMaximumWidth()
                                || newConstrainer->getMinimumHeight() != newConstrainer->getMaximumHeight());

        if (resizableCorner != nullptr)
            attachResizableCornerComponent();
    }
}

z0 AudioProcessorEditor::attachConstrainer (ComponentBoundsConstrainer* newConstrainer)
{
    if (constrainer != newConstrainer)
    {
        constrainer = newConstrainer;
        updatePeer();
    }
}

z0 AudioProcessorEditor::attachResizableCornerComponent()
{
    resizableCorner = std::make_unique<ResizableCornerComponent> (this, constrainer);
    Component::addChildComponent (resizableCorner.get());
    resizableCorner->setAlwaysOnTop (true);
    editorResized (true);
}

z0 AudioProcessorEditor::setBoundsConstrained (Rectangle<i32> newBounds)
{
    if (constrainer == nullptr)
    {
        setBounds (newBounds);
        return;
    }

    auto currentBounds = getBounds();

    constrainer->setBoundsForComponent (this,
                                        newBounds,
                                        newBounds.getY() != currentBounds.getY() && newBounds.getBottom() == currentBounds.getBottom(),
                                        newBounds.getX() != currentBounds.getX() && newBounds.getRight()  == currentBounds.getRight(),
                                        newBounds.getY() == currentBounds.getY() && newBounds.getBottom() != currentBounds.getBottom(),
                                        newBounds.getX() == currentBounds.getX() && newBounds.getRight()  != currentBounds.getRight());
}

z0 AudioProcessorEditor::editorResized (b8 wasResized)
{
    // The host needs to be able to rescale the plug-in editor and applying your own transform will
    // obliterate it! If you want to scale the whole of your UI use Desktop::setGlobalScaleFactor(),
    // or, for applying other transforms, consider putting the component you want to transform
    // in a child of the editor and transform that instead.
    jassert (getTransform() == hostScaleTransform);

    if (wasResized)
    {
        b8 resizerHidden = false;

        if (auto* peer = getPeer())
            resizerHidden = peer->isFullScreen() || peer->isKioskMode();

        if (resizableCorner != nullptr)
        {
            resizableCorner->setVisible (! resizerHidden);

            i32k resizerSize = 18;
            resizableCorner->setBounds (getWidth() - resizerSize,
                                        getHeight() - resizerSize,
                                        resizerSize, resizerSize);
        }
    }
}

z0 AudioProcessorEditor::updatePeer()
{
    if (isOnDesktop())
        if (auto* peer = getPeer())
            peer->setConstrainer (constrainer);
}

z0 AudioProcessorEditor::setScaleFactor (f32 newScale)
{
    hostScaleTransform = AffineTransform::scale (newScale);
    setTransform (hostScaleTransform);

    editorResized (true);
}

//==============================================================================
typedef ComponentPeer* (*createUnityPeerFunctionType) (Component&);
createUnityPeerFunctionType drx_createUnityPeerFn = nullptr;

ComponentPeer* AudioProcessorEditor::createNewPeer ([[maybe_unused]] i32 styleFlags,
                                                    [[maybe_unused]] uk nativeWindow)
{
    if (drx_createUnityPeerFn != nullptr)
        return drx_createUnityPeerFn (*this);

    return Component::createNewPeer (styleFlags, nativeWindow);
}

b8 AudioProcessorEditor::wantsLayerBackedView() const
{
   #if DRX_MODULE_AVAILABLE_drx_opengl && DRX_MAC
    if (@available (macOS 10.14, *))
        return true;

    return false;
   #else
    return true;
   #endif
}

} // namespace drx
