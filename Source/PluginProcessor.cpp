/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SampleDumpAudioProcessor::SampleDumpAudioProcessor()
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
    capturingSamples = false;
    currentBuffer = 0;
    currentIndex = 0;

}

SampleDumpAudioProcessor::~SampleDumpAudioProcessor()
{
}

//==============================================================================
const juce::String SampleDumpAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SampleDumpAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SampleDumpAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SampleDumpAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SampleDumpAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SampleDumpAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SampleDumpAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SampleDumpAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SampleDumpAudioProcessor::getProgramName (int index)
{
    return {};
}

void SampleDumpAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SampleDumpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..


}

void SampleDumpAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SampleDumpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SampleDumpAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

    auto leftChannel = buffer.getWritePointer(0);
    for (int i = 0; i < buffer.getNumSamples() && capturingSamples; i++) {
        captureSample(leftChannel[i]);
    }
}

//==============================================================================
bool SampleDumpAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SampleDumpAudioProcessor::createEditor()
{
    return new SampleDumpAudioProcessorEditor (*this);
}

//==============================================================================
void SampleDumpAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SampleDumpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleDumpAudioProcessor();
}

void SampleDumpAudioProcessor::captureSample(float& sample)
{
    if (currentIndex > bufferSize) {
        currentIndex = 0;
        currentBuffer++;
    }

    if (currentBuffer < bufferCount) {
        buffers[currentBuffer][currentIndex] = sample;
        currentIndex++;
    }
    else {
        capturingSamples = false;
        currentIndex = 0;
        currentBuffer = 0;
    }
    
}
