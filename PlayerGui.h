#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"
class PlayerGui: public juce::Component,
                 public juce::Button::Listener,
                 public juce::Slider::Listener 
{
public:
    PlayerGui();
    ~PlayerGui() override;

    void setPlayerAudio(PlayerAudio* audio) {
        playerAudio = audio;
    }
    void paint(juce::Graphics& g)override ;
    void resized() override ;

private:
    PlayerAudio* playerAudio = nullptr; //to be connected to player audio


    juce::TextButton loadButton{ "Load" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton pauseButton{ "Pause" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton go_to_end_Button{ "Go to end" };
    juce::Slider volumeSlider;


    std::unique_ptr<juce::FileChooser> fileChooser;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

};