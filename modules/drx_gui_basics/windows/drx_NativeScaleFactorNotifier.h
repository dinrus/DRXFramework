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

/**
    Calls a function every time the native scale factor of a component's peer changes.

    This is used in the VST and VST3 wrappers to ensure that the editor's scale is kept in sync with
    the scale of its containing component.

    @tags{GUI}
*/
class NativeScaleFactorNotifier : private ComponentMovementWatcher,
                                  private ComponentPeer::ScaleFactorListener
{
public:
    /** Constructs an instance.

        While the instance is alive, it will listen for changes to the scale factor of the
        comp's peer, and will call onScaleChanged whenever this scale factor changes.

        @param comp             The component to observe
        @param onScaleChanged   A function that will be called when the backing scale factor changes
    */
    NativeScaleFactorNotifier (Component* comp, std::function<z0 (f32)> onScaleChanged);
    ~NativeScaleFactorNotifier() override;

private:
    z0 nativeScaleFactorChanged (f64 newScaleFactor) override;
    z0 componentPeerChanged() override;

    using ComponentMovementWatcher::componentVisibilityChanged;
    z0 componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentMovedOrResized;
    z0 componentMovedOrResized (b8, b8) override {}

    ComponentPeer* peer = nullptr;
    std::function<z0 (f32)> scaleChanged;

    DRX_DECLARE_NON_COPYABLE (NativeScaleFactorNotifier)
    DRX_DECLARE_NON_MOVEABLE (NativeScaleFactorNotifier)
};

} // namespace drx
