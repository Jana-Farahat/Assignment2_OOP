#pragma once

#include <JuceHeader.h>
#include "PlayerGui.h"


class MainComponent : public juce::AudioAppComponent
    
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void resized() override  
    {
        playerGui.setBounds(getLocalBounds());  
    }
   

private:    

    PlayerAudio playerAudioLeft;   
    PlayerAudio playerAudioRight;
    PlayerGui playerGui;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

