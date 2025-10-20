#include "MainComponent.h"
#include "PlayerGui.h"


MainComponent::MainComponent()
{
    playerGui.setPlayerAudio(&playerAudio);
    addAndMakeVisible(playerGui);
    setSize(800, 250);
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
    playerAudio.performLoop(); //check for loop and restart if loop toggled to true
}

void MainComponent::releaseResources()
{
    playerAudio.releaseResources();
}





