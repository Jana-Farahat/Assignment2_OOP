#pragma once
#include <JuceHeader.h>

class PlayerAudio {
private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

 
    double lastKnownPosition = 0.0;

    juce::String currentSong;
    double currentPosition = 0.0;
    float currentVolume = 1.0f;

    

    bool isLooping = false;
    bool wasPlaying = false;
    bool isMuted = false;
    float lastGain = 1.0f;

    double currentSampleRate = 0.0;

    bool isABLoopEnabled = false;
    double markerA = -1.0;
    double markerB = -1.0;

    juce::Array<double> trackMarkers;

    juce::File loadedFile; //edit

    bool sessionFileLoaded = false;

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
    double getPosition() ;
    double getLength() const;
    void setPositionNormalized(double normalizedPos);
    double getPositionNormalized() const;
    bool isLoopingEnabled() const { return isLooping; }
    void performLoop();
    void setSpeed(double speed);
    void addtoPlaylist(const juce::Array<juce::File>& files);
    void loadFromPlaylist(int i);

    void setMarkerA();
    void setMarkerB();
    void clearMarkers();
    void toggleABLoop();
    double getMarkerA() const;
    double getMarkerB() const;
    bool isABLoopActive() const;
    double getMarkerATime() const;
    double getMarkerBTime() const;

    void addTrackMarker();
    void removeTrackMarker(int index);
    void jumpToMarker(int index);
    int getMarkerCount() const { return trackMarkers.size(); }
    double getMarkerTime(int index) const;
    void clearTrackMarkers();

    void saveSession(const juce::File& sessionFile);
    void loadSession(const juce::File& sessionFile);
    float getCurrentVolume() const { return currentVolume; }
    juce::String getCurrentSongPath() const { return currentSong; }

    void tenSec(bool forward);

    bool getIsMuted() const { return isMuted; }
    bool isPlaying() const { return transportSource.isPlaying(); }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerAudio)
};