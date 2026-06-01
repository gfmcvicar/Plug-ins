#include "PluginProcessor.h"
#include "PluginEditor.h"

KarplusStrongAudioProcessorEditor::KarplusStrongAudioProcessorEditor (KarplusStrongAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

KarplusStrongAudioProcessorEditor::~KarplusStrongAudioProcessorEditor()
{
}

void KarplusStrongAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void KarplusStrongAudioProcessorEditor::resized()
{
}
