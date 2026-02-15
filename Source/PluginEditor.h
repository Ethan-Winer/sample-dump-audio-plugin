/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SampleDumpAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SampleDumpAudioProcessorEditor (SampleDumpAudioProcessor&);
    ~SampleDumpAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SampleDumpAudioProcessor& audioProcessor;

    juce::Label parentDirectoryInputLabel;
    juce::Label directoryInputLabel;
    juce::Label bufferSizeLabel;
    juce::Label toggleFFTLabel;
    juce::Label bufferCountLabel;
    juce::TextEditor parentDirectoryInput;
    juce::TextEditor directoryInput;
    juce::TextEditor bufferCountInput;
    juce::TextButton browseButton;
    juce::TextButton startButton;
    juce::ComboBox bufferSizeInput;
    juce::ToggleButton toggleFFTButton;

    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<juce::AlertWindow> alertWindow;

    int timerValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleDumpAudioProcessorEditor)
};
