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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 DRX Module Format.md file.


 BEGIN_DRX_MODULE_DECLARATION

  ID:                 drx_gui_basics
  vendor:             drx
  version:            8.0.7
  name:               DRX GUI core classes
  description:        Basic user-interface components and related classes.
  website:            http://www.drx.com/drx
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       drx_graphics drx_data_structures
  OSXFrameworks:      Cocoa QuartzCore
  WeakOSXFrameworks:  Metal MetalKit
  iOSFrameworks:      CoreServices UIKit
  WeakiOSFrameworks:  Metal MetalKit UniformTypeIdentifiers UserNotifications

 END_DRX_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define DRX_GUI_BASICS_H_INCLUDED

#include <drx_graphics/drx_graphics.h>
#include <drx_data_structures/drx_data_structures.h>

//==============================================================================
/** Config: DRX_ENABLE_REPAINT_DEBUGGING
    If this option is turned on, each area of the screen that gets repainted will
    flash in a random colour, so that you can see exactly which bits of your
    components are being drawn.
*/
#ifndef DRX_ENABLE_REPAINT_DEBUGGING
 #define DRX_ENABLE_REPAINT_DEBUGGING 0
#endif

/** Config: DRX_USE_XRANDR
    Enables Xrandr multi-monitor support (Linux only).
    Unless you specifically want to disable this, it's best to leave this option turned on.
    Note that your users do not need to have Xrandr installed for your DRX app to run, as
    the availability of Xrandr is queried during runtime.
*/
#ifndef DRX_USE_XRANDR
 #define DRX_USE_XRANDR 1
#endif

/** Config: DRX_USE_XINERAMA
    Enables Xinerama multi-monitor support (Linux only).
    Unless you specifically want to disable this, it's best to leave this option turned on.
    This will be used as a fallback if DRX_USE_XRANDR not set or libxrandr cannot be found.
    Note that your users do not need to have Xinerama installed for your DRX app to run, as
    the availability of Xinerama is queried during runtime.
*/
#ifndef DRX_USE_XINERAMA
 #define DRX_USE_XINERAMA 1
#endif

/** Config: DRX_USE_XSHM
    Enables X shared memory for faster rendering on Linux. This is best left turned on
    unless you have a good reason to disable it.
*/
#ifndef DRX_USE_XSHM
 #define DRX_USE_XSHM 1
#endif

/** Config: DRX_USE_XRENDER
    Enables XRender to allow semi-transparent windowing on Linux.
*/
#ifndef DRX_USE_XRENDER
 #define DRX_USE_XRENDER 0
#endif

/** Config: DRX_USE_XCURSOR
    Uses XCursor to allow ARGB cursor on Linux. This is best left turned on unless you have
    a good reason to disable it.
*/
#ifndef DRX_USE_XCURSOR
 #define DRX_USE_XCURSOR 1
#endif

/** Config: DRX_WIN_PER_MONITOR_DPI_AWARE
    Enables per-monitor DPI awareness on Windows 8.1 and above.
*/
#ifndef DRX_WIN_PER_MONITOR_DPI_AWARE
 #define DRX_WIN_PER_MONITOR_DPI_AWARE 1
#endif

//==============================================================================
namespace drx
{
    class Component;
    class LookAndFeel;
    class MouseInputSource;
    class ComponentPeer;
    class MouseEvent;
    struct MouseWheelDetails;
    struct PenDetails;
    class ToggleButton;
    class TextButton;
    class AlertWindow;
    class TextLayout;
    class ScrollBar;
    class ComboBox;
    class Button;
    class FilenameComponent;
    class ResizableWindow;
    class MenuBarComponent;
    class GlyphArrangement;
    class TableHeaderComponent;
    class Toolbar;
    class PopupMenu;
    class ProgressBar;
    class FileBrowserComponent;
    class DirectoryContentsDisplayComponent;
    class FilePreviewComponent;
    class CallOutBox;
    class Drawable;
    class DrawablePath;
    class DrawableComposite;
    class CaretComponent;
    class KeyPressMappingSet;
    class ApplicationCommandManagerListener;
    class DrawableButton;
    class Displays;
    class AccessibilityHandler;
    class KeyboardFocusTraverser;

    class FlexBox;
    class Grid;
    class FocusOutline;

   #if DRX_MAC || DRX_WINDOWS || DRX_LINUX || DRX_BSD
    Image createSnapshotOfNativeWindow (uk nativeWindowHandle);
   #endif

    namespace detail
    {
        struct ComponentHelpers;
        class MouseInputSourceImpl;
        class MouseInputSourceList;
        class PointerState;
        class ScopedMessageBoxImpl;
        class ToolbarItemDragAndDropOverlayComponent;
        class TopLevelWindowManager;
    } // namespace detail

} // namespace drx

#include "mouse/drx_MouseCursor.h"
#include "mouse/drx_MouseListener.h"
#include "keyboard/drx_ModifierKeys.h"
#include "mouse/drx_MouseInputSource.h"
#include "mouse/drx_MouseEvent.h"
#include "keyboard/drx_KeyPress.h"
#include "keyboard/drx_KeyListener.h"
#include "components/drx_ComponentTraverser.h"
#include "components/drx_FocusTraverser.h"
#include "components/drx_ModalComponentManager.h"
#include "components/drx_ComponentListener.h"
#include "components/drx_CachedComponentImage.h"
#include "components/drx_Component.h"
#include "layout/drx_ComponentAnimator.h"
#include "desktop/drx_Desktop.h"
#include "desktop/drx_Displays.h"
#include "layout/drx_ComponentBoundsConstrainer.h"
#include "layout/drx_BorderedComponentBoundsConstrainer.h"
#include "mouse/drx_ComponentDragger.h"
#include "mouse/drx_DragAndDropTarget.h"
#include "mouse/drx_DragAndDropContainer.h"
#include "mouse/drx_FileDragAndDropTarget.h"
#include "mouse/drx_SelectedItemSet.h"
#include "mouse/drx_MouseInactivityDetector.h"
#include "mouse/drx_TextDragAndDropTarget.h"
#include "mouse/drx_TooltipClient.h"
#include "keyboard/drx_CaretComponent.h"
#include "keyboard/drx_KeyboardFocusTraverser.h"
#include "keyboard/drx_SystemClipboard.h"
#include "keyboard/drx_TextEditorKeyMapper.h"
#include "keyboard/drx_TextInputTarget.h"
#include "commands/drx_ApplicationCommandID.h"
#include "commands/drx_ApplicationCommandInfo.h"
#include "commands/drx_ApplicationCommandTarget.h"
#include "commands/drx_ApplicationCommandManager.h"
#include "commands/drx_KeyPressMappingSet.h"
#include "buttons/drx_Button.h"
#include "buttons/drx_ArrowButton.h"
#include "buttons/drx_DrawableButton.h"
#include "buttons/drx_HyperlinkButton.h"
#include "buttons/drx_ImageButton.h"
#include "buttons/drx_ShapeButton.h"
#include "buttons/drx_TextButton.h"
#include "buttons/drx_ToggleButton.h"
#include "layout/drx_AnimatedPosition.h"
#include "layout/drx_AnimatedPositionBehaviours.h"
#include "layout/drx_ComponentBuilder.h"
#include "layout/drx_ComponentMovementWatcher.h"
#include "layout/drx_ConcertinaPanel.h"
#include "layout/drx_GroupComponent.h"
#include "layout/drx_ResizableBorderComponent.h"
#include "layout/drx_ResizableCornerComponent.h"
#include "layout/drx_ResizableEdgeComponent.h"
#include "layout/drx_ScrollBar.h"
#include "layout/drx_StretchableLayoutManager.h"
#include "layout/drx_StretchableLayoutResizerBar.h"
#include "layout/drx_StretchableObjectResizer.h"
#include "layout/drx_TabbedButtonBar.h"
#include "layout/drx_TabbedComponent.h"
#include "accessibility/interfaces/drx_AccessibilityCellInterface.h"
#include "accessibility/interfaces/drx_AccessibilityTableInterface.h"
#include "accessibility/interfaces/drx_AccessibilityTextInterface.h"
#include "accessibility/interfaces/drx_AccessibilityValueInterface.h"
#include "accessibility/enums/drx_AccessibilityActions.h"
#include "accessibility/enums/drx_AccessibilityEvent.h"
#include "accessibility/enums/drx_AccessibilityRole.h"
#include "accessibility/drx_AccessibilityState.h"
#include "accessibility/drx_AccessibilityHandler.h"
#include "drawables/drx_Drawable.h"
#include "layout/drx_Viewport.h"
#include "menus/drx_PopupMenu.h"
#include "menus/drx_MenuBarModel.h"
#include "menus/drx_MenuBarComponent.h"
#include "positioning/drx_RelativeCoordinate.h"
#include "positioning/drx_MarkerList.h"
#include "positioning/drx_RelativePoint.h"
#include "positioning/drx_RelativeRectangle.h"
#include "positioning/drx_RelativeCoordinatePositioner.h"
#include "positioning/drx_RelativeParallelogram.h"
#include "positioning/drx_RelativePointPath.h"
#include "drawables/drx_DrawableShape.h"
#include "drawables/drx_DrawableComposite.h"
#include "drawables/drx_DrawableImage.h"
#include "drawables/drx_DrawablePath.h"
#include "drawables/drx_DrawableRectangle.h"
#include "drawables/drx_DrawableText.h"
#include "widgets/drx_TextEditor.h"
#include "widgets/drx_Label.h"
#include "widgets/drx_ComboBox.h"
#include "widgets/drx_ImageComponent.h"
#include "widgets/drx_ListBox.h"
#include "widgets/drx_ProgressBar.h"
#include "widgets/drx_Slider.h"
#include "widgets/drx_TableHeaderComponent.h"
#include "widgets/drx_TableListBox.h"
#include "widgets/drx_Toolbar.h"
#include "widgets/drx_ToolbarItemComponent.h"
#include "widgets/drx_ToolbarItemFactory.h"
#include "widgets/drx_ToolbarItemPalette.h"
#include "menus/drx_BurgerMenuComponent.h"
#include "buttons/drx_ToolbarButton.h"
#include "misc/drx_DropShadower.h"
#include "misc/drx_FocusOutline.h"
#include "widgets/drx_TreeView.h"
#include "windows/drx_TopLevelWindow.h"
#include "windows/drx_MessageBoxOptions.h"
#include "windows/drx_ScopedMessageBox.h"
#include "windows/drx_AlertWindow.h"
#include "windows/drx_CallOutBox.h"
#include "windows/drx_ComponentPeer.h"
#include "windows/drx_ResizableWindow.h"
#include "windows/drx_DocumentWindow.h"
#include "windows/drx_DialogWindow.h"
#include "windows/drx_NativeMessageBox.h"
#include "windows/drx_ThreadWithProgressWindow.h"
#include "windows/drx_TooltipWindow.h"
#include "windows/drx_VBlankAttachment.h"
#include "windows/drx_WindowUtils.h"
#include "windows/drx_NativeScaleFactorNotifier.h"
#include "layout/drx_MultiDocumentPanel.h"
#include "layout/drx_SidePanel.h"
#include "filebrowser/drx_FileBrowserListener.h"
#include "filebrowser/drx_DirectoryContentsList.h"
#include "filebrowser/drx_DirectoryContentsDisplayComponent.h"
#include "filebrowser/drx_FileBrowserComponent.h"
#include "filebrowser/drx_FileChooser.h"
#include "filebrowser/drx_FileChooserDialogBox.h"
#include "filebrowser/drx_FileListComponent.h"
#include "filebrowser/drx_FilenameComponent.h"
#include "filebrowser/drx_FilePreviewComponent.h"
#include "filebrowser/drx_FileSearchPathListComponent.h"
#include "filebrowser/drx_FileTreeComponent.h"
#include "filebrowser/drx_ImagePreviewComponent.h"
#include "filebrowser/drx_ContentSharer.h"
#include "properties/drx_PropertyComponent.h"
#include "properties/drx_BooleanPropertyComponent.h"
#include "properties/drx_ButtonPropertyComponent.h"
#include "properties/drx_ChoicePropertyComponent.h"
#include "properties/drx_PropertyPanel.h"
#include "properties/drx_SliderPropertyComponent.h"
#include "properties/drx_TextPropertyComponent.h"
#include "properties/drx_MultiChoicePropertyComponent.h"
#include "application/drx_Application.h"
#include "misc/drx_BubbleComponent.h"
#include "lookandfeel/drx_LookAndFeel.h"
#include "lookandfeel/drx_LookAndFeel_V2.h"
#include "lookandfeel/drx_LookAndFeel_V1.h"
#include "lookandfeel/drx_LookAndFeel_V3.h"
#include "lookandfeel/drx_LookAndFeel_V4.h"
#include "mouse/drx_LassoComponent.h"

#if DRX_LINUX || DRX_BSD
 #if DRX_GUI_BASICS_INCLUDE_XHEADERS
  // If you're missing these headers, you need to install the libx11-dev package
  DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wvariadic-macros")
  #include <X11/Xlib.h>
  DRX_END_IGNORE_WARNINGS_GCC_LIKE
  #include <X11/Xatom.h>
  #include <X11/Xresource.h>
  #include <X11/Xutil.h>
  #include <X11/Xmd.h>
  #include <X11/keysym.h>
  #include <X11/XKBlib.h>
  #include <X11/cursorfont.h>
  #include <unistd.h>

  #if DRX_USE_XRANDR
   // If you're missing this header, you need to install the libxrandr-dev package
   #include <X11/extensions/Xrandr.h>
  #endif

  #if DRX_USE_XINERAMA
   // If you're missing this header, you need to install the libxinerama-dev package
   #include <X11/extensions/Xinerama.h>
  #endif

  #if DRX_USE_XSHM
   #include <X11/extensions/XShm.h>
   #include <sys/shm.h>
   #include <sys/ipc.h>
  #endif

  #if DRX_USE_XRENDER
   // If you're missing these headers, you need to install the libxrender-dev and libxcomposite-dev packages
   #include <X11/extensions/Xrender.h>
   #include <X11/extensions/Xcomposite.h>
  #endif

  #if DRX_USE_XCURSOR
   // If you're missing this header, you need to install the libxcursor-dev package
   #include <X11/Xcursor/Xcursor.h>
  #endif

  #undef SIZEOF
  #undef KeyPress

  #include "native/drx_XWindowSystem_linux.h"
  #include "native/drx_XSymbols_linux.h"
 #endif
#endif

#if DRX_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER && DRX_WINDOWS
 #include "native/drx_ScopedThreadDPIAwarenessSetter_windows.h"
#endif

#include "layout/drx_FlexItem.h"
#include "layout/drx_FlexBox.h"

#include "layout/drx_GridItem.h"
#include "layout/drx_Grid.h"
#include "native/drx_ScopedDPIAwarenessDisabler.h"
