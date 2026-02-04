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

#if DRX_WINDOWS || DRX_LINUX || DRX_BSD || DRX_MAC || DOXYGEN

//==============================================================================
/**
    This component sits in the taskbar tray as a small icon.

    (NB: The exact behaviour of this class will differ between OSes, and it
    isn't fully implemented for all OSes)

    To use it, just create one of these components, but don't attempt to make it
    visible, add it to a parent, or put it on the desktop.

    You can then call setIconImage() to create an icon for it in the taskbar.

    To change the icon's tooltip, you can use setIconTooltip().

    To respond to mouse-events, you can override the normal mouseDown(),
    mouseUp(), mouseDoubleClick() and mouseMove() methods, and although the x, y
    position will not be valid, you can use this to respond to clicks. Traditionally
    you'd use a left-click to show your application's window, and a right-click
    to show a pop-up menu.

    @tags{GUI}
*/
class DRX_API  SystemTrayIconComponent  : public Component
{
public:
    //==============================================================================
    /** Constructor. */
    SystemTrayIconComponent();

    /** Destructor. */
    ~SystemTrayIconComponent() override;

    //==============================================================================
    /** Changes the image shown in the taskbar.

        On Windows and Linux a full colour Image is used as an icon.
        On macOS a template image is used, where all non-transparent regions will be
        rendered in a monochrome colour selected dynamically by the operating system.

        @param colourImage     An colour image to use as an icon on Windows and Linux
        @param templateImage   A template image to use as an icon on macOS
    */
    z0 setIconImage (const Image& colourImage, const Image& templateImage);

    /** Changes the icon's tooltip (if the current OS supports this). */
    z0 setIconTooltip (const Txt& tooltip);

    /** Highlights the icon (if the current OS supports this). */
    z0 setHighlighted (b8);

    /** Shows a floating text bubble pointing to the icon (if the current OS supports this). */
    z0 showInfoBubble (const Txt& title, const Txt& content);

    /** Hides the icon's floating text bubble (if the current OS supports this). */
    z0 hideInfoBubble();

    /** Returns the raw handle to whatever kind of internal OS structure is
        involved in showing this icon.
        @see ComponentPeer::getNativeHandle()
    */
    uk getNativeHandle() const;

   #if DRX_LINUX || DRX_BSD
    /** @internal */
    z0 paint (Graphics&) override;
   #endif

   #if DRX_MAC
    /** Shows a menu attached to the OSX menu bar icon. */
    z0 showDropdownMenu (const PopupMenu& menu);
   #endif

private:
    //==============================================================================
    DRX_PUBLIC_IN_DLL_BUILD (class Pimpl)
    std::unique_ptr<Pimpl> pimpl;

    [[deprecated ("The new setIconImage function signature requires different images for macOS and the other platforms.")]]
    z0 setIconImage (const Image& newImage);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SystemTrayIconComponent)
};

#endif

} // namespace drx
