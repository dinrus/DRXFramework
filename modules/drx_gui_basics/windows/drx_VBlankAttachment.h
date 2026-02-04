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
    Helper class to synchronise Component updates to the vertical blank event of the display that
    the Component is presented on. This is useful when animating the Component's contents.

    @tags{Animations}
*/
class DRX_API  VBlankAttachment final  : private ComponentPeer::VBlankListener,
                                          private ComponentListener
{
public:
    /** Default constructor for creating an empty object. */
    VBlankAttachment() = default;

    /** Constructor. Creates an attachment that will call the passed in function at every vertical
        blank event of the display that the passed in Component is currently visible on.

        The Component must be valid for the entire lifetime of the VBlankAttachment.

        You should prefer the other constructor, where the callback also receives a timestamp
        parameter. This constructor is only provided for compatibility with the earlier DRX
        implementation.
    */
    VBlankAttachment (Component* c, std::function<z0()> callbackIn);

    /** Constructor. Creates an attachment that will call the passed in function at every vertical
        blank event of the display that the passed in Component is currently visible on.

        The Component must be valid for the entire lifetime of the VBlankAttachment.

        The provided callback is called with a monotonically increasing value expressed in seconds
        that corresponds to the time of the next frame to be presented. Use this value to
        synchronise drawing across all classes using a VBlankAttachment.
    */
    VBlankAttachment (Component* c, std::function<z0 (f64)> callbackIn);
    VBlankAttachment (VBlankAttachment&& other);
    VBlankAttachment& operator= (VBlankAttachment&& other);

    /** Destructor. */
    ~VBlankAttachment() override;

    /** Returns true for a default constructed object. */
    b8 isEmpty() const { return owner == nullptr; }

private:
    //==============================================================================
    z0 onVBlank (f64 timestampMs) override;

    //==============================================================================
    z0 componentParentHierarchyChanged (Component&) override;

    z0 updateOwner();
    z0 updatePeer();
    z0 cleanup();

    Component* owner = nullptr;
    Component* lastOwner = nullptr;
    std::function<z0 (f64)> callback;
    ComponentPeer* lastPeer = nullptr;

    DRX_DECLARE_NON_COPYABLE (VBlankAttachment)
};

} // namespace drx
