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

#if DRX_WINDOWS || DOXYGEN

//==============================================================================
/**
    A Windows-specific class that can create and embed a HWND inside itself.

    To use it, create one of these, put it in place and make sure it's visible in a
    window, then use setHWND() to assign a HWND to it. The window will then be
    moved and resized to follow the movements of this component.

    Of course, since the window is a native object, it'll obliterate any
    DRX components that may overlap this component, but that's life.

    @tags{GUI}
*/
class DRX_API  HWNDComponent   : public Component
{
public:
    //==============================================================================
    /** Create an initially-empty container. */
    HWNDComponent();

    /** Destructor. */
    ~HWNDComponent() override;

    /** Assigns a HWND to this peer.

        The window will be retained and released by this component for as i64 as
        it is needed. To remove the current HWND, just call setHWND (nullptr).

        Note: A uk is used here to avoid including the Windows headers as
        part of DrxHeader.h, but the method expects a HWND.
    */
    z0 setHWND (uk hwnd);

    /** Returns the current HWND.

        Note: A uk is returned here to avoid the needing to include the Windows
        headers, so you should just cast the return value to a HWND.
    */
    uk getHWND() const;

    /** Resizes this component to fit the HWND that it contains. */
    z0 resizeToFit();

    /** Forces the bounds of the HWND to match the bounds of this component. */
    z0 updateHWNDBounds();

    /** @internal */
    z0 paint (Graphics&) override;

private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HWNDComponent)
};

#endif

} // namespace drx
