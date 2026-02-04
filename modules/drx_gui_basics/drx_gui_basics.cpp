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

#ifdef DRX_GUI_BASICS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of DRX cpp file"
#endif

#define NS_FORMAT_FUNCTION(F,A) // To avoid spurious warnings from GCC

#define DRX_CORE_INCLUDE_OBJC_HELPERS 1
#define DRX_CORE_INCLUDE_COM_SMART_PTR 1
#define DRX_CORE_INCLUDE_JNI_HELPERS 1
#define DRX_CORE_INCLUDE_NATIVE_HEADERS 1
#define DRX_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW 1
#define DRX_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1
#define DRX_GUI_BASICS_INCLUDE_XHEADERS 1
#define DRX_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1

#include "drx_gui_basics.h"

#include <cctype>

//==============================================================================
#ifdef DRX_DISPLAY_SPLASH_SCREEN
 DRX_COMPILER_WARNING ("This version of DRX does not use the splash screen, the flag DRX_DISPLAY_SPLASH_SCREEN is ignored")
#endif

#ifdef DRX_USE_DARK_SPLASH_SCREEN
 DRX_COMPILER_WARNING ("This version of DRX does not use the splash screen, the flag DRX_USE_DARK_SPLASH_SCREEN is ignored")
#endif

//==============================================================================
#if DRX_MAC
 #import <IOKit/pwr_mgt/IOPMLib.h>
 #import <MetalKit/MetalKit.h>

 #if DRX_MAC_API_VERSION_MIN_REQUIRED_AT_LEAST (14, 4)
  #import <ScreenCaptureKit/ScreenCaptureKit.h>
 #endif

#elif DRX_IOS
 #import <UserNotifications/UserNotifications.h>
 #import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
 #import <MetalKit/MetalKit.h>
 #import <UIKit/UIActivityViewController.h>

//==============================================================================
#elif DRX_WINDOWS
 #include <commctrl.h>
 #include <commdlg.h>
 #include <d2d1_3.h>
 #include <d3d11_2.h>
 #include <dxgi1_3.h>
 #include <sapi.h>
 #include <vfw.h>
 #include <windowsx.h>
 #include <dwmapi.h>

 #if DRX_ETW_TRACELOGGING
  #include <TraceLoggingProvider.h>
  #include <evntrace.h>
 #endif

 #include <uiautomation.h>

 #undef new

 #if DRX_WEB_BROWSER
  #include <exdisp.h>
  #include <exdispid.h>
 #endif

 #if ! DRX_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment(lib, "vfw32.lib")
  #pragma comment(lib, "imm32.lib")
  #pragma comment(lib, "comctl32.lib")
  #pragma comment(lib, "dwmapi.lib")

  // Link a newer version of the side-by-side comctl32 dll.
  // Required to enable the newer native message box and visual styles on vista and above.
  #pragma comment(linker,                             \
          "\"/MANIFESTDEPENDENCY:type='Win32' "       \
          "name='Microsoft.Windows.Common-Controls' " \
          "version='6.0.0.0' "                        \
          "processorArchitecture='*' "                \
          "publicKeyToken='6595b64144ccf1df' "        \
          "language='*'\""                            \
      )
  #if DRX_OPENGL
   #pragma comment(lib, "OpenGL32.Lib")
   #pragma comment(lib, "GlU32.Lib")
  #endif

 #endif
#endif

//==============================================================================
#include <drx_graphics/native/drx_EventTracing.h>

#include "detail/drx_AccessibilityHelpers.h"
#include "detail/drx_ButtonAccessibilityHandler.h"
#include "detail/drx_ScalingHelpers.h"
#include "detail/drx_ComponentHelpers.h"
#include "detail/drx_FocusHelpers.h"
#include "detail/drx_FocusRestorer.h"
#include "detail/drx_ViewportHelpers.h"
#include "detail/drx_LookAndFeelHelpers.h"
#include "detail/drx_PointerState.h"
#include "detail/drx_CustomMouseCursorInfo.h"
#include "detail/drx_MouseInputSourceImpl.h"
#include "detail/drx_MouseInputSourceList.h"
#include "detail/drx_ToolbarItemDragAndDropOverlayComponent.h"
#include "detail/drx_ScopedMessageBoxInterface.h"
#include "detail/drx_ScopedMessageBoxImpl.h"
#include "detail/drx_ScopedContentSharerInterface.h"
#include "detail/drx_ScopedContentSharerImpl.h"
#include "detail/drx_WindowingHelpers.h"
#include "detail/drx_AlertWindowHelpers.h"
#include "detail/drx_TopLevelWindowManager.h"
#include "detail/drx_StandardCachedComponentImage.h"

//==============================================================================
#if DRX_IOS || DRX_WINDOWS
 #include "native/drx_MultiTouchMapper.h"
#endif

#if DRX_ANDROID || DRX_WINDOWS || DRX_IOS || DRX_UNIT_TESTS
 #include "native/accessibility/drx_AccessibilityTextHelpers.h"
#endif

#if DRX_MAC || DRX_IOS
 #include "native/accessibility/drx_AccessibilitySharedCode_mac.mm"
 #include "native/drx_CGMetalLayerRenderer_mac.h"

 #if DRX_IOS
  #include "native/drx_UIViewComponentPeer_ios.mm"
  #include "native/accessibility/drx_Accessibility_ios.mm"
  #include "native/drx_WindowUtils_ios.mm"
  #include "native/drx_Windowing_ios.mm"
  #include "native/drx_NativeMessageBox_ios.mm"
  #include "native/drx_NativeModalWrapperComponent_ios.h"
  #include "native/drx_FileChooser_ios.mm"

  #if DRX_CONTENT_SHARING
   #include "native/drx_ContentSharer_ios.cpp"
  #endif

 #else
  #include "native/accessibility/drx_Accessibility_mac.mm"
  #include "native/drx_PerScreenDisplayLinks_mac.h"
  #include "native/drx_NSViewComponentPeer_mac.mm"
  #include "native/drx_WindowUtils_mac.mm"
  #include "native/drx_Windowing_mac.mm"
  #include "native/drx_NativeMessageBox_mac.mm"
  #include "native/drx_MainMenu_mac.mm"
  #include "native/drx_FileChooser_mac.mm"
  #include "detail/drx_ComponentPeerHelpers.h"
  #include "detail/drx_ComponentPeerHelpers.cpp"
 #endif

 #include "native/drx_MouseCursor_mac.mm"

#elif DRX_WINDOWS
 #include <drx_graphics/native/drx_Direct2DMetrics_windows.h>
 #include <drx_graphics/native/drx_Direct2DGraphicsContext_windows.h>
 #include <drx_graphics/native/drx_Direct2DHwndContext_windows.h>
 #include <drx_graphics/native/drx_DirectX_windows.h>
 #include <drx_graphics/native/drx_Direct2DPixelDataPage_windows.h>
 #include <drx_graphics/native/drx_Direct2DImage_windows.h>
 #include <drx_graphics/native/drx_Direct2DImageContext_windows.h>

 #include "native/accessibility/drx_WindowsUIAWrapper_windows.h"
 #include "native/accessibility/drx_AccessibilityElement_windows.h"
 #include "native/accessibility/drx_UIAHelpers_windows.h"
 #include "native/accessibility/drx_UIAProviders_windows.h"
 #include "native/accessibility/drx_AccessibilityElement_windows.cpp"
 #include "native/accessibility/drx_Accessibility_windows.cpp"
 #include "native/drx_WindowsHooks_windows.h"
 #include "native/drx_WindowUtils_windows.cpp"
 #include "native/drx_VBlank_windows.cpp"
 #include "native/drx_Windowing_windows.cpp"
 #include "native/drx_WindowsHooks_windows.cpp"
 #include "native/drx_NativeMessageBox_windows.cpp"
 #include "native/drx_DragAndDrop_windows.cpp"
 #include "native/drx_FileChooser_windows.cpp"

#elif DRX_LINUX || DRX_BSD
 #include "native/drx_XSymbols_linux.cpp"
 #include "native/drx_DragAndDrop_linux.cpp"

 DRX_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

 #include "native/drx_ScopedWindowAssociation_linux.h"
 #include "native/drx_WindowUtils_linux.cpp"
 #include "native/drx_Windowing_linux.cpp"
 #include "native/drx_NativeMessageBox_linux.cpp"
 #include "native/drx_XWindowSystem_linux.cpp"

 DRX_END_IGNORE_WARNINGS_GCC_LIKE

 #include "native/drx_FileChooser_linux.cpp"

#elif DRX_ANDROID

 #include "drx_core/files/drx_common_MimeTypes.h"
 #include "native/accessibility/drx_Accessibility_android.cpp"
 #include "native/drx_WindowUtils_android.cpp"
 #include "native/drx_Windowing_android.cpp"
 #include "native/drx_NativeMessageBox_android.cpp"
 #include "native/drx_FileChooser_android.cpp"

 #if DRX_CONTENT_SHARING
  #include "native/drx_ContentSharer_android.cpp"
 #endif

#endif

//==============================================================================
// Depends on types defined in platform-specific windowing files
#include "mouse/drx_MouseCursor.cpp"

#if DRX_UNIT_TESTS
 #include "native/accessibility/drx_AccessibilityTextHelpers_test.cpp"
#endif

//==============================================================================
#include "native/accessibility/drx_Accessibility.cpp"
#include "accessibility/drx_AccessibilityHandler.cpp"
#include "application/drx_Application.cpp"
#include "buttons/drx_ArrowButton.cpp"
#include "buttons/drx_Button.cpp"
#include "buttons/drx_DrawableButton.cpp"
#include "buttons/drx_HyperlinkButton.cpp"
#include "buttons/drx_ImageButton.cpp"
#include "buttons/drx_ShapeButton.cpp"
#include "buttons/drx_TextButton.cpp"
#include "buttons/drx_ToggleButton.cpp"
#include "buttons/drx_ToolbarButton.cpp"
#include "commands/drx_ApplicationCommandInfo.cpp"
#include "commands/drx_ApplicationCommandManager.cpp"
#include "commands/drx_ApplicationCommandTarget.cpp"
#include "commands/drx_KeyPressMappingSet.cpp"
#include "components/drx_Component.cpp"
#include "components/drx_ComponentListener.cpp"
#include "components/drx_FocusTraverser.cpp"
#include "components/drx_ModalComponentManager.cpp"
#include "desktop/drx_Desktop.cpp"
#include "desktop/drx_Displays.cpp"
#include "detail/drx_AccessibilityHelpers.cpp"
#include "drawables/drx_Drawable.cpp"
#include "drawables/drx_DrawableComposite.cpp"
#include "drawables/drx_DrawableImage.cpp"
#include "drawables/drx_DrawablePath.cpp"
#include "drawables/drx_DrawableRectangle.cpp"
#include "drawables/drx_DrawableShape.cpp"
#include "drawables/drx_DrawableText.cpp"
#include "drawables/drx_SVGParser.cpp"
#include "filebrowser/drx_ContentSharer.cpp"
#include "filebrowser/drx_DirectoryContentsDisplayComponent.cpp"
#include "filebrowser/drx_DirectoryContentsList.cpp"
#include "filebrowser/drx_FileBrowserComponent.cpp"
#include "filebrowser/drx_FileChooser.cpp"
#include "filebrowser/drx_FileChooserDialogBox.cpp"
#include "filebrowser/drx_FileListComponent.cpp"
#include "filebrowser/drx_FileSearchPathListComponent.cpp"
#include "filebrowser/drx_FileTreeComponent.cpp"
#include "filebrowser/drx_FilenameComponent.cpp"
#include "filebrowser/drx_ImagePreviewComponent.cpp"
#include "keyboard/drx_CaretComponent.cpp"
#include "keyboard/drx_KeyListener.cpp"
#include "keyboard/drx_KeyPress.cpp"
#include "keyboard/drx_KeyboardFocusTraverser.cpp"
#include "keyboard/drx_ModifierKeys.cpp"
#include "layout/drx_ComponentAnimator.cpp"
#include "layout/drx_ComponentBoundsConstrainer.cpp"
#include "layout/drx_BorderedComponentBoundsConstrainer.cpp"
#include "layout/drx_ComponentBuilder.cpp"
#include "layout/drx_ComponentMovementWatcher.cpp"
#include "layout/drx_ConcertinaPanel.cpp"
#include "layout/drx_FlexBox.cpp"
#include "layout/drx_Grid.cpp"
#include "layout/drx_GridItem.cpp"
#include "layout/drx_GroupComponent.cpp"
#include "layout/drx_MultiDocumentPanel.cpp"
#include "layout/drx_ResizableBorderComponent.cpp"
#include "layout/drx_ResizableCornerComponent.cpp"
#include "layout/drx_ResizableEdgeComponent.cpp"
#include "layout/drx_ScrollBar.cpp"
#include "layout/drx_SidePanel.cpp"
#include "layout/drx_StretchableLayoutManager.cpp"
#include "layout/drx_StretchableLayoutResizerBar.cpp"
#include "layout/drx_StretchableObjectResizer.cpp"
#include "layout/drx_TabbedButtonBar.cpp"
#include "layout/drx_TabbedComponent.cpp"
#include "layout/drx_Viewport.cpp"
#include "lookandfeel/drx_LookAndFeel.cpp"
#include "lookandfeel/drx_LookAndFeel_V1.cpp"
#include "lookandfeel/drx_LookAndFeel_V2.cpp"
#include "lookandfeel/drx_LookAndFeel_V3.cpp"
#include "lookandfeel/drx_LookAndFeel_V4.cpp"
#include "menus/drx_BurgerMenuComponent.cpp"
#include "menus/drx_MenuBarComponent.cpp"
#include "menus/drx_MenuBarModel.cpp"
#include "menus/drx_PopupMenu.cpp"
#include "misc/drx_BubbleComponent.cpp"
#include "misc/drx_DropShadower.cpp"
#include "misc/drx_FocusOutline.cpp"
#include "mouse/drx_ComponentDragger.cpp"
#include "mouse/drx_DragAndDropContainer.cpp"
#include "mouse/drx_MouseEvent.cpp"
#include "mouse/drx_MouseInactivityDetector.cpp"
#include "mouse/drx_MouseInputSource.cpp"
#include "mouse/drx_MouseListener.cpp"
#include "native/drx_ScopedDPIAwarenessDisabler.cpp"
#include "positioning/drx_MarkerList.cpp"
#include "positioning/drx_RelativeCoordinate.cpp"
#include "positioning/drx_RelativeCoordinatePositioner.cpp"
#include "positioning/drx_RelativeParallelogram.cpp"
#include "positioning/drx_RelativePoint.cpp"
#include "positioning/drx_RelativePointPath.cpp"
#include "positioning/drx_RelativeRectangle.cpp"
#include "properties/drx_BooleanPropertyComponent.cpp"
#include "properties/drx_ButtonPropertyComponent.cpp"
#include "properties/drx_ChoicePropertyComponent.cpp"
#include "properties/drx_MultiChoicePropertyComponent.cpp"
#include "properties/drx_PropertyComponent.cpp"
#include "properties/drx_PropertyPanel.cpp"
#include "properties/drx_SliderPropertyComponent.cpp"
#include "properties/drx_TextPropertyComponent.cpp"
#include "widgets/drx_ComboBox.cpp"
#include "widgets/drx_ImageComponent.cpp"
#include "widgets/drx_Label.cpp"
#include "widgets/drx_ListBox.cpp"
#include "widgets/drx_ProgressBar.cpp"
#include "widgets/drx_Slider.cpp"
#include "widgets/drx_TableHeaderComponent.cpp"
#include "widgets/drx_TableListBox.cpp"
#include "widgets/drx_TextEditorModel.cpp"
#include "widgets/drx_TextEditor.cpp"
#include "widgets/drx_Toolbar.cpp"
#include "widgets/drx_ToolbarItemComponent.cpp"
#include "widgets/drx_ToolbarItemPalette.cpp"
#include "widgets/drx_TreeView.cpp"
#include "windows/drx_NativeMessageBox.cpp"
#include "windows/drx_AlertWindow.cpp"
#include "windows/drx_CallOutBox.cpp"
#include "windows/drx_ComponentPeer.cpp"
#include "windows/drx_DialogWindow.cpp"
#include "windows/drx_DocumentWindow.cpp"
#include "windows/drx_MessageBoxOptions.cpp"
#include "windows/drx_ResizableWindow.cpp"
#include "windows/drx_ScopedMessageBox.cpp"
#include "windows/drx_ThreadWithProgressWindow.cpp"
#include "windows/drx_TooltipWindow.cpp"
#include "windows/drx_TopLevelWindow.cpp"
#include "windows/drx_VBlankAttachment.cpp"
#include "windows/drx_NativeScaleFactorNotifier.cpp"
