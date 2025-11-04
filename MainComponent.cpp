#include "MainComponent.h"
#include "PlayerGui.h"


MainComponent::MainComponent()
{
    playerGui.setPlayerAudio(&playerAudioLeft, &playerAudioRight);
    addAndMakeVisible(playerGui);
    setSize(1500, 650);
    setAudioChannels(0, 2);

    playerAudioLeft.loadSession();
    playerAudioRight.loadSession();


}


MainComponent::~MainComponent()
{
    playerAudioLeft.saveSession();
    playerAudioRight.saveSession();
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    playerAudioLeft.prepareToPlay(samplesPerBlockExpected, sampleRate);
    playerAudioRight.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.buffer->clear();
    
    juce::AudioSourceChannelInfo leftBuffer;
    leftBuffer.buffer = bufferToFill.buffer;
    leftBuffer.startSample = bufferToFill.startSample;
    leftBuffer.numSamples = bufferToFill.numSamples;
    playerAudioLeft.getNextAudioBlock(leftBuffer);
    
    if (bufferToFill.buffer->getNumChannels() >= 2) {
        auto tempBuffer = std::make_unique<juce::AudioBuffer<float>>(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
        tempBuffer->clear();
        
        juce::AudioSourceChannelInfo rightBuffer;
        rightBuffer.buffer = tempBuffer.get();
        rightBuffer.startSample = 0;
        rightBuffer.numSamples = bufferToFill.numSamples;
        playerAudioRight.getNextAudioBlock(rightBuffer);
        
        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel) {
            float* channelData = bufferToFill.buffer->getWritePointer(channel);
            const float* rightChannelData = tempBuffer->getReadPointer(channel);
            for (int sample = 0; sample < bufferToFill.numSamples; ++sample) {
                int index = bufferToFill.startSample + sample;
                if (channel == 0) {
                    channelData[index] = channelData[index];
                }
                else {
                    channelData[index] = rightChannelData[sample];
                }
            }
        }
    }
    
    playerAudioLeft.performLoop();
    playerAudioRight.performLoop();
}

void MainComponent::releaseResources()
{
    playerAudioLeft.releaseResources();
    playerAudioRight.releaseResources();
}





