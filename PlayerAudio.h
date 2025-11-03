#pragma once
#include <JuceHeader.h>

class PlayerAudio {
private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;


    
    bool isLooping = false;
    bool wasPlaying = false;
    bool isMuted = false;
    float lastGain = 1.0f;

    double currentSampleRate = 0.0;

    // A-B loop state
    bool isABLoopEnabled = false;
    double markerA = -1.0;
    double markerB = -1.0;

public:
    juce::Array<juce::File> playlist;
    PlayerAudio();
    ~PlayerAudio();
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    bool loadFile(const juce::File& file);
    juce::String getMetadataInfo();
    juce::String formatTime(double seconds);
    void play();
    void restart();
    void stop();
    void pause();
    void goToEnd();
    void goToStart();
    void loop();
    void setGain(float gain);
    void setGain(float gain, bool isMute);
    void setPosition(double pos);
    double getPosition() const;
    double getLength() const;
    void setPositionNormalized(double normalizedPos);
    double getPositionNormalized() const;
    bool isLoopingEnabled() const { return isLooping; }
    void performLoop();
    void setSpeed(double speed);
    void addtoPlaylist(const juce::Array<juce::File>& files);
    void loadFromPlaylist(int i);

    // A-B loop API
    void setMarkerA();
    void setMarkerB();
    void clearMarkers();
    void toggleABLoop();
    double getMarkerA() const { return markerA; }
    double getMarkerB() const { return markerB; }
    bool isABLoopActive() const { return isABLoopEnabled && markerA >= 0 && markerB >= 0 && markerB > markerA; }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerAudio)
};