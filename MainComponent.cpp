#include "MainComponent.h"
#include "PlayerGui.h"

MainComponent::MainComponent()
{
    playerGui.setPlayerAudio(&playerAudioLeft, &playerAudioRight);
    addAndMakeVisible(playerGui);
    setSize(1500, 650);

    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    juce::File appDir = appDataDir.getChildFile("SimpleAudioPlayer");

    DBG("App data directory: " + appDataDir.getFullPathName());
    DBG("App directory: " + appDir.getFullPathName());

    if (!appDir.exists()) {
        appDir.createDirectory();
        DBG("Created app directory");
    }

    juce::File sessionFileLeft = appDir.getChildFile("session_left.txt");
    juce::File sessionFileRight = appDir.getChildFile("session_right.txt");

    DBG("Left session file: " + sessionFileLeft.getFullPathName());
    DBG("Right session file: " + sessionFileRight.getFullPathName());

    // Load sessions
    playerAudioLeft.loadSession(sessionFileLeft);
    playerAudioRight.loadSession(sessionFileRight);

    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    juce::File appDir = appDataDir.getChildFile("SimpleAudioPlayer");

    // [DELETED] ??? ???? ?????? ?????? ?????? ?? ???
    // if (!appDir.exists())
    //     appDir.createDirectory();

    juce::File sessionFileLeft = appDir.getChildFile("session_left.txt");
    juce::File sessionFileRight = appDir.getChildFile("session_right.txt");

    // FIX: Save sessions properly
    playerAudioLeft.saveSession(sessionFileLeft);
    playerAudioRight.saveSession(sessionFileRight);

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




