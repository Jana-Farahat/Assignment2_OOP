#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGui : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    PlayerGui();
    ~PlayerGui() override;

    void setPlayerAudio(PlayerAudio* audio) {
        playerAudio = audio;
    }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PlayerAudio* playerAudio = NULL;

    juce::TextButton loadButton{ "Load" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton pauseButton{ "Pause ||" };
    juce::TextButton playButton{ "Play >" };
    juce::TextButton goToEndButton{ "Go to end >|" };
    juce::TextButton goToStartButton{ "Go to start |<" };
    juce::TextButton loopButton{ "Loop" };
    juce::Slider volumeSlider;
    juce::Slider positionSlider;
    juce::Slider speedSlider;
    juce::Label timeLabel;
    juce::TextButton muteButton{ "Mute" };
    bool isDraggingSlider = false;
    juce::Label fileInfoLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void sliderDragStarted(juce::Slider* slider) override;
    void sliderDragEnded(juce::Slider* slider) override;
    void timerCallback() override;
    juce::String formatTime(double seconds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGui)
};
