/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <filesystem>


//==============================================================================
SampleDumpAudioProcessorEditor::SampleDumpAudioProcessorEditor (SampleDumpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 300);
    countDownState = 5;

    parentDirectoryInputLabel.setText("Parent Folder:", juce::dontSendNotification);
    directoryInputLabel.setText("Directory Name:", juce::dontSendNotification);
    bufferSizeLabel.setText("Buffer Size:", juce::dontSendNotification);
    toggleFFTLabel.setText("Perform FFT:", juce::dontSendNotification);
    bufferCountLabel.setText("Number of Buffers: ", juce::dontSendNotification);
	
    parentDirectoryInput.setBorder(juce::BorderSize<int>(1));
    directoryInput.setBorder(juce::BorderSize<int>(1));

    startButton.setButtonText(juce::String("Start"));
    browseButton.setButtonText(juce::String("Browse"));

    bufferSizeInput.addItemList({ "64", "128", "256", "512", "1024", "2048", "4096" }, 1);
    bufferSizeInput.setSelectedId(6);

    bufferCountInput.setInputRestrictions(0, "0123456789");
    bufferCountInput.setText("50");

    browseButton.onClick = [this](){
        fileChooser.reset(new juce::FileChooser(juce::String("choose a folder"), juce::File::getCurrentWorkingDirectory(), juce::String("*")));

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this](const juce::FileChooser& chooser) {
            auto result = chooser.getURLResult();
            parentDirectoryInput.setText(result.getLocalFile().getFullPathName());           
        });

    };

    startButton.onClick = [this]() {
        if (!parentDirectoryInput.getText().containsChar(':')) {
            alertWindow->showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Invalid Parrent Directory", "Parent Directory is not an absolute path.");
            return;
        }

        juce::File parentDirectory = juce::File(parentDirectoryInput.getText());

        if (!parentDirectory.isDirectory()) {
            alertWindow->showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Invalid Parrent Directory", "Parent Directory doesn't exist.");
            return;
        }
        else if (directoryInput.getText().isEmpty()) {
            alertWindow->showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Enter A Directory", "Enter the name for the new directory the buffer logs will be saved.");
            return;
        }
        else if (directoryInput.getText().containsAnyOf("/\\<>:\"|?*")) {
            alertWindow->showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Invalid Directory Name", "New Directory is using an invalid character (/ \\<>:\"|?*).");
            return;
        }
        
        directory = juce::File(parentDirectory.getFullPathName() + '\\' + directoryInput.getText());
        if (directory.exists()) {
            int suffix = 1;
            while (directory.exists()) {
                directory = juce::File(parentDirectory.getFullPathName() + '\\' + directoryInput.getText() + "(" + std::to_string(suffix) + ")");
                suffix++;
            }
        }

        startTimer(1000);
        timerCallback();
    };

    addAndMakeVisible(parentDirectoryInputLabel);
    addAndMakeVisible(directoryInputLabel);
	addAndMakeVisible(parentDirectoryInput);
    addAndMakeVisible(directoryInput);
    addAndMakeVisible(startButton);
    addAndMakeVisible(browseButton);
    addAndMakeVisible(bufferSizeInput);
    addAndMakeVisible(bufferSizeLabel);
    addAndMakeVisible(toggleFFTLabel);
    addAndMakeVisible(toggleFFTButton);
    addAndMakeVisible(bufferCountLabel);
    addAndMakeVisible(bufferCountInput);
}

SampleDumpAudioProcessorEditor::~SampleDumpAudioProcessorEditor()
{
}

//==============================================================================
void SampleDumpAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));


}

void SampleDumpAudioProcessorEditor::resized()
{
    auto mainArea = getLocalBounds();
    mainArea.reduce(10, 30);

    auto parentDirectoryArea = mainArea.removeFromTop(25);
    parentDirectoryInputLabel.setBounds(parentDirectoryArea.removeFromLeft(130));
    browseButton.setBounds(parentDirectoryArea.removeFromRight(80));
    parentDirectoryArea.removeFromRight(10);
    parentDirectoryInput.setBounds(parentDirectoryArea);

    mainArea.removeFromTop(10);

    auto directoryArea = mainArea.removeFromTop(25);
    directoryInputLabel.setBounds(directoryArea.removeFromLeft(130));
    directoryInput.setBounds(directoryArea);
    
    mainArea.removeFromTop(10);

    auto captureCountArea = mainArea.removeFromTop(25);

    bufferCountLabel.setBounds(captureCountArea.removeFromLeft(130));
    bufferCountInput.setBounds(captureCountArea.removeFromLeft(130));

    mainArea.removeFromTop(10);

    auto bufferSizeArea = mainArea.removeFromTop(25);
    bufferSizeLabel.setBounds(bufferSizeArea.removeFromLeft(130));
    bufferSizeInput.setBounds(bufferSizeArea.removeFromLeft(130));

    mainArea.removeFromTop(10);

    auto toggleFFTArea = mainArea.removeFromTop(25);
    toggleFFTLabel.setBounds(toggleFFTArea.removeFromLeft(125));
    toggleFFTButton.setBounds(toggleFFTArea);

    mainArea.removeFromTop(10);

    auto buttonArea = mainArea.removeFromTop(25);
    buttonArea.reduce(130, 0);

    startButton.setBounds(buttonArea);
}

void SampleDumpAudioProcessorEditor::timerCallback()
{
    if (countDownState > 0) {
        if (startButton.isEnabled())
            startButton.setEnabled(false);
        startButton.setButtonText(std::to_string(countDownState));
        countDownState--;
    }
    else if (countDownState == 0 && !audioProcessor.capturingSamples) {
        
        audioProcessor.bufferSize = std::stoi(bufferSizeInput.getText().toStdString());
        audioProcessor.bufferCount = std::stoi(bufferCountInput.getText().toStdString());

        audioProcessor.buffers = new float* [audioProcessor.bufferCount];
        for (int i = 0; i < audioProcessor.bufferCount; i++) {
            audioProcessor.buffers[i] = new float[audioProcessor.bufferSize];
        }
        startButton.setButtonText("Capturing...");
        countDownState--;
        audioProcessor.capturingSamples = true;
    }
    else if (countDownState == -1 && !audioProcessor.capturingSamples) {
        dumpSamples();
        countDownState = 5;
        startButton.setEnabled(true);
        startButton.setButtonText("Start");
        stopTimer();
    }

}

void SampleDumpAudioProcessorEditor::dumpSamples() {
    directory.createDirectory();
    for (int i = 0; i < audioProcessor.bufferCount; i++) {
        std::string dataString = "";
        for (int k = 0; k < audioProcessor.bufferSize; k++) {
            dataString += std::to_string(audioProcessor.buffers[i][k]) + '\n';
        }

        std::string logFilePath = directory.getFullPathName().toStdString() + "/" + std::to_string(i) + ".txt";
        juce::File logFile = juce::File(logFilePath);

        logFile.create();
        logFile.appendText(dataString);
    }
}
