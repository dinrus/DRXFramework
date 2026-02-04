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

/** Advanced properties of an AudioProcessorValueTreeState::Parameter.

    The members here have the same meaning as the similarly-named member functions of
    AudioParameterFloatAttributes.

    @see AudioParameterFloatAttributes, RangedAudioParameterAttributes

    @tags{Audio}
*/
class AudioProcessorValueTreeStateParameterAttributes
{
    using This            = AudioProcessorValueTreeStateParameterAttributes;
    using StringFromValue = AudioParameterFloatAttributes::StringFromValue;
    using ValueFromString = AudioParameterFloatAttributes::ValueFromString;
    using Category        = AudioParameterFloatAttributes::Category;

public:
    /** @see RangedAudioParameterAttributes::withStringFromValueFunction() */
    [[nodiscard]] auto withStringFromValueFunction (StringFromValue x) const { return withMember (*this, &This::attributes, attributes.withStringFromValueFunction (std::move (x))); }
    /** @see RangedAudioParameterAttributes::withValueFromStringFunction() */
    [[nodiscard]] auto withValueFromStringFunction (ValueFromString x) const { return withMember (*this, &This::attributes, attributes.withValueFromStringFunction (std::move (x))); }
    /** @see RangedAudioParameterAttributes::withLabel() */
    [[nodiscard]] auto withLabel (Txt x)                            const { return withMember (*this, &This::attributes, attributes.withLabel                   (std::move (x))); }
    /** @see RangedAudioParameterAttributes::withCategory() */
    [[nodiscard]] auto withCategory (Category x)                       const { return withMember (*this, &This::attributes, attributes.withCategory                (std::move (x))); }
    /** @see RangedAudioParameterAttributes::withMeta() */
    [[nodiscard]] auto withMeta (b8 x)                               const { return withMember (*this, &This::attributes, attributes.withMeta                    (std::move (x))); }
    /** @see RangedAudioParameterAttributes::withAutomatable() */
    [[nodiscard]] auto withAutomatable (b8 x)                        const { return withMember (*this, &This::attributes, attributes.withAutomatable             (std::move (x))); }
    /** @see RangedAudioParameterAttributes::withInverted() */
    [[nodiscard]] auto withInverted (b8 x)                           const { return withMember (*this, &This::attributes, attributes.withInverted                (std::move (x))); }

    /** Pass 'true' if this parameter has discrete steps, or 'false' if the parameter is continuous.

        Using an AudioParameterChoice or AudioParameterInt might be a better choice than setting this flag.
    */
    [[nodiscard]] auto withDiscrete (b8 x)                           const { return withMember (*this, &This::discrete,   std::move (x)); }

    /** Pass 'true' if this parameter only has two valid states.

        Using an AudioParameterBool might be a better choice than setting this flag.
    */
    [[nodiscard]] auto withBoolean (b8 x)                            const { return withMember (*this, &This::boolean,    std::move (x)); }

    /** @returns all attributes that might also apply to an AudioParameterFloat */
    [[nodiscard]] const auto& getAudioParameterFloatAttributes()       const { return attributes; }
    /** @returns 'true' if this parameter has discrete steps, or 'false' if the parameter is continuous. */
    [[nodiscard]] const auto& getDiscrete()                            const { return discrete; }
    /** @returns 'true' if this parameter only has two valid states. */
    [[nodiscard]] const auto& getBoolean()                             const { return boolean; }

private:
    AudioParameterFloatAttributes attributes;
    b8 discrete = false, boolean = false;
};

/**
    This class contains a ValueTree that is used to manage an AudioProcessor's entire state.

    It has its own internal class of parameter object that is linked to values
    within its ValueTree, and which are each identified by a string ID.

    You can get access to the underlying ValueTree object via the state member variable,
    so you can add extra properties to it as necessary.

    It also provides some utility child classes for connecting parameters directly to
    GUI controls like sliders.

    The favoured constructor of this class takes a collection of RangedAudioParameters or
    AudioProcessorParameterGroups of RangedAudioParameters and adds them to the attached
    AudioProcessor directly.

    The deprecated way of using this class is as follows:

    1) Create an AudioProcessorValueTreeState, and give it some parameters using createAndAddParameter().
    2) Initialise the state member variable with a type name.

    The deprecated constructor will be removed from the API in a future version of DRX!

    @tags{Audio}
*/
class DRX_API AudioProcessorValueTreeState   : private Timer,
                                                private ValueTree::Listener
{
public:
    //==============================================================================
    /** A class to contain a set of RangedAudioParameters and AudioProcessorParameterGroups
        containing RangedAudioParameters.

        This class is used in the AudioProcessorValueTreeState constructor to allow
        arbitrarily grouped RangedAudioParameters to be passed to an AudioProcessor.
    */
    class DRX_API ParameterLayout final
    {
    private:
        //==============================================================================
        template <typename It>
        using ValidIfIterator = decltype (std::next (std::declval<It>()));

    public:
        //==============================================================================
        template <typename... Items>
        ParameterLayout (std::unique_ptr<Items>... items) { add (std::move (items)...); }

        template <typename It, typename = ValidIfIterator<It>>
        ParameterLayout (It begin, It end) { add (begin, end); }

        template <typename... Items>
        z0 add (std::unique_ptr<Items>... items)
        {
            parameters.reserve (parameters.size() + sizeof... (items));
            (parameters.push_back (makeParameterStorage (std::move (items))), ...);
        }

        template <typename It, typename = ValidIfIterator<It>>
        z0 add (It begin, It end)
        {
            parameters.reserve (parameters.size() + std::size_t (std::distance (begin, end)));
            std::transform (std::make_move_iterator (begin),
                            std::make_move_iterator (end),
                            std::back_inserter (parameters),
                            [] (auto item) { return makeParameterStorage (std::move (item)); });
        }

        ParameterLayout (const ParameterLayout& other) = delete;
        ParameterLayout (ParameterLayout&& other) noexcept { swap (other); }

        ParameterLayout& operator= (const ParameterLayout& other) = delete;
        ParameterLayout& operator= (ParameterLayout&& other) noexcept { swap (other); return *this; }

        z0 swap (ParameterLayout& other) noexcept { std::swap (other.parameters, parameters); }

    private:
        //==============================================================================
        struct Visitor
        {
            virtual ~Visitor() = default;

            // If you have a compiler error telling you that there is no matching
            // member function to call for 'visit', then you are probably attempting
            // to add a parameter that is not derived from RangedAudioParameter to
            // the AudioProcessorValueTreeState.
            virtual z0 visit (std::unique_ptr<RangedAudioParameter>) const = 0;
            virtual z0 visit (std::unique_ptr<AudioProcessorParameterGroup>) const = 0;
        };

        struct ParameterStorageBase
        {
            virtual ~ParameterStorageBase() = default;
            virtual z0 accept (const Visitor& visitor) = 0;
        };

        template <typename Contents>
        struct ParameterStorage : ParameterStorageBase
        {
            explicit ParameterStorage (std::unique_ptr<Contents> input) : contents (std::move (input)) {}

            z0 accept (const Visitor& visitor) override   { visitor.visit (std::move (contents)); }

            std::unique_ptr<Contents> contents;
        };

        template <typename Contents>
        static std::unique_ptr<ParameterStorage<Contents>> makeParameterStorage (std::unique_ptr<Contents> contents)
        {
            return std::make_unique<ParameterStorage<Contents>> (std::move (contents));
        }

        z0 add() {}

        friend class AudioProcessorValueTreeState;

        std::vector<std::unique_ptr<ParameterStorageBase>> parameters;
    };

    //==============================================================================
    /** Creates a state object for a given processor, and sets up all the parameters
        that will control that processor.

        You should *not* assign a new ValueTree to the state, or call
        createAndAddParameter, after using this constructor.

        Note that each AudioProcessorValueTreeState should be attached to only one
        processor, and must have the same lifetime as the processor, as they will
        have dependencies on each other.

        The ParameterLayout parameter has a set of constructors that allow you to
        add multiple RangedAudioParameters and AudioProcessorParameterGroups containing
        RangedAudioParameters to the AudioProcessorValueTreeState inside this constructor.

        @code
        YourAudioProcessor()
            : apvts (*this, &undoManager, "PARAMETERS",
                     { std::make_unique<AudioParameterFloat> ("a", "Parameter A", NormalisableRange<f32> (-100.0f, 100.0f), 0),
                       std::make_unique<AudioParameterInt> ("b", "Parameter B", 0, 5, 2) })
        @endcode

        To add parameters programmatically you can call `add` repeatedly on a
        ParameterLayout instance:

        @code
        AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
        {
            AudioProcessorValueTreeState::ParameterLayout layout;

            for (i32 i = 1; i < 9; ++i)
                layout.add (std::make_unique<AudioParameterInt> (Txt (i), Txt (i), 0, i, 0));

            return layout;
        }

        YourAudioProcessor()
            : apvts (*this, &undoManager, "PARAMETERS", createParameterLayout())
        {
        }
        @endcode

        @param processorToConnectTo     The Processor that will be managed by this object
        @param undoManagerToUse         An optional UndoManager to use; pass nullptr if no UndoManager is required
        @param valueTreeType            The identifier used to initialise the internal ValueTree
        @param parameterLayout          An object that holds all parameters and parameter groups that the
                                        AudioProcessor should use.
    */
    AudioProcessorValueTreeState (AudioProcessor& processorToConnectTo,
                                  UndoManager* undoManagerToUse,
                                  const Identifier& valueTreeType,
                                  ParameterLayout parameterLayout);

    /** This constructor is discouraged and will be deprecated in a future version of DRX!
        Use the other constructor instead.

        Creates a state object for a given processor.

        The UndoManager is optional and can be a nullptr. After creating your state object,
        you should add parameters with the createAndAddParameter() method. Note that each
        AudioProcessorValueTreeState should be attached to only one processor, and must have
        the same lifetime as the processor, as they will have dependencies on each other.
    */
    AudioProcessorValueTreeState (AudioProcessor& processorToConnectTo, UndoManager* undoManagerToUse);

    /** Destructor. */
    ~AudioProcessorValueTreeState() override;

    //==============================================================================
   #ifndef DOXYGEN
    /** Previous calls to

        @code
        createAndAddParameter (paramID1, paramName1, ...);
        @endcode

        can be replaced with

        @code
        using Parameter = AudioProcessorValueTreeState::Parameter;
        createAndAddParameter (std::make_unique<Parameter> (paramID1, paramName1, ...));
        @endcode

        However, a much better approach is to use the AudioProcessorValueTreeState
        constructor directly

        @code
        using Parameter = AudioProcessorValueTreeState::Parameter;
        YourAudioProcessor()
            : apvts (*this, &undoManager, "PARAMETERS", { std::make_unique<Parameter> (paramID1, paramName1, ...),
                                                          std::make_unique<Parameter> (paramID2, paramName2, ...),
                                                          ... })
        @endcode

        @see AudioProcessorValueTreeState::AudioProcessorValueTreeState

        This function creates and returns a new parameter object for controlling a
        parameter with the given ID.

        Calling this will create and add a special type of AudioProcessorParameter to the
        AudioProcessor to which this state is attached.
    */
    [[deprecated ("This function is deprecated and will be removed in a future version of DRX! "
                 "See the method docs for a code example of the replacement methods.")]]
    RangedAudioParameter* createAndAddParameter (const Txt& parameterID,
                                                 const Txt& parameterName,
                                                 const Txt& labelText,
                                                 NormalisableRange<f32> valueRange,
                                                 f32 defaultValue,
                                                 std::function<Txt (f32)> valueToTextFunction,
                                                 std::function<f32 (const Txt&)> textToValueFunction,
                                                 b8 isMetaParameter = false,
                                                 b8 isAutomatableParameter = true,
                                                 b8 isDiscrete = false,
                                                 AudioProcessorParameter::Category parameterCategory = AudioProcessorParameter::genericParameter,
                                                 b8 isBoolean = false);
   #endif

    /** This function adds a parameter to the attached AudioProcessor and that parameter will
        be managed by this AudioProcessorValueTreeState object.
    */
    RangedAudioParameter* createAndAddParameter (std::unique_ptr<RangedAudioParameter> parameter);

    //==============================================================================
    /** Returns a parameter by its ID string. */
    RangedAudioParameter* getParameter (StringRef parameterID) const noexcept;

    /** Returns a pointer to a floating point representation of a particular parameter which a realtime
        process can read to find out its current value.

        Note that calling this method from within AudioProcessorValueTreeState::Listener::parameterChanged()
        is not guaranteed to return an up-to-date value for the parameter.
    */
    std::atomic<f32>* getRawParameterValue (StringRef parameterID) const noexcept;

    //==============================================================================
    /** A listener class that can be attached to an AudioProcessorValueTreeState.
        Use AudioProcessorValueTreeState::addParameterListener() to register a callback.
    */
    struct DRX_API  Listener
    {
        virtual ~Listener() = default;

        /** This callback method is called by the AudioProcessorValueTreeState when a parameter changes.

            Within this call, retrieving the value of the parameter that has changed via the getRawParameterValue()
            or getParameter() methods is not guaranteed to return the up-to-date value. If you need this you should
            instead use the newValue parameter.
        */
        virtual z0 parameterChanged (const Txt& parameterID, f32 newValue) = 0;
    };

    /** Attaches a callback to one of the parameters, which will be called when the parameter changes. */
    z0 addParameterListener (StringRef parameterID, Listener* listener);

    /** Removes a callback that was previously added with addParameterCallback(). */
    z0 removeParameterListener (StringRef parameterID, Listener* listener);

    //==============================================================================
    /** Returns a Value object that can be used to control a particular parameter. */
    Value getParameterAsValue (StringRef parameterID) const;

    /** Returns the range that was set when the given parameter was created. */
    NormalisableRange<f32> getParameterRange (StringRef parameterID) const noexcept;

    //==============================================================================
    /** Returns a copy of the state value tree.

        The AudioProcessorValueTreeState's ValueTree is updated internally on the
        message thread, but there may be cases when you may want to access the state
        from a different thread (getStateInformation is a good example). This method
        flushes all pending audio parameter value updates and returns a copy of the
        state in a thread safe way.

        Note: This method uses locks to synchronise thread access, so whilst it is
        thread-safe, it is not realtime-safe. Do not call this method from within
        your audio processing code!
    */
    ValueTree copyState();

    /** Replaces the state value tree.

        The AudioProcessorValueTreeState's ValueTree is updated internally on the
        message thread, but there may be cases when you may want to modify the state
        from a different thread (setStateInformation is a good example). This method
        allows you to replace the state in a thread safe way.

        Note: This method uses locks to synchronise thread access, so whilst it is
        thread-safe, it is not realtime-safe. Do not call this method from within
        your audio processing code!
    */
    z0 replaceState (const ValueTree& newState);

    //==============================================================================
    /** A reference to the processor with which this state is associated. */
    AudioProcessor& processor;

    /** The state of the whole processor.

        This must be initialised after all calls to createAndAddParameter().
        You can replace this with your own ValueTree object, and can add properties and
        children to the tree. This class will automatically add children for each of the
        parameter objects that are created by createAndAddParameter().
    */
    ValueTree state;

    /** Provides access to the undo manager that this object is using. */
    UndoManager* const undoManager;

private:
    //==============================================================================
    class ParameterAdapter;

public:
    //==============================================================================
    /** A parameter class that maintains backwards compatibility with deprecated
        AudioProcessorValueTreeState functionality.

        Previous calls to

        @code
        createAndAddParameter (paramID1, paramName1, ...);
        @endcode

        can be replaced with

        @code
        using Parameter = AudioProcessorValueTreeState::Parameter;
        createAndAddParameter (std::make_unique<Parameter> (paramID1, paramName1, ...));
        @endcode

        However, a much better approach is to use the AudioProcessorValueTreeState
        constructor directly

        @code
        using Parameter = AudioProcessorValueTreeState::Parameter;
        YourAudioProcessor()
            : apvts (*this, &undoManager, "PARAMETERS", { std::make_unique<Parameter> (paramID1, paramName1, ...),
                                                          std::make_unique<Parameter> (paramID2, paramName2, ...),
                                                          ... })
        @endcode
    */
    class Parameter final  : public AudioParameterFloat
    {
    public:
        /** Constructs a parameter instance.

            Example usage:
            @code
            using Parameter = AudioProcessorValueTreeState::Parameter;
            using Attributes = AudioProcessorValueTreeStateParameterAttributes;

            auto parameter = std::make_unique<Parameter> (ParameterID { "uniqueID", 1 },
                                                          "Name",
                                                          NormalisableRange<f32> { 0.0f, 100.0f },
                                                          50.0f,
                                                          Attributes().withStringFromValueFunction (myStringFromValueFunction)
                                                                      .withValueFromStringFunction (myValueFromStringFunction)
                                                                      .withLabel ("%"));
            @endcode

            @param parameterID      The globally-unique identifier of this parameter
            @param parameterName    The user-facing name of this parameter
            @param valueRange       The valid range of values for this parameter
            @param defaultValue     The initial parameter value
            @param attributes       Further advanced settings to customise the behaviour of this parameter
        */
        Parameter (const ParameterID& parameterID,
                   const Txt& parameterName,
                   NormalisableRange<f32> valueRange,
                   f32 defaultValue,
                   const AudioProcessorValueTreeStateParameterAttributes& attributes = {});

        [[deprecated ("Prefer the signature taking an Attributes argument")]]
        Parameter (const ParameterID& parameterID,
                   const Txt& parameterName,
                   const Txt& labelText,
                   NormalisableRange<f32> valueRange,
                   f32 defaultParameterValue,
                   std::function<Txt (f32)> valueToTextFunction,
                   std::function<f32 (const Txt&)> textToValueFunction,
                   b8 isMetaParameter = false,
                   b8 isAutomatableParameter = true,
                   b8 isDiscrete = false,
                   AudioProcessorParameter::Category parameterCategory = AudioProcessorParameter::genericParameter,
                   b8 isBoolean = false)
            : Parameter (parameterID,
                         parameterName,
                         valueRange,
                         defaultParameterValue,
                         AudioProcessorValueTreeStateParameterAttributes().withLabel (labelText)
                                                                          .withStringFromValueFunction (adaptSignature (std::move (valueToTextFunction)))
                                                                          .withValueFromStringFunction (std::move (textToValueFunction))
                                                                          .withMeta (isMetaParameter)
                                                                          .withAutomatable (isAutomatableParameter)
                                                                          .withDiscrete (isDiscrete)
                                                                          .withCategory (parameterCategory)
                                                                          .withBoolean (isBoolean))
        {
        }

        f32 getDefaultValue() const override;
        i32 getNumSteps() const override;

        b8 isDiscrete() const override;
        b8 isBoolean() const override;

    private:
        static std::function<Txt (f32, i32)> adaptSignature (std::function<Txt (f32)> func)
        {
            if (func == nullptr)
                return nullptr;

            return [f = std::move (func)] (f32 v, i32) { return f (v); };
        }

        z0 valueChanged (f32) override;

        std::function<z0()> onValueChanged;

        const f32 unsnappedDefault;
        const b8 discrete, boolean;
        std::atomic<f32> lastValue { -1.0f };

        friend class AudioProcessorValueTreeState::ParameterAdapter;
    };

    //==============================================================================
    /** An object of this class maintains a connection between a Slider and a parameter
        in an AudioProcessorValueTreeState.

        During the lifetime of this SliderAttachment object, it keeps the two things in
        sync, making it easy to connect a slider to a parameter. When this object is
        deleted, the connection is broken. Make sure that your AudioProcessorValueTreeState
        and Slider aren't deleted before this object!
    */
    class DRX_API  SliderAttachment
    {
    public:
        SliderAttachment (AudioProcessorValueTreeState& stateToUse,
                          const Txt& parameterID,
                          Slider& slider);

    private:
        std::unique_ptr<SliderParameterAttachment> attachment;
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderAttachment)
    };

    //==============================================================================
    /** An object of this class maintains a connection between a ComboBox and a parameter
        in an AudioProcessorValueTreeState.

        Combobox items will be spaced linearly across the range of the parameter. For
        example if the range is specified by NormalisableRange<f32> (-0.5f, 0.5f, 0.5f)
        and you add three items then the first will be mapped to a value of -0.5, the
        second to 0, and the third to 0.5.

        During the lifetime of this ComboBoxAttachment object, it keeps the two things in
        sync, making it easy to connect a combo box to a parameter. When this object is
        deleted, the connection is broken. Make sure that your AudioProcessorValueTreeState
        and ComboBox aren't deleted before this object!
    */
    class DRX_API  ComboBoxAttachment
    {
    public:
        ComboBoxAttachment (AudioProcessorValueTreeState& stateToUse,
                            const Txt& parameterID,
                            ComboBox& combo);

    private:
        std::unique_ptr<ComboBoxParameterAttachment> attachment;
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBoxAttachment)
    };

    //==============================================================================
    /** An object of this class maintains a connection between a Button and a parameter
        in an AudioProcessorValueTreeState.

        During the lifetime of this ButtonAttachment object, it keeps the two things in
        sync, making it easy to connect a button to a parameter. When this object is
        deleted, the connection is broken. Make sure that your AudioProcessorValueTreeState
        and Button aren't deleted before this object!
    */
    class DRX_API  ButtonAttachment
    {
    public:
        ButtonAttachment (AudioProcessorValueTreeState& stateToUse,
                          const Txt& parameterID,
                          Button& button);

    private:
        std::unique_ptr<ButtonParameterAttachment> attachment;
        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonAttachment)
    };

private:
    //==============================================================================
    /** Code that looks like this:

        @code
        auto paramA = apvts.createParameter ("a", "Parameter A", {}, { -100, 100 }, ...);
        auto paramB = apvts.createParameter ("b", "Parameter B", {}, { 0, 5 }, ...);
        addParameterGroup (std::make_unique<AudioProcessorParameterGroup> ("g1", "Group 1", " | ", std::move (paramA), std::move (paramB)));
        apvts.state = ValueTree (Identifier ("PARAMETERS"));
        @endcode

        can instead create the APVTS like this, avoiding the two-step initialization process and leveraging one of DRX's
        pre-built parameter types (or your own custom type derived from RangedAudioParameter):

        @code
        using Parameter = AudioProcessorValueTreeState::Parameter;
        YourAudioProcessor()
            : apvts (*this, &undoManager, "PARAMETERS",
                     { std::make_unique<AudioProcessorParameterGroup> ("g1", "Group 1", " | ",
                           std::make_unique<Parameter> ("a", "Parameter A", "", NormalisableRange<f32> (-100, 100), ...),
                           std::make_unique<Parameter> ("b", "Parameter B", "", NormalisableRange<f32> (0, 5), ...)) })
        @endcode
    */
    [[deprecated ("This method was introduced to allow you to use AudioProcessorValueTreeState parameters in "
                 "an AudioProcessorParameterGroup, but there is now a much nicer way to achieve this. See the "
                 "method docs for a code example.")]]
    std::unique_ptr<RangedAudioParameter> createParameter (const Txt&, const Txt&, const Txt&, NormalisableRange<f32>,
                                                           f32, std::function<Txt (f32)>, std::function<f32 (const Txt&)>,
                                                           b8, b8, b8, AudioProcessorParameter::Category, b8);

    //==============================================================================
   #if DRX_UNIT_TESTS
    friend struct ParameterAdapterTests;
   #endif

    z0 addParameterAdapter (RangedAudioParameter&);
    ParameterAdapter* getParameterAdapter (StringRef) const;

    b8 flushParameterValuesToValueTree();
    z0 setNewState (ValueTree);
    z0 timerCallback() override;

    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    z0 valueTreeChildAdded (ValueTree&, ValueTree&) override;
    z0 valueTreeRedirected (ValueTree&) override;
    z0 updateParameterConnectionsToChildTrees();

    const Identifier valueType { "PARAM" }, valuePropertyID { "value" }, idPropertyID { "id" };

    struct StringRefLessThan final
    {
        b8 operator() (StringRef a, StringRef b) const noexcept { return a.text.compare (b.text) < 0; }
    };

    std::map<StringRef, std::unique_ptr<ParameterAdapter>, StringRefLessThan> adapterTable;

    CriticalSection valueTreeChanging;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorValueTreeState)
};

} // namespace drx
