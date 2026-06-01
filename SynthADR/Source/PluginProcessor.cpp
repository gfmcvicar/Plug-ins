/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Signals.h"  // ADD THIS LINE - explicit include to ensure classes are visible

//==============================================================================
SynthADRAudioProcessor::SynthADRAudioProcessor()
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
}

SynthADRAudioProcessor::~SynthADRAudioProcessor()
{
}

//==============================================================================
const juce::String SynthADRAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynthADRAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SynthADRAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SynthADRAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SynthADRAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynthADRAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SynthADRAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SynthADRAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SynthADRAudioProcessor::getProgramName (int index)
{
    return {};
}

void SynthADRAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SynthADRAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.clearVoices();
    synth.clearSounds();
    //Add Sound
    synth.addSound(new MySignal());

    // Add Voices (Polyphony)
    for (int i = 0; i < 3; i++) synth.addVoice(new SineVoice());
    for (int i = 0; i < 3; i++) synth.addVoice(new SawVoice());
    for (int i = 0; i < 3; i++) synth.addVoice(new SquareVoice());

    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SynthADRAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SynthADRAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SynthADRAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear(); 
    auto numSamples = buffer.getNumSamples();
    auto currentRate = getSampleRate();
    auto osc=parameters.getRawParameterValue("osc")->load();
    auto a = parameters.getRawParameterValue("attack")->load();
    auto d = parameters.getRawParameterValue("decay")->load();
    auto s = parameters.getRawParameterValue("sustain")->load();
    auto r = parameters.getRawParameterValue("release")->load();

        // Update every voice with these values
        for (int i = 0; i < synth.getNumVoices(); ++i) {
            if (auto* v = dynamic_cast<VoiceBase*>(synth.getVoice(i))) {
                v->updateAdsr(a, d, s, r,currentRate, (int)osc);
            }
        }
    
    keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
     
}

//==============================================================================
bool SynthADRAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SynthADRAudioProcessor::createEditor()
{
    return new SynthADRAudioProcessorEditor (*this);
}

//==============================================================================
void SynthADRAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SynthADRAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthADRAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout SynthADRAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    //Swithc oscillators
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {"osc", 1},"Oscillator",juce::StringArray {"Sine", "SawTooth", "Square"},0));
    //ADSR Parameters
    //Attack
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"attack", 1},"Attack", 0.0f, 10.0f, 0.5f));
    //decay
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"decay", 1},"Decay", 0.0f, 10.0f, 0.5f));
    //sustain
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"sustain", 1},"Sustain", 0.0f, 10.0f, 0.5f));
    //realese
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"release", 1},"Release", 0.0f, 10.0f, 0.5f));
    
    return {parameters.begin(),parameters.end()};
    
}
