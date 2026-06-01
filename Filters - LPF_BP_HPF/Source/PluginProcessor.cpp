/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//This plugin implements a four-band graphic equaliser. Each band has independent gain control. The filtered signals are summed to produce the final output.

//==============================================================================
LPF_BP_HPFAudioProcessor::LPF_BP_HPFAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),parameters(*this, nullptr, "parameters", createParameterLayout())
#endif
{
    formatManager.registerBasicFormats(); //which formats we're gonna be using
        DBG("Constructor complete");
}

LPF_BP_HPFAudioProcessor::~LPF_BP_HPFAudioProcessor()
{
}

//==============================================================================
const juce::String LPF_BP_HPFAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LPF_BP_HPFAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LPF_BP_HPFAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LPF_BP_HPFAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LPF_BP_HPFAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LPF_BP_HPFAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LPF_BP_HPFAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LPF_BP_HPFAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LPF_BP_HPFAudioProcessor::getProgramName (int index)
{
    return {};
}

void LPF_BP_HPFAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LPF_BP_HPFAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare DSP objects using host playback configuration
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    lowBand.filter.prepare(spec);
    lowBand.gain.prepare(spec);

    midLowBand.filter.prepare(spec);
    midLowBand.gain.prepare(spec);

    midHighBand.filter.prepare(spec);
    midHighBand.gain.prepare(spec);

    highBand.filter.prepare(spec);
    highBand.gain.prepare(spec);
    
}

void LPF_BP_HPFAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LPF_BP_HPFAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LPF_BP_HPFAudioProcessor::loadFile(const juce::File& file)
{
    // Loads an audio file and connects it to the AudioTransportSource so that it can be streamed during processing
    
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        DBG("File Loaded Successfully: " << file.getFileName());

        // First, create the reader source and assign to readerSource
        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

        // Then set the transport's source safely
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

        transportSource.start();
    }
    else
    {
        DBG("Failed to create reader for: " << file.getFileName());
    }
}
void LPF_BP_HPFAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    // Clear buffer before summing the EQ bands
    for (int ch = 0; ch < numOutputChannels; ++ch)
        buffer.clear(ch, 0, numSamples);

    // Retrieve play and stop button states from the parameter system
    
    auto play = parameters.getRawParameterValue("play");
    auto stop = parameters.getRawParameterValue("stop");

    // Handle stop button
    if (stop->load() > 0.5f)
    {
        transportSource.stop();
        transportSource.setPosition(0.0);
        buffer.clear();
        return;
    }

    // If not playing, just return
    if (play->load() < 0.5f)
    {
        buffer.clear();
        return;
    }

    // ------------------------
    // Prepare input audio
    // ------------------------
    juce::AudioBuffer<float> inputBuffer;
    inputBuffer.makeCopyOf(buffer);

    if (readerSource != nullptr)
    {
        if (!transportSource.isPlaying())
            transportSource.start();

        juce::AudioSourceChannelInfo loadedFile(inputBuffer);
        transportSource.getNextAudioBlock(loadedFile);
    }
    else
    {
        // Generate white noise only if "play" is on
        for (int ch = 0; ch < numOutputChannels; ++ch)
        {
            auto* channelData = inputBuffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                channelData[i] = (2.0f * rand() / (float)RAND_MAX - 1.0f) * 0.2f; //generates white noise - code taken from HelloWorld plugin
        }
    }

    // ------------------------
    // Parallel EQ processing
    // ------------------------
   // The input signal is copied and processed through four independent filter bands. Each band processes a copy of the original input signal, applies gain, and then the result is summed into the output buffer.
    
    auto sampleRate = getSampleRate();

    auto lowFreq      = parameters.getRawParameterValue("lowFrequency")->load();
    auto lowGainDb    = parameters.getRawParameterValue("lowGain")->load();
    auto midLowFreq   = parameters.getRawParameterValue("midlowFrequency")->load();
    auto midLowGainDb = parameters.getRawParameterValue("midlowGain")->load();
    auto midLowQ      = parameters.getRawParameterValue("midlowQ")->load();
    auto midHighFreq   = parameters.getRawParameterValue("midhighFrequency")->load();
    auto midHighGainDb = parameters.getRawParameterValue("midhighGain")->load();
    auto midHighQ      = parameters.getRawParameterValue("midhighQ")->load();
    auto highFreq      = parameters.getRawParameterValue("highFrequency")->load();
    auto highGainDb    = parameters.getRawParameterValue("highGain")->load();

    juce::AudioBuffer<float> tempBuffer;
    tempBuffer.setSize(numOutputChannels, numSamples);
    tempBuffer.clear();

    juce::dsp::AudioBlock<float> inputBlock(inputBuffer);
    juce::dsp::AudioBlock<float> tempBlock(tempBuffer);
    
    // Helper function to process a band and sum it into the output
    auto processBand = [&](Band& band)
    {
        // Copy input signal to temporary buffer
        tempBuffer.copyFrom(0, 0, inputBuffer, 0, 0, numSamples);
        tempBuffer.copyFrom(1, 0, inputBuffer, 1, 0, numSamples);

        juce::dsp::ProcessContextReplacing<float> context(tempBlock);

        // Process filter then gain
        band.filter.process(context);
        band.gain.process(context);

        // Sum processed band into output
        buffer.addFrom(0, 0, tempBuffer, 0, 0, numSamples);
        buffer.addFrom(1, 0, tempBuffer, 1, 0, numSamples);
    };
    
    // Structure representing each EQ band. Each band contains an IIR filter and a gain stage.

    // --- LOW ---
    *lowBand.filter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lowFreq);
    lowBand.gain.setGainLinear(std::pow(10.0f, lowGainDb / 20.0f)); // Converts gain from decibels to linear amplitude

    processBand(lowBand);

    // --- MID LOW ---
    *midLowBand.filter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, midLowFreq, midLowQ);
    midLowBand.gain.setGainLinear(std::pow(10.0f, midLowGainDb / 20.0f));

    processBand(midLowBand);

    // --- MID HIGH ---
    *midHighBand.filter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, midHighFreq, midHighQ);
    midHighBand.gain.setGainLinear(std::pow(10.0f, midHighGainDb / 20.0f));

    processBand(midHighBand);

    // --- HIGH ---
    *highBand.filter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, highFreq);
    highBand.gain.setGainLinear(std::pow(10.0f, highGainDb / 20.0f));

    processBand(highBand);
}
//==============================================================================
bool LPF_BP_HPFAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LPF_BP_HPFAudioProcessor::createEditor()
{
    return new LPF_BP_HPFAudioProcessorEditor (*this);
}

//==============================================================================
void LPF_BP_HPFAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void LPF_BP_HPFAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LPF_BP_HPFAudioProcessor();
}
juce::AudioProcessorValueTreeState::ParameterLayout LPF_BP_HPFAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    //Buttons
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"file", 1},"File", 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"play", 1},"Play", 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"stop", 1},"Stop", 0));
    //Sliders
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowGain", 1},"lowGain", -10.0f, 10.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowQ", 1},"LowQ", 0.01f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"lowFrequency", 1},"LowFrequency", 0.01f, 400.0f, 200.0f));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midlowGain", 1},"midlowGain", -10.0f, 10.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midlowQ", 1},"midlowQ", 0.01f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midlowFrequency", 1},"midlowFrequency", 200.0f, 1000.0f, 600.0f));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midhighGain", 1},"midhighGain", -10.0f, 10.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midhighQ", 1},"midhighQ", 0.01f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"midhighFrequency", 1},"midhighFrequency", 1000.0f, 6000.0f, 3500.0f));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highGain", 1},"HighGain", -10.0f, 10.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highQ", 1},"HighQ", 0.01f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"highFrequency", 1},"highFrequency", 6000.0f, 20000.0f, 13000.0f));
    
    // Note that LowQ and HighQ parameters are currently unused in the DSP section, but they are included for consistency with the other bands.
    
    return {parameters.begin(),parameters.end()};
    
}
