#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <filesystem>

SampleDumpAudioProcessorEditor::SampleDumpAudioProcessorEditor (SampleDumpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    parentDirectoryInputLabel.setText("Parent Folder:", juce::dontSendNotification);
    directoryInputLabel.setText("Directory Name:", juce::dontSendNotification);
    bufferSizeLabel.setText("Buffer Size:", juce::dontSendNotification);
    bufferCountLabel.setText("Number of Buffers: ", juce::dontSendNotification);
	
    parentDirectoryInput.setBorder(juce::BorderSize<int>(1));
    directoryInput.setBorder(juce::BorderSize<int>(1));

    startButton.setButtonText(juce::String("Start"));
    browseButton.setButtonText(juce::String("Browse"));

    bufferSizeInput.addItemList({ "64", "128", "256", "512", "1024", "2048", "4096" }, 1);
    bufferSizeInput.setSelectedId(6);

    bufferCountInput.setInputRestrictions(0, "0123456789");
    bufferCountInput.setText("50");
    
    setSize (600, 175);
    addAndMakeVisible(parentDirectoryInputLabel);
    addAndMakeVisible(directoryInputLabel);
	addAndMakeVisible(parentDirectoryInput);
    addAndMakeVisible(directoryInput);
    addAndMakeVisible(startButton);
    addAndMakeVisible(browseButton);
    addAndMakeVisible(bufferSizeInput);
    addAndMakeVisible(bufferSizeLabel);
    addAndMakeVisible(bufferCountLabel);
    addAndMakeVisible(bufferCountInput);
    
    //button events
    browseButton.onClick = [this](){
        fileChooser.reset(new juce::FileChooser(juce::String("choose a folder"), juce::File::getCurrentWorkingDirectory(), juce::String("*")));

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories, [this](const juce::FileChooser& chooser) {
            auto result = chooser.getURLResult();
            parentDirectoryInput.setText(result.getLocalFile().getFullPathName());           
        });

    };

    startButton.onClick = [this]() {
        if (validInput()) {
            // numbering folder if it is a duplicate
            auto parentDirectoryPath = juce::File(parentDirectoryInput.getText()).getFullPathName();
            directory = juce::File(parentDirectoryPath + '\\' + directoryInput.getText());
            if (directory.exists()) {
                int suffix = 1;
                while (directory.exists()) {
                    directory = juce::File(parentDirectoryPath + '\\' + directoryInput.getText() + "(" + std::to_string(suffix) + ")");
                    suffix++;
                }
            }

            startButton.setEnabled(false);
            countDownState = 5;
            startTimer(1000);
            timerCallback();
        }
    };

}

SampleDumpAudioProcessorEditor::~SampleDumpAudioProcessorEditor()
{
}

void SampleDumpAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SampleDumpAudioProcessorEditor::resized()
{
    auto mainArea = getLocalBounds();
    int labelWidth = 140;
    int areaHeight = 35;
    int areaPadding = 5;

    auto parentDirectoryArea = mainArea.removeFromTop(areaHeight).reduced(areaPadding);
    parentDirectoryInputLabel.setBounds(parentDirectoryArea.removeFromLeft(labelWidth));
    browseButton.setBounds(parentDirectoryArea.removeFromRight(80));
    parentDirectoryInput.setBounds(parentDirectoryArea);


    auto directoryArea = mainArea.removeFromTop(areaHeight).reduced(areaPadding);
    directoryInputLabel.setBounds(directoryArea.removeFromLeft(labelWidth));
    directoryInput.setBounds(directoryArea);


    auto captureCountArea = mainArea.removeFromTop(areaHeight).reduced(areaPadding);
    bufferCountLabel.setBounds(captureCountArea.removeFromLeft(labelWidth));
    bufferCountInput.setBounds(captureCountArea.removeFromLeft(labelWidth));


    auto bufferSizeArea = mainArea.removeFromTop(areaHeight).reduced(areaPadding);
    bufferSizeLabel.setBounds(bufferSizeArea.removeFromLeft(labelWidth));
    bufferSizeInput.setBounds(bufferSizeArea.removeFromLeft(labelWidth));

    auto buttonArea = mainArea.removeFromTop(areaHeight).reduced(areaPadding);
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
    else if (countDownState == 0) {
        
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

bool SampleDumpAudioProcessorEditor::validInput() {
    if (!parentDirectoryInput.getText().containsChar(':')) {
        alertWindow->showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Invalid Parrent Directory", "Parent Directory is not an absolute path.");
        return false;
    }

    juce::File parentDirectory = juce::File(parentDirectoryInput.getText());

    if (!parentDirectory.isDirectory()) {
        alertWindow->showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Invalid Parrent Directory", "Parent Directory doesn't exist.");
        return false;
    }
    else if (directoryInput.getText().isEmpty()) {
        alertWindow->showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Enter A Directory", "Enter the name for the new directory the buffer logs will be saved.");
        return false;
    }
    else if (directoryInput.getText().containsAnyOf("/\\<>:\"|?*")) {
        alertWindow->showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Invalid Directory Name", "New Directory is using an invalid character (/ \\<>:\"|?*).");
        return false;
    }

    return true;
}