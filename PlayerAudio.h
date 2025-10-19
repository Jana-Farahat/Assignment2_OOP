#pragma once
#include <JuceHeader.h>

class PlayerAudio {

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

public:
    PlayerAudio();
    ~PlayerAudio();
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    bool loadFile(const juce::File& file);
    void play();
    void restart();
    void stop();
    void pause();
    void goToEnd();
    void setGain(float gain);
    void setPosition(double pos);
    double getPosition() const;
    double getLength() const;

};

