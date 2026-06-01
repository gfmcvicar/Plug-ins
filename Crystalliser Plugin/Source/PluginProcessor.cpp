#include "PluginProcessor.h"
#include "PluginEditor.h"

// ==============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ==============================================================================

CrystalliserAudioProcessor::CrystalliserAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMS", createParameterLayout())
{
}

CrystalliserAudioProcessor::~CrystalliserAudioProcessor() {}

// ==============================================================================
// AUDIO ENGINE
// ==============================================================================

void CrystalliserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    CrystalliserBuffer.setSize (2, (int)(sr * 5.0));
    CrystalliserBuffer.clear();
    reverb.setParameters (reverbParams);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels      = getTotalNumOutputChannels();

    masterLimiter.prepare (spec);
    masterLimiter.setRelease   (50.0f);   // Smooth 50ms release
    masterLimiter.setThreshold (-0.5f);   // Hard ceiling just under 0dBFS
}

void CrystalliserAudioProcessor::releaseResources() {}

bool CrystalliserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void CrystalliserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    const float baseLength  = *parameters.getRawParameterValue ("length");
    const float mix         = *parameters.getRawParameterValue ("mix");
    const float persistence = *parameters.getRawParameterValue ("persistence");
    const float lfoRate     = *parameters.getRawParameterValue ("lfoRate");
    const float lfoDepth    = *parameters.getRawParameterValue ("lfoDepth");

    // Persistence math:
    // At 0%: feedbackGain = 0.6,  inputGain = 0.4
    // At 100%: feedbackGain = 0.995, inputGain = 0.005
    const float feedbackGain  = 0.6f  + (persistence * (0.995f - 0.6f));
    const float inputGain     = 0.4f  + (persistence * (0.005f - 0.4f));
    const float persistenceGain = 0.2f + (persistence * 0.8f);

    reverbParams.roomSize = *parameters.getRawParameterValue ("reverbRoom");
    reverbParams.damping  = *parameters.getRawParameterValue ("reverbDamping");
    reverbParams.width    = *parameters.getRawParameterValue ("reverbWidth");
    reverbParams.wetLevel = *parameters.getRawParameterValue ("reverbWet");
    reverb.setParameters (reverbParams);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float lfoValue = std::sin (lfoPhase);
        lfoPhase += (juce::MathConstants<float>::twoPi * lfoRate) / (float)sr;
        if (lfoPhase > juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;

        targetLoopLength = (int)(juce::jlimit (0.05f, 5.0f,
            baseLength + (lfoValue * baseLength * lfoDepth * 0.5f)) * sr);

        if (state == Recording)
        {
            state = Frozen;
            playbackIndex     = 0.0f;
            currentLoopLength = (float)targetLoopLength;
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float inSample = buffer.getSample (ch, sample);

            if (state == Frozen)
            {
                //Single-pole IIR smoother
                currentLoopLength = 0.8f * currentLoopLength + 0.2f * (float)targetLoopLength;
                int   bufferSize  = CrystalliserBuffer.getNumSamples();
                float readPos     = std::fmod (playbackIndex, currentLoopLength);

                // Dynamic fade size (capped at 100ms)
                float fadeSize = juce::jmin (currentLoopLength * 0.1f, 4410.0f);

                // Linear interpolation
                int   indexA = (int)readPos % bufferSize;
                int   indexB = (indexA + 1) % bufferSize;
                float frac   = readPos - (float)((int)readPos);
                float frozen = CrystalliserBuffer.getSample (ch, indexA)
                             + frac * (CrystalliserBuffer.getSample (ch, indexB)
                                     - CrystalliserBuffer.getSample (ch, indexA));

                // Dynamic windowing
                float windowGain = 1.0f;
                if      (readPos < fadeSize)
                    windowGain = readPos / fadeSize;
                else if (readPos > (currentLoopLength - fadeSize))
                    windowGain = (currentLoopLength - readPos) / fadeSize;

                windowGain = juce::jlimit (0.0f, 1.0f, windowGain);
                frozen *= (0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * windowGain));

                // Reverb
                float revL = frozen, revR = frozen;
                reverb.processStereo (&revL, &revR, 1);
                frozen = (ch == 0) ? revL : revR;

                // Dry/wet mix (constant-power)
                float dryG = std::cos (mix * juce::MathConstants<float>::halfPi);
                float wetG = std::sin (mix * juce::MathConstants<float>::halfPi);

                buffer.setSample (ch, sample,
                    std::tanh ((dryG * inSample) + (wetG * (frozen * 8.0f * persistenceGain))));

                // Feedback / persistence write-back
                float existing = CrystalliserBuffer.getSample (ch, (int)readPos % bufferSize);
                CrystalliserBuffer.setSample (ch, (int)readPos % bufferSize,
                    (existing * feedbackGain) + (inSample * inputGain));
            }
            else
            {
                CrystalliserBuffer.setSample (ch, writeIndex, inSample);
            }
        }

        playbackIndex += 1.0f;
        if (playbackIndex >= currentLoopLength) { playbackIndex = 0; state = Recording; }
        writeIndex = (writeIndex + 1) % CrystalliserBuffer.getNumSamples();
    }

    juce::dsp::AudioBlock<float>            audioBlock (buffer);
    juce::dsp::ProcessContextReplacing<float> context (audioBlock);
    masterLimiter.process (context);
}

// ==============================================================================
// PARAMETER LAYOUT
// ==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout CrystalliserAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back (std::make_unique<juce::AudioParameterFloat> ("length",        "Length",     0.01f, 1.0f,  0.07f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("mix",           "Mix",        0.0f,  1.0f,  1.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("persistence",   "Persistence",0.0f,  1.0f,  0.95f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoRate",       "LFO Rate",   0.1f,  10.0f, 0.1f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoDepth",      "LFO Depth",  0.0f,  1.0f,  1.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("reverbRoom",    "Room",       0.0f,  1.0f,  0.5f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("reverbDamping", "Damping",    0.0f,  1.0f,  0.5f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("reverbWidth",   "Width",      0.0f,  1.0f,  1.0f));
    p.push_back (std::make_unique<juce::AudioParameterFloat> ("reverbWet",     "Reverb Wet", 0.0f,  1.0f,  0.5f));

    return { p.begin(), p.end() };
}

// ==============================================================================
// BOILERPLATE
// ==============================================================================

bool                        CrystalliserAudioProcessor::hasEditor()        const { return true; }
juce::AudioProcessorEditor* CrystalliserAudioProcessor::createEditor()           { return new CrystalliserAudioProcessorEditor (*this); }
const juce::String          CrystalliserAudioProcessor::getName()          const { return "The Crystalliser"; }
bool                        CrystalliserAudioProcessor::acceptsMidi()      const { return false; }
bool                        CrystalliserAudioProcessor::producesMidi()     const { return false; }
bool                        CrystalliserAudioProcessor::isMidiEffect()     const { return false; }
double                      CrystalliserAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int                         CrystalliserAudioProcessor::getNumPrograms()         { return 1; }
int                         CrystalliserAudioProcessor::getCurrentProgram()      { return 0; }
void                        CrystalliserAudioProcessor::setCurrentProgram (int)  {}
const juce::String          CrystalliserAudioProcessor::getProgramName    (int)  { return {}; }
void                        CrystalliserAudioProcessor::changeProgramName (int, const juce::String&) {}

void CrystalliserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary (*parameters.copyState().createXml(), destData);
}

void CrystalliserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

// ==============================================================================
// PLUGIN ENTRY POINT
// ==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CrystalliserAudioProcessor();
}
