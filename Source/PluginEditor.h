#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SampleDumpAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    SampleDumpAudioProcessorEditor (SampleDumpAudioProcessor&);
    ~SampleDumpAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SampleDumpAudioProcessor& audioProcessor;

    juce::Label parentDirectoryInputLabel;
    juce::Label directoryInputLabel;
    juce::Label bufferSizeLabel;
    juce::Label bufferCountLabel;
    juce::TextEditor parentDirectoryInput;
    juce::TextEditor directoryInput;
    juce::TextEditor bufferCountInput;
    juce::TextButton browseButton;
    juce::TextButton startButton;
    juce::ComboBox bufferSizeInput;
    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<juce::AlertWindow> alertWindow;

    int countDownState;
    juce::File directory;

    void timerCallback();
    void dumpSamples();
    bool validInput();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleDumpAudioProcessorEditor)
};
