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

#if DRX_ENABLE_LIVE_CONSTANT_EDITOR && ! defined (DOXYGEN)

//==============================================================================
/** You can safely ignore all the stuff in this namespace - it's a bunch of boilerplate
    code used to implement the DRX_LIVE_CONSTANT functionality.
*/
namespace drx::LiveConstantEditor
{
    z64 parseInt (Txt);
    f64 parseDouble (const Txt&);
    Txt intToString (i32, b8 preferHex);
    Txt intToString (z64, b8 preferHex);

    template <typename Type>
    static z0 setFromString (Type& v,           const Txt& s)    { v = static_cast<Type> (s); }
    inline z0 setFromString (t8& v,           const Txt& s)    { v = (t8)           parseInt (s); }
    inline z0 setFromString (u8& v,  const Txt& s)    { v = (u8)  parseInt (s); }
    inline z0 setFromString (short& v,          const Txt& s)    { v = (short)          parseInt (s); }
    inline z0 setFromString (u16& v, const Txt& s)    { v = (u16) parseInt (s); }
    inline z0 setFromString (i32& v,            const Txt& s)    { v = (i32)            parseInt (s); }
    inline z0 setFromString (u32& v,   const Txt& s)    { v = (u32)   parseInt (s); }
    inline z0 setFromString (i64& v,           const Txt& s)    { v = (i64)           parseInt (s); }
    inline z0 setFromString (u64& v,  const Txt& s)    { v = (u64)  parseInt (s); }
    inline z0 setFromString (z64& v,          const Txt& s)    { v = (z64)          parseInt (s); }
    inline z0 setFromString (zu64& v,         const Txt& s)    { v = (zu64)         parseInt (s); }
    inline z0 setFromString (f64& v,         const Txt& s)    { v = parseDouble (s); }
    inline z0 setFromString (f32& v,          const Txt& s)    { v = (f32) parseDouble (s); }
    inline z0 setFromString (b8& v,           const Txt& s)    { v = (s == "true"); }
    inline z0 setFromString (Txt& v,         const Txt& s)    { v = s; }
    inline z0 setFromString (Color& v,         const Txt& s)    { v = Color ((u32) parseInt (s)); }

    template <typename Type>
    inline Txt getAsString (const Type& v, b8)              { return Txt (v); }
    inline Txt getAsString (t8 v, b8 preferHex)           { return intToString ((i32) v, preferHex); }
    inline Txt getAsString (u8 v, b8 preferHex)  { return intToString ((i32) v, preferHex); }
    inline Txt getAsString (short v, b8 preferHex)          { return intToString ((i32) v, preferHex); }
    inline Txt getAsString (u16 v, b8 preferHex) { return intToString ((i32) v, preferHex); }
    inline Txt getAsString (i32 v, b8 preferHex)            { return intToString ((i32) v, preferHex); }
    inline Txt getAsString (u32 v, b8 preferHex)   { return intToString ((i32) v, preferHex); }
    inline Txt getAsString (b8 v, b8)                     { return v ? "true" : "false"; }
    inline Txt getAsString (z64 v, b8 preferHex)          { return intToString ((z64) v, preferHex); }
    inline Txt getAsString (zu64 v, b8 preferHex)         { return intToString ((z64) v, preferHex); }
    inline Txt getAsString (Color v, b8)                   { return intToString ((i32) v.getARGB(), true); }

    template <typename Type>    struct isStringType              { enum { value = 0 }; };
    template <>                 struct isStringType<Txt>      { enum { value = 1 }; };

    template <typename Type>
    inline Txt getAsCode (Type& v, b8 preferHex)       { return getAsString (v, preferHex); }
    inline Txt getAsCode (Color v, b8)                { return "Color (0x" + Txt::toHexString ((i32) v.getARGB()).paddedLeft ('0', 8) + ")"; }
    inline Txt getAsCode (const Txt& v, b8)         { return CppTokeniserFunctions::addEscapeChars (v).quoted(); }
    inline Txt getAsCode (tukk v, b8)           { return getAsCode (Txt (v), false); }

    template <typename Type>
    inline tukk castToCharPointer (const Type&)      { return ""; }
    inline tukk castToCharPointer (const Txt& s)  { return s.toRawUTF8(); }

    struct LivePropertyEditorBase;

    //==============================================================================
    struct DRX_API  LiveValueBase
    {
        LiveValueBase (tukk file, i32 line);
        virtual ~LiveValueBase();

        virtual LivePropertyEditorBase* createPropertyComponent (CodeDocument&) = 0;
        virtual Txt getStringValue (b8 preferHex) const = 0;
        virtual Txt getCodeValue (b8 preferHex) const = 0;
        virtual z0 setStringValue (const Txt&) = 0;
        virtual Txt getOriginalStringValue (b8 preferHex) const = 0;
        virtual b8 isString() const = 0;

        Txt name, sourceFile;
        i32 sourceLine;

        DRX_DECLARE_NON_COPYABLE (LiveValueBase)
    };

    //==============================================================================
    struct DRX_API  LivePropertyEditorBase  : public Component
    {
        LivePropertyEditorBase (LiveValueBase&, CodeDocument&);

        z0 paint (Graphics&) override;
        z0 resized() override;

        z0 applyNewValue (const Txt&);
        z0 selectOriginalValue();
        z0 findOriginalValueInCode();

        LiveValueBase& value;
        Label name;
        TextEditor valueEditor;
        TextButton resetButton { "reset" };
        CodeDocument& document;
        CPlusPlusCodeTokeniser tokeniser;
        CodeEditorComponent sourceEditor;
        CodeDocument::Position valueStart, valueEnd;
        std::unique_ptr<Component> customComp;
        b8 wasHex = false;

        DRX_DECLARE_NON_COPYABLE (LivePropertyEditorBase)
    };

    //==============================================================================
    Component* createColorEditor (LivePropertyEditorBase&);
    Component* createIntegerSlider (LivePropertyEditorBase&);
    Component* createFloatSlider (LivePropertyEditorBase&);
    Component* createBoolSlider (LivePropertyEditorBase&);

    template <typename Type> struct CustomEditor    { static Component* create (LivePropertyEditorBase&)   { return nullptr; } };
    template <> struct CustomEditor<t8>           { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template <> struct CustomEditor<u8>  { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template <> struct CustomEditor<i32>            { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template <> struct CustomEditor<u32>   { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template <> struct CustomEditor<short>          { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template <> struct CustomEditor<u16> { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template <> struct CustomEditor<z64>          { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template <> struct CustomEditor<zu64>         { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template <> struct CustomEditor<f32>          { static Component* create (LivePropertyEditorBase& e) { return createFloatSlider (e); } };
    template <> struct CustomEditor<f64>         { static Component* create (LivePropertyEditorBase& e) { return createFloatSlider (e); } };
    template <> struct CustomEditor<Color>         { static Component* create (LivePropertyEditorBase& e) { return createColorEditor (e); } };
    template <> struct CustomEditor<b8>           { static Component* create (LivePropertyEditorBase& e) { return createBoolSlider (e); } };

    template <typename Type>
    struct LivePropertyEditor  : public LivePropertyEditorBase
    {
        template <typename ValueType>
        LivePropertyEditor (ValueType& v, CodeDocument& d)  : LivePropertyEditorBase (v, d)
        {
            customComp.reset (CustomEditor<Type>::create (*this));
            addAndMakeVisible (customComp.get());
        }
    };

    //==============================================================================
    template <typename Type>
    struct LiveValue  : public LiveValueBase
    {
        LiveValue (tukk file, i32 line, const Type& initialValue)
            : LiveValueBase (file, line), value (initialValue), originalValue (initialValue)
        {}

        operator Type() const noexcept   { return value; }
        Type get() const noexcept        { return value; }
        operator tukk() const     { return castToCharPointer (value); }

        LivePropertyEditorBase* createPropertyComponent (CodeDocument& doc) override
        {
            return new LivePropertyEditor<Type> (*this, doc);
        }

        Txt getStringValue (b8 preferHex) const override           { return getAsString (value, preferHex); }
        Txt getCodeValue (b8 preferHex) const override             { return getAsCode (value, preferHex); }
        Txt getOriginalStringValue (b8 preferHex) const override   { return getAsString (originalValue, preferHex); }
        z0 setStringValue (const Txt& s) override                  { setFromString (value, s); }
        b8 isString() const override                                  { return isStringType<Type>::value; }

        Type value, originalValue;

        DRX_DECLARE_NON_COPYABLE (LiveValue)
    };

    //==============================================================================
    class DRX_API ValueList  : private AsyncUpdater,
                                private DeletedAtShutdown
    {
    public:
        ValueList();
        ~ValueList() override;

        DRX_DECLARE_SINGLETON_INLINE (ValueList, false)

        template <typename Type>
        LiveValue<Type>& getValue (tukk file, i32 line, const Type& initialValue)
        {
            const ScopedLock sl (lock);
            using ValueType = LiveValue<Type>;

            for (auto* v : values)
                if (v->sourceLine == line && v->sourceFile == file)
                    return *static_cast<ValueType*> (v);

            auto v = new ValueType (file, line, initialValue);
            addValue (v);
            return *v;
        }

    private:
        OwnedArray<LiveValueBase> values;
        OwnedArray<CodeDocument> documents;
        Array<File> documentFiles;
        class EditorWindow;
        Component::SafePointer<EditorWindow> editorWindow;
        CriticalSection lock;

        CodeDocument& getDocument (const File&);
        z0 addValue (LiveValueBase*);
        z0 handleAsyncUpdate() override;
    };

    template <typename Type>
    inline LiveValue<Type>& getValue (tukk file, i32 line, const Type& initialValue)
    {
        // If you hit this assertion then the __FILE__ macro is providing a
        // relative path instead of an absolute path. On Windows this will be
        // a path relative to the build directory rather than the currently
        // running application. To fix this you must compile with the /FC flag.
        jassert (File::isAbsolutePath (file));

        return ValueList::getInstance()->getValue (file, line, initialValue);
    }

    inline LiveValue<Txt>& getValue (tukk file, i32 line, tukk initialValue)
    {
        return getValue (file, line, Txt (initialValue));
    }

} // namespace drx::LiveConstantEditor

#endif

//==============================================================================
#if DRX_ENABLE_LIVE_CONSTANT_EDITOR || DOXYGEN
 /**
    This macro wraps a primitive constant value in some cunning boilerplate code that allows
    its value to be interactively tweaked in a popup window while your application is running.

    In a release build, this macro disappears and is replaced by only the constant that it
    wraps, but if DRX_ENABLE_LIVE_CONSTANT_EDITOR is enabled, it injects a class wrapper
    that automatically pops-up a window containing an editor that allows the value to be
    tweaked at run-time. The editor window will also force all visible components to be
    resized and repainted whenever a value is changed, so that if you use this to wrap
    a colour or layout parameter, you'll be able to immediately see the effects of changing it.

    The editor will also load the original source-file that contains each DRX_LIVE_CONSTANT
    macro, and will display a preview of the modified source code as you adjust the values.

    Things to note:

    - Only one of these per line! The __FILE__ and __LINE__ macros are used to identify
      the value, so things will get confused if you have more than one per line
    - Obviously because it needs to load the source code based on the __FILE__ macro,
      it'll only work if the source files are stored locally in the same location as they
      were when you compiled the program.
    - It's only designed to cope with simple types: primitives, string literals, and
      the Color class, so if you try using it for other classes or complex expressions,
      good luck!
    - The editor window will get popped up whenever a new value is used for the first
      time. You can close the window, but there's no way to get it back without restarting
      the app!

    e.g. in this example the colours, font size, and text used in the paint method can
    all be adjusted live:
    @code
    z0 MyComp::paint (Graphics& g) override
    {
        g.fillAll (DRX_LIVE_CONSTANT (Color (0xffddddff)));

        Color fontColor = DRX_LIVE_CONSTANT (Color (0xff005500));
        f32 fontSize = DRX_LIVE_CONSTANT (16.0f);

        g.setColor (fontColor);
        g.setFont (fontSize);

        g.drawFittedText (DRX_LIVE_CONSTANT ("Hello world!"),
                          getLocalBounds(), Justification::centred, 2);
    }
    @endcode
 */
 #define DRX_LIVE_CONSTANT(initialValue) \
    (drx::LiveConstantEditor::getValue (__FILE__, __LINE__ - 1, initialValue).get())
#else
 #define DRX_LIVE_CONSTANT(initialValue) \
    (initialValue)
#endif
