#include "MainComponent.h"
#include "PlayerGui.h"


MainComponent::MainComponent()
{
    playerGui.setPlayerAudio(&playerAudio);
    addAndMakeVisible(playerGui);
    setSize(900, 600);
    setAudioChannels(0, 2);

    playerAudio.loadSession();

    if (playerAudio.getCurrentSongPath().isNotEmpty())
    {
        playerGui.loadSessionState(playerAudio.getCurrentSongPath(),playerAudio.getCurrentVolume());
    }


}


MainComponent::~MainComponent()
{
    playerAudio.saveSession();
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





