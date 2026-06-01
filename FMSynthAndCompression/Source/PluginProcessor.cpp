/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FMSynthCompressionAudioProcessor::FMSynthCompressionAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
      parameters(*this, nullptr, "parameters", createParameterLayout())
{

}

FMSynthCompressionAudioProcessor::~FMSynthCompressionAudioProcessor() {}

//==============================================================================
const juce::String FMSynthCompressionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FMSynthCompressionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FMSynthCompressionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FMSynthCompressionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FMSynthCompressionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FMSynthCompressionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FMSynthCompressionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FMSynthCompressionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FMSynthCompressionAudioProcessor::getProgramName (int index)
{
    return {};
}

void FMSynthCompressionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FMSynthCompressionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = getTotalNumOutputChannels();

        compressor.prepare(spec);
    }

void FMSynthCompressionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FMSynthCompressionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FMSynthCompressionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    modulationDepthParam = parameters.getRawParameterValue("ModulationDepth");
    modulatorFrequencyParam = parameters.getRawParameterValue("ModulatorFrequency");
    carrierFrequencyParam = parameters.getRawParameterValue("CarrierFrequency");
    
    {
        juce::ScopedNoDenormals noDenormals;

        auto numOutputChannels = getTotalNumOutputChannels();
        auto numSamples = buffer.getNumSamples();
        float sampleRate = (float) getSampleRate();

        // FM parameters
        
        float modulationDepth = modulationDepthParam->load();
        float modulatorFrequency = modulatorFrequencyParam->load();
        float carrierFrequency = carrierFrequencyParam->load();

        // Compressor parameters (you need to add these properly later)
        float threshold = -20.0f;
        float ratio = 4.0f;
        float attack = parameters.getRawParameterValue("AttackTime")->load();
        float release = parameters.getRawParameterValue("ReleaseTime")->load();
        float makeUpGain = 6.0f;

        // Set compressor
        compressor.setThreshold(threshold);
        compressor.setRatio(ratio);
        compressor.setAttack(attack);
        compressor.setRelease(release);

        float modulatorPhaseIncrement = modulatorFrequency / sampleRate;
        float carrierPhaseIncrement = carrierFrequency / sampleRate;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Modulator signal

            float modulator = std::sin(juce::MathConstants<float>:: pi * modulatorPhase);
            
            // FM carrier (THIS is your equation)
            
            float carrier = std::sin(juce::MathConstants<float>:: twoPi * (carrierPhase + modulationDepth * modulator));

            float output = carrier;

            for (int channel = 0; channel < numOutputChannels; ++channel)
                buffer.setSample(channel, sample, output);

            // Increment phases
            modulatorPhase += modulatorFrequency / sampleRate;
            while (modulatorPhase >= 1.0) modulatorPhase -= 1.0;
            carrierPhase += carrierFrequency / sampleRate;
            while (carrierPhase >= 1.0) carrierPhase -= 1.0;
        }

        // Apply compressor
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        compressor.process(context);

        // Apply makeup gain
        buffer.applyGain(juce::Decibels::decibelsToGain(makeUpGain));
    }
}

//==============================================================================
bool FMSynthCompressionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FMSynthCompressionAudioProcessor::createEditor()
{
    //return new juce::GenericAudioProcessorEditor(this);
    return new FMSynthCompressionAudioProcessorEditor (*this);
}

//==============================================================================
void FMSynthCompressionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FMSynthCompressionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FMSynthCompressionAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout
FMSynthCompressionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ModulationDepth", "Modulation Depth", 0.0f, 100.0f, 200.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ModulatorFrequency", "Modulator Frequency", 10.0f, 40.0f, 100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CarrierFrequency", "Carrier Frequency", 50.0f, 2000.0f, 800.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "AttackTime", "Attack Time", 0.1f, 80.0f, 15.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ReleaseTime", "Release Time", 1.0f, 1000.0f, 100.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "Threshold", "Threshold", -60.0f, 0.0f, -20.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "Ratio", "Ratio", 1.0f, 20.0f, 4.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "MakeUpGain", "Make Up Gain", 0.0f, 24.0f, 6.0f));

    return { params.begin(), params.end() };
}
