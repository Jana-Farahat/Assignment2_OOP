#include "MainComponent.h"
#include "PlayerGui.h"


MainComponent::MainComponent()
{
    playerGui.setPlayerAudio(&playerAudio);
    addAndMakeVisible(playerGui);
    setSize(800, 200);
    setAudioChannels(0, 2);
    
  
}


MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    playerAudio.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    playerAudio.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    playerAudio.releaseResources();
}




