/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "parameters", createParameterLayout())
#endif
{ //Decide if yo wanna use the tree state or the geeric audio processor.
    //juce::AudioParameterFloat (Param ID, user ID, min, max, starting point)
    //addParameter(delayTimeParam = new juce::AudioParameterFloat("delayTime", "Delay time", 0.0f, 2.0f, 0.0f));
  
}

DelayAudioProcessor::~DelayAudioProcessor()
{
}

//==============================================================================
const juce::String DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // We need to create the circular buffer here.
    //Initialize everything before going to the processBlock.
      //auto delayTime = delayTimeParam->get();
    auto delayTime = parameters.getRawParameterValue("delayTime")->load();
    //Set size for the buffer ang lenght of the buffer.
    delayBufferLength = (int)(2.0 * sampleRate);
    delayBuffer.setSize(2, delayBufferLength);
    //clear the buffer.
      delayBuffer.clear();
    //Try to code the position of the Read pointer... you have to get the position of the write pointer,use the % to wrap the buffer.
    
    delayReadPosition = (int)(delayWritePosition - (delayTime * getSampleRate()) + delayBufferLength) % delayBufferLength;
    
}

void DelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
void DelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    float sampleRate = (float)getSampleRate();
//Depending on generic audio processor or value tree stae get the values of the user.
    auto delayTime = parameters.getRawParameterValue("delayTime")->load();
    auto feedback = parameters.getRawParameterValue("feedback")->load();
    auto mix = parameters.getRawParameterValue("mix")->load();
    

    // Update read position (same as in the prepare to play).
    
    delayReadPosition = (int)(delayWritePosition - (delayTime * getSampleRate()) + delayBufferLength) % delayBufferLength;

    for (int i = 0; i < numSamples; ++i)
        {
            for (int j = 0; j < numInputChannels; ++j)
            {
                // 1. GET the mic input from the buffer
                const float micIn = buffer.getReadPointer(j)[i];

                //Create a pointer for the  delay buffer.
                float* delayData = delayBuffer.getWritePointer(j);

                // 2. READ the old sound from the circular buffer
                float delayedSignal = delayData[delayReadPosition];

                // 3. WRITE the mic input + feedback into the buffer
                // This is what creates the "echo" of your voice
                //pointer(position) = micIn + (delayedSignal * feedback);
                delayData[delayWritePosition] = micIn + (delayedSignal * feedback);

                // 4. MIX the original mic sound (Dry) with the echo (Wet)
                float out = (1.0f - mix) * micIn + (mix * delayedSignal);
            

                // 5. SET the output to the speakers
                buffer.getWritePointer(j)[i] = out;
            }

            // 6. Advance the pointers
            if (++delayReadPosition >= delayBufferLength) delayReadPosition = 0;
            if (++delayWritePosition >= delayBufferLength) delayWritePosition = 0;
            
            
           //Write the write position update.
        }
      // In case we have more outputs than inputs, clear any output channels that didn't contain input data
      for (int i = numInputChannels; i < numOutputChannels; ++i)
      {
        buffer.clear(i, 0, buffer.getNumSamples());
      }
}

//==============================================================================
bool DelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayAudioProcessor::createEditor()
{
   // return new juce::GenericAudioProcessorEditor(this); //Not using the editor
    return new DelayAudioProcessorEditor (*this); //Connecting the value parameter state
}

//==============================================================================
void DelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayAudioProcessor();
}


//comment this function if you're not using the editor

juce::AudioProcessorValueTreeState::ParameterLayout DelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"delayTime", 1},"DelayTime", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"feedback", 1},"feedback", 0.0f, 1.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix", 1},"mix", 0.0f, 1.0f, 0.5f));

 //Push back the other parameters
    
    return {parameters.begin(),parameters.end()};
    
}
