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
/** This class is used to hold a few look and feel base classes which are associated
    with classes that may not be present because they're from modules other than
    drx_gui_basics.

    @tags{GUI}
*/
struct DRX_API  ExtraLookAndFeelBaseClasses
{
    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  LassoComponentMethods
    {
        virtual ~LassoComponentMethods() = default;

        virtual z0 drawLasso (Graphics&, Component& lassoComp) = 0;
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  KeyMappingEditorComponentMethods
    {
        virtual ~KeyMappingEditorComponentMethods() = default;

        virtual z0 drawKeymapChangeButton (Graphics&, i32 width, i32 height, Button&, const Txt& keyDescription) = 0;
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct DRX_API  AudioDeviceSelectorComponentMethods
    {
        virtual ~AudioDeviceSelectorComponentMethods() = default;

        virtual z0 drawLevelMeter (Graphics&, i32 width, i32 height, f32 level) = 0;
    };
};


//==============================================================================
/**
    LookAndFeel objects define the appearance of all the DRX widgets, and subclasses
    can be used to apply different 'skins' to the application.

    This class is an abstract base-class - for actual look-and-feels that you can
    instantiate, see LookAndFeel_V1, LookAndFeel_V2 and LookAndFeel_V3.

    @see LookAndFeel_V1, LookAndFeel_V2, LookAndFeel_V3

    @tags{GUI}
*/
class DRX_API  LookAndFeel   : public ScrollBar::LookAndFeelMethods,
                                public Button::LookAndFeelMethods,
                                public ImageButton::LookAndFeelMethods,
                                public TextEditor::LookAndFeelMethods,
                                public FileBrowserComponent::LookAndFeelMethods,
                                public TreeView::LookAndFeelMethods,
                                public BubbleComponent::LookAndFeelMethods,
                                public AlertWindow::LookAndFeelMethods,
                                public PopupMenu::LookAndFeelMethods,
                                public ComboBox::LookAndFeelMethods,
                                public Label::LookAndFeelMethods,
                                public Slider::LookAndFeelMethods,
                                public ResizableWindow::LookAndFeelMethods,
                                public DocumentWindow::LookAndFeelMethods,
                                public TooltipWindow::LookAndFeelMethods,
                                public TabbedButtonBar::LookAndFeelMethods,
                                public PropertyComponent::LookAndFeelMethods,
                                public FilenameComponent::LookAndFeelMethods,
                                public GroupComponent::LookAndFeelMethods,
                                public TableHeaderComponent::LookAndFeelMethods,
                                public CallOutBox::LookAndFeelMethods,
                                public Toolbar::LookAndFeelMethods,
                                public ConcertinaPanel::LookAndFeelMethods,
                                public ProgressBar::LookAndFeelMethods,
                                public StretchableLayoutResizerBar::LookAndFeelMethods,
                                public ExtraLookAndFeelBaseClasses::KeyMappingEditorComponentMethods,
                                public ExtraLookAndFeelBaseClasses::AudioDeviceSelectorComponentMethods,
                                public ExtraLookAndFeelBaseClasses::LassoComponentMethods,
                                public SidePanel::LookAndFeelMethods
{
public:
    //==============================================================================
    /** Creates the default DRX look and feel. */
    LookAndFeel();

    /** Destructor. */
    ~LookAndFeel() override;

    //==============================================================================
    /** Returns the current default look-and-feel for a component to use when it
        hasn't got one explicitly set.

        @see setDefaultLookAndFeel
    */
    static LookAndFeel& getDefaultLookAndFeel() noexcept;

    /** Changes the default look-and-feel.

        @param newDefaultLookAndFeel    the new look-and-feel object to use - if this is
                                        set to null, it will revert to using the default one. The
                                        object passed-in must be deleted by the caller when
                                        it's no longer needed.
        @see getDefaultLookAndFeel
    */
    static z0 setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel) noexcept;

    //==============================================================================
    /** Looks for a colour that has been registered with the given colour ID number.

        If a colour has been set for this ID number using setColor(), then it is
        returned. If none has been set, it will just return Colors::black.

        The colour IDs for various purposes are stored as enums in the components that
        they are relevant to - for an example, see Slider::ColorIds,
        Label::ColorIds, TextEditor::ColorIds, TreeView::ColorIds, etc.

        If you're looking up a colour for use in drawing a component, it's usually
        best not to call this directly, but to use the Component::findColor() method
        instead. That will first check whether a suitable colour has been registered
        directly with the component, and will fall-back on calling the component's
        LookAndFeel's findColor() method if none is found.

        @see setColor, Component::findColor, Component::setColor
    */
    Color findColor (i32 colourId) const noexcept;

    /** Registers a colour to be used for a particular purpose.

        For more details, see the comments for findColor().

        @see findColor, Component::findColor, Component::setColor
    */
    z0 setColor (i32 colourId, Color colour) noexcept;

    /** Возвращает true, если the specified colour ID has been explicitly set using the
        setColor() method.
    */
    b8 isColorSpecified (i32 colourId) const noexcept;

    //==============================================================================
    /** Returns the typeface that should be used for a given font.

        The default implementation just does what you'd expect it to, but you can override
        this if you want to intercept fonts and use your own custom typeface object.

        @see setDefaultTypeface
    */
    virtual Typeface::Ptr getTypefaceForFont (const Font&);

    /** Widgets can call this to find out the kind of metrics they should use when creating their
        own fonts.

        The default implementation returns the portable metrics kind, but you can override this if
        you want to use the legacy metrics kind instead, to avoid rendering changes in existing
        projects. Switching between metrics kinds may cause text to render at a different size, so you
        should check that text in your app still renders at an appropriate size, and potentially adjust
        font sizes where necessary after overriding this function.
    */
    virtual TypefaceMetricsKind getDefaultMetricsKind() const { return TypefaceMetricsKind::portable; }

    /** Returns a copy of the FontOptions with the LookAndFeel's default metrics kind set. */
    FontOptions withDefaultMetrics (FontOptions opt) const { return opt.withMetricsKind (getDefaultMetricsKind()); }

    /** Allows you to supply a default typeface that will be returned as the default
        sans-serif font.

        Instead of a typeface object, you can specify a typeface by name using the
        setDefaultSansSerifTypefaceName() method.

        You can perform more complex typeface substitutions by overloading
        getTypefaceForFont() but this lets you easily set a global typeface.
    */
    z0 setDefaultSansSerifTypeface (Typeface::Ptr newDefaultTypeface);

    /** Allows you to change the default sans-serif font.

        If you need to supply your own Typeface object for any of the default fonts, rather
        than just supplying the name (e.g. if you want to use an embedded font), then
        you can instead call setDefaultSansSerifTypeface() with an object to use.
    */
    z0 setDefaultSansSerifTypefaceName (const Txt& newName);

    //==============================================================================
    /** Sets whether native alert windows (if available) or standard DRX AlertWindows
        drawn with AlertWindow::LookAndFeelMethods will be used.

        @see isUsingNativeAlertWindows
    */
    z0 setUsingNativeAlertWindows (b8 shouldUseNativeAlerts);

    /** Возвращает true, если native alert windows will be used (if available).

        The default setting for this is false.

        @see setUsingNativeAlertWindows
    */
    b8 isUsingNativeAlertWindows();

    //==============================================================================
    /** Draws a small image that spins to indicate that something's happening.

        This method should use the current time to animate itself, so just keep
        repainting it every so often.
    */
    virtual z0 drawSpinningWaitAnimation (Graphics&, const Color& colour,
                                            i32 x, i32 y, i32 w, i32 h) = 0;

    /** Returns a tick shape for use in yes/no boxes, etc. */
    virtual Path getTickShape (f32 height) = 0;

    /** Returns a cross shape for use in yes/no boxes, etc. */
    virtual Path getCrossShape (f32 height) = 0;

    /** Creates a drop-shadower for a given component, if required.

        @see DropShadower
    */
    virtual std::unique_ptr<DropShadower> createDropShadowerForComponent (Component&) = 0;

    /** Creates a focus outline for a given component, if required.

        @see FocusOutline
    */
    virtual std::unique_ptr<FocusOutline> createFocusOutlineForComponent (Component&) = 0;

    //==============================================================================
    /** Override this to get the chance to swap a component's mouse cursor for a
        customised one.

        @see MouseCursor
    */
    virtual MouseCursor getMouseCursorFor (Component&);

    /** Creates a new graphics context object. */
    virtual std::unique_ptr<LowLevelGraphicsContext> createGraphicsContext (const Image& imageToRenderOn,
                                                                            Point<i32> origin,
                                                                            const RectangleList<i32>& initialClip);

    /** Plays the system's default 'beep' noise, to alert the user about something
        very important. This is only supported on some platforms.
    */
    virtual z0 playAlertSound();

private:
    //==============================================================================
    struct ColorSetting
    {
        i32 colourID;
        Color colour;

        b8 operator<  (const ColorSetting& other) const noexcept  { return colourID <  other.colourID; }
        b8 operator== (const ColorSetting& other) const noexcept  { return colourID == other.colourID; }
    };

    SortedSet<ColorSetting> colours;
    Txt defaultSans, defaultSerif, defaultFixed;
    Typeface::Ptr defaultTypeface;
    b8 useNativeAlertWindows = false;

    DRX_DECLARE_WEAK_REFERENCEABLE (LookAndFeel)
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel)
};

} // namespace drx
