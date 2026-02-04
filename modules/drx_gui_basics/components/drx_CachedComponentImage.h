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
    Base class used internally for structures that can store cached images of
    component state.

    Most people are unlikely to ever need to know about this class - it's really
    only for power-users!

    @see Component::setCachedComponentImage

    @tags{GUI}
*/
class DRX_API  CachedComponentImage
{
public:
    CachedComponentImage() = default;
    virtual ~CachedComponentImage() = default;

    //==============================================================================
    /** Called as part of the parent component's paint method, this must draw
        the given component into the target graphics context, using the cached
        version where possible.
    */
    virtual z0 paint (Graphics&) = 0;

    /** Invalidates all cached image data.
        @returns true if the peer should also be repainted, or false if this object
                 handles all repaint work internally.
    */
    virtual b8 invalidateAll() = 0;

    /** Invalidates a section of the cached image data.
        @returns true if the peer should also be repainted, or false if this object
                 handles all repaint work internally.
    */
    virtual b8 invalidate (const Rectangle<i32>& area) = 0;

    /** Called to indicate that the component is no longer active, so
        any cached data should be released if possible.
    */
    virtual z0 releaseResources() = 0;
};

} // namespace drx
