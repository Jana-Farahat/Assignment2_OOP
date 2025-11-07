#include "MainComponent.h"
#include "PlayerGui.h"

MainComponent::MainComponent()
{
    playerGui.setPlayerAudio(&playerAudioLeft, &playerAudioRight);
    addAndMakeVisible(playerGui);
    setSize(1500, 650);

    juce::File exeFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    juce::File sessionFile = exeFile.getParentDirectory().getChildFile("session.txt");

    playerGui.loadSession(sessionFile);

    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    juce::File exeFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    juce::File sessionFile = exeFile.getParentDirectory().getChildFile("session.txt");

    playerGui.saveSession(sessionFile);

    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    playerAudioLeft.prepareToPlay(samplesPerBlockExpected, sampleRate);
    playerAudioRight.prepareToPlay(samplesPerBlockExpected, sampleRate);

    juce::MessageManager::callAsync([this]() {
        playerGui.restoreGUIFromSession();
    });
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.buffer->clear();

    playerAudioLeft.getNextAudioBlock(bufferToFill);

    
    if (bufferToFill.buffer->getNumChannels() >= 2) {
      
        auto tempBuffer = std::make_unique<juce::AudioBuffer<float>>(1, bufferToFill.numSamples);
        tempBuffer->clear();

        juce::AudioSourceChannelInfo rightInfo;
        rightInfo.buffer = tempBuffer.get();
        rightInfo.startSample = 0;
        rightInfo.numSamples = bufferToFill.numSamples;

        playerAudioRight.getNextAudioBlock(rightInfo);

        bufferToFill.buffer->copyFrom(1, bufferToFill.startSample,
            *tempBuffer, 0, 0, bufferToFill.numSamples);
    }

    
    playerAudioLeft.performLoop();
    playerAudioRight.performLoop();
}
void MainComponent::releaseResources()
{
    playerAudioLeft.releaseResources();
    playerAudioRight.releaseResources();
}




