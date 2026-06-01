/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KarplusStrongAudioProcessor::KarplusStrongAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
 
    addParameter(pluckParam =
        new juce::AudioParameterBool("pluck", "Pluck", 0));

    addParameter(decayParam =
        new juce::AudioParameterFloat("decay", "Decay",
                                      0.8f, 0.999f, 0.99f));

    addParameter(delayParam =
        new juce::AudioParameterFloat("delay", "Delay Time",
                                      0.0f, 0.02f, 0.005f));

    addParameter(widthParam =
        new juce::AudioParameterFloat("width", "Noise Width",
                                      0.0f, 0.02f, 0.01f));
    
    addParameter(cutoffParam =
        new juce::AudioParameterFloat("cutoff",
                                      "Lowpass Cutoff",
                                      100.0f,
                                      10000.0f,
                                      3000.0f));
    
    addParameter(burstTypeParam =
        new juce::AudioParameterChoice("burst",
                                       "Burst Type",
                                       juce::StringArray { "Noise",
                                                           "Sine",
                                                           "Triangle",
                                                           "Square",
                                                           "Saw" },
                                       0));

    
}

KarplusStrongAudioProcessor::~KarplusStrongAudioProcessor()
{
}

//==============================================================================
const juce::String KarplusStrongAudioProcessor::getName() const
{
    return JucePlugin_Name;
    
}

bool KarplusStrongAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KarplusStrongAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KarplusStrongAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KarplusStrongAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KarplusStrongAudioProcessor::getNumPrograms()
{
    return 1;
             
}

int KarplusStrongAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KarplusStrongAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KarplusStrongAudioProcessor::getProgramName (int index)
{
    return {};
}

void KarplusStrongAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KarplusStrongAudioProcessor::prepareToPlay(double sampleRate, int)
{

    delayBuffer.setSize(getTotalNumOutputChannels(), 0.02f * sampleRate + 1);
    delayBuffer.clear();

    delayWritePosition = 0;

        filterState.resize(getTotalNumOutputChannels());
        std::fill(filterState.begin(), filterState.end(), 0.0f);
    
    float frequency = 800.0f;
    oscPhase = 0.0f;
    oscPhaseIncrement = 2.0f * juce::MathConstants<float>::pi
                        * frequency / (float)sampleRate;
    
}


void KarplusStrongAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KarplusStrongAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void KarplusStrongAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer&)
{
    float cutoff = cutoffParam->get();
    float sampleRate = (float)getSampleRate();
    
    filterCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi
                                  * cutoff / sampleRate);
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = getTotalNumOutputChannels();
    
    float feedbackGain = decayParam->get();
    float delayTime = delayParam->get();

    
    int delaySamples = (int)(delayTime * sampleRate);
    delaySamples = juce::jmax(1, delaySamples);
    
    auto pluck = pluckParam->get();
    if (pluck)
    {
        pluckParam->setValueNotifyingHost(0);
        noiseGain = 1.0f;
    }
    
    for (int i = 0; i < numSamples; ++i)
    {
        float excitation = 0.0f;
        
        if (noiseGain > 0.0f)
        {
            int burstType = burstTypeParam->getIndex();
            
            switch (burstType)
            {
                case 0:
                    excitation = 2.0f * noiseGain *
                    (random.nextFloat()) - noiseGain;
                    break;
                    
                case 1:
                    excitation = std::sin(oscPhase) * noiseGain;
                    break;
                    
                case 2:
                {
                    float norm = oscPhase / juce::MathConstants<float>::twoPi;
                    float tri = 2.0f * std::abs(2.0f * (norm - std::floor(norm + 0.5f))) - 1.0f;
                    excitation = tri * noiseGain;
                    break;
                }
                    
                case 3:
                    excitation = (std::sin(oscPhase) >= 0.0f ? 1.0f : -1.0f)
                    * noiseGain;
                    break;
                    
                case 4:
                {
                    float norm = oscPhase / juce::MathConstants<float>::twoPi;
                    float saw = 2.0f * (norm - std::floor(norm + 0.5f));
                    excitation = saw * noiseGain;
                    break;
                }
            }
            
            oscPhase += oscPhaseIncrement;
            if (oscPhase >= juce::MathConstants<float>::twoPi)
                oscPhase -= juce::MathConstants<float>::twoPi;
        }
        
        if (noiseGain >= 0.0f)
            noiseGain -= 1.0f / (widthParam->get() * (float)getSampleRate());
        
        if (noiseGain < 0.0f)
            noiseGain = 0.0f;
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* delayData = delayBuffer.getWritePointer(channel);
            int bufferSize = delayBuffer.getNumSamples();
            
            int readPosition =
            (delayWritePosition + bufferSize - delaySamples) % bufferSize;
            
            float delayedSample = delayData[readPosition];
            
            float filtered = filterCoeff * delayedSample
            + (1.0f - filterCoeff) * filterState[channel];
            
            filterState[channel] = filtered;
            
            float writeSample = excitation + filtered * feedbackGain;
            
            delayData[delayWritePosition] = writeSample;
            
            buffer.getWritePointer(channel)[i] = delayedSample;
        }
        
        delayWritePosition++;
        if (delayWritePosition >= delayBuffer.getNumSamples())
            delayWritePosition = 0;
    }
    
}




//==============================================================================
bool KarplusStrongAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* KarplusStrongAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(this);
}

//==============================================================================
void KarplusStrongAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{

}

void KarplusStrongAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KarplusStrongAudioProcessor();
}



