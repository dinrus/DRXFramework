/*
  ==============================================================================

    This file contains the basic framework code for a DRX plugin processor.

  ==============================================================================
*/

%%filter_headers%%

//==============================================================================
%%filter_class_name%%::%%filter_class_name%%()
#ifndef DrxPlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! DrxPlugin_IsMidiEffect
                      #if ! DrxPlugin_IsSynth
                       .withInput  ("Input",  drx::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", drx::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

%%filter_class_name%%::~%%filter_class_name%%()
{
}

//==============================================================================
const drx::Txt %%filter_class_name%%::getName() const
{
    return DrxPlugin_Name;
}

b8 %%filter_class_name%%::acceptsMidi() const
{
   #if DrxPlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

b8 %%filter_class_name%%::producesMidi() const
{
   #if DrxPlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

b8 %%filter_class_name%%::isMidiEffect() const
{
   #if DrxPlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

f64 %%filter_class_name%%::getTailLengthSeconds() const
{
    return 0.0;
}

i32 %%filter_class_name%%::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

i32 %%filter_class_name%%::getCurrentProgram()
{
    return 0;
}

z0 %%filter_class_name%%::setCurrentProgram (i32 index)
{
}

const drx::Txt %%filter_class_name%%::getProgramName (i32 index)
{
    return {};
}

z0 %%filter_class_name%%::changeProgramName (i32 index, const drx::Txt& newName)
{
}

//==============================================================================
z0 %%filter_class_name%%::prepareToPlay (f64 sampleRate, i32 samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

z0 %%filter_class_name%%::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef DrxPlugin_PreferredChannelConfigurations
b8 %%filter_class_name%%::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if DrxPlugin_IsMidiEffect
    drx::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != drx::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != drx::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! DrxPlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

z0 %%filter_class_name%%::processBlock (drx::AudioBuffer<f32>& buffer, drx::MidiBuffer& midiMessages)
{
    drx::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (i32 channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
b8 %%filter_class_name%%::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

drx::AudioProcessorEditor* %%filter_class_name%%::createEditor()
{
    return new %%editor_class_name%% (*this);
}

//==============================================================================
z0 %%filter_class_name%%::getStateInformation (drx::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

z0 %%filter_class_name%%::setStateInformation (ukk data, i32 sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
drx::AudioProcessor* DRX_CALLTYPE createPluginFilter()
{
    return new %%filter_class_name%%();
}
