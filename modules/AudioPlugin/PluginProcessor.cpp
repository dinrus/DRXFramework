#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! DrxPlugin_IsMidiEffect
                      #if ! DrxPlugin_IsSynth
                       .withInput  ("Input",  drx::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", drx::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const drx::Txt AudioPluginAudioProcessor::getName() const
{
    return DrxPlugin_Name;
}

b8 AudioPluginAudioProcessor::acceptsMidi() const
{
   #if DrxPlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

b8 AudioPluginAudioProcessor::producesMidi() const
{
   #if DrxPlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

b8 AudioPluginAudioProcessor::isMidiEffect() const
{
   #if DrxPlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

f64 AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

i32 AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

i32 AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

z0 AudioPluginAudioProcessor::setCurrentProgram (i32 index)
{
    drx::ignoreUnused (index);
}

const drx::Txt AudioPluginAudioProcessor::getProgramName (i32 index)
{
    drx::ignoreUnused (index);
    return {};
}

z0 AudioPluginAudioProcessor::changeProgramName (i32 index, const drx::Txt& newName)
{
    drx::ignoreUnused (index, newName);
}

//==============================================================================
z0 AudioPluginAudioProcessor::prepareToPlay (f64 sampleRate, i32 samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    drx::ignoreUnused (sampleRate, samplesPerBlock);
}

z0 AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

b8 AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

z0 AudioPluginAudioProcessor::processBlock (drx::AudioBuffer<f32>& buffer,
                                              drx::MidiBuffer& midiMessages)
{
    drx::ignoreUnused (midiMessages);

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
        drx::ignoreUnused (channelData);
        // ..do something to the data...
    }
}

//==============================================================================
b8 AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

drx::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
z0 AudioPluginAudioProcessor::getStateInformation (drx::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    drx::ignoreUnused (destData);
}

z0 AudioPluginAudioProcessor::setStateInformation (ukk data, i32 sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    drx::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
drx::AudioProcessor* DRX_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
