class %%class_name%%  : public drx::AudioProcessor
{
public:
    //==============================================================================
    %%class_name%%()
        : AudioProcessor (BusesProperties().withInput  ("Input",  drx::AudioChannelSet::stereo())
                                           .withOutput ("Output", drx::AudioChannelSet::stereo()))
    {
    }

    ~%%class_name%%() override
    {
    }

    //==============================================================================
    z0 prepareToPlay (f64, i32) override
    {
        // Use this method as the place to do any pre-playback
        // initialisation that you need..
    }

    z0 releaseResources() override
    {
        // When playback stops, you can use this as an opportunity to free up any
        // spare memory, etc.
    }

    z0 processBlock (drx::AudioBuffer<f32>& buffer, drx::MidiBuffer&) override
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
    drx::AudioProcessorEditor* createEditor() override          { return nullptr; }
    b8 hasEditor() const override                              { return false;   }

    //==============================================================================
    const drx::Txt getName() const override                  { return "%%name%%"; }
    b8 acceptsMidi() const override                            { return false; }
    b8 producesMidi() const override                           { return false; }
    f64 getTailLengthSeconds() const override                 { return 0; }

    //==============================================================================
    i32 getNumPrograms() override                                { return 1; }
    i32 getCurrentProgram() override                             { return 0; }
    z0 setCurrentProgram (i32) override                        {}
    const drx::Txt getProgramName (i32) override             { return {}; }
    z0 changeProgramName (i32, const drx::Txt&) override   {}

    //==============================================================================
    z0 getStateInformation (drx::MemoryBlock& destData) override
    {
        // You should use this method to store your parameters in the memory block.
        // You could do that either as raw data, or use the XML or ValueTree classes
        // as intermediaries to make it easy to save and load complex data.
    }

    z0 setStateInformation (ukk data, i32 sizeInBytes) override
    {
        // You should use this method to restore your parameters from this memory block,
        // whose contents will have been created by the getStateInformation() call.
    }

    //==============================================================================
    b8 isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // This is the place where you check if the layout is supported.
        // In this template code we only support mono or stereo.
        if (layouts.getMainOutputChannelSet() != drx::AudioChannelSet::mono()
            && layouts.getMainOutputChannelSet() != drx::AudioChannelSet::stereo())
            return false;

        // This checks if the input layout matches the output layout
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;

        return true;
    }

private:
    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%class_name%%)
};
