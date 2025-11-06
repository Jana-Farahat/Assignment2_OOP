#include "PlayerAudio.h"
#include <fstream>
#include <string>
#include <iostream>


PlayerAudio::PlayerAudio() {
    formatManager.registerBasicFormats();
    transportSource.setLooping(false);
}

PlayerAudio::~PlayerAudio() {
    releaseResources();
}

juce::String PlayerAudio::formatTime(double seconds) {
    int minutes = (int)(seconds / 60);
    int secs = (int)(seconds) % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}


void PlayerAudio::saveSession(const juce::File& sessionFile)
{
 
    juce::String song = currentSong;

    DBG("Saving session for: " + sessionFile.getFileName() +
        " - Position: " + juce::String(currentPosition) +
        ", Song: " + song +
        ", Playlist size: " + juce::String(playlist.size()));

    std::unique_ptr<juce::FileOutputStream> stream(sessionFile.createOutputStream());
    if (stream == nullptr) {
        DBG("Could not open session file for writing: " + sessionFile.getFullPathName());
        return;
    }

 
    stream->writeString(song + "\n");
    stream->writeString(juce::String(currentPosition) + "\n");
    stream->writeString(juce::String(currentVolume) + "\n");

    
    stream->writeString(juce::String(playlist.size()) + "\n");
    for (const auto& file : playlist) {
        stream->writeString(file.getFullPathName() + "\n");
        DBG("Saving playlist file: " + file.getFullPathName());
    }

 
    stream->writeString(juce::String(trackMarkers.size()) + "\n");
    for (const auto& marker : trackMarkers) {
        stream->writeString(juce::String(marker) + "\n");
    }

    
    stream->flush();
    stream.reset();

    DBG("Session saved successfully to: " + sessionFile.getFullPathName());
}

void PlayerAudio::loadSession(const juce::File& sessionFile)
{
    if (!sessionFile.existsAsFile()) {
        
        DBG("No saved session found for: " + sessionFile.getFullPathName());
        return;
    }

    std::unique_ptr<juce::FileInputStream> stream(sessionFile.createInputStream());
    if (stream == nullptr) {
        DBG("Could not open session file for reading: " + sessionFile.getFullPathName());
        return;
    }

    currentSong = stream->readNextLine();
    loadedFile = juce::File(currentSong);
    currentPosition = stream->readNextLine().getDoubleValue();
    currentVolume = stream->readNextLine().getFloatValue();

    playlist.clear();
    int numTracks = stream->readNextLine().getIntValue();
    for (int i = 0; i < numTracks; ++i) {
        juce::String trackPath = stream->readNextLine();
        if (trackPath.isNotEmpty()) {
            juce::File file(trackPath);
            playlist.add(file);
        }
    }

    trackMarkers.clear();
    int numMarkers = stream->readNextLine().getIntValue();
    for (int i = 0; i < numMarkers; ++i) {
        double marker = stream->readNextLine().getDoubleValue();
        trackMarkers.add(marker);
    }

    
    sessionFileLoaded = true;

    DBG("Session loaded - File: " + currentSong + ", Position: " + juce::String(currentPosition));
}
void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

   
    if (sessionFileLoaded)
    {
        if (currentSong.isNotEmpty()) {
            juce::File lastFile(currentSong);
            if (lastFile.existsAsFile()) {
                bool loaded = loadFile(lastFile);
                if (loaded) {
                    setGain(currentVolume);

                    if (currentPosition > 0.0) {
                        double length = getLength();
                        if (length > 0.0) {
                            double newPos = juce::jmin(currentPosition, length);
                            transportSource.setPosition(newPos);
                            DBG("Position restored in prepareToPlay: " + juce::String(newPos));
                        }
                    }
                }
            }
        }
       
        sessionFileLoaded = false;
    }
}

//playlist
void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    transportSource.getNextAudioBlock(bufferToFill);
}

void PlayerAudio::releaseResources() {
    transportSource.releaseResources();
}

bool PlayerAudio::loadFile(const juce::File& file) {
    if (file.existsAsFile()) {
        if (auto* reader = formatManager.createReaderFor(file)) {
            transportSource.stop();
            transportSource.setSource(NULL);
            readerSource.reset();

            readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

            currentSampleRate = reader->sampleRate;

            transportSource.setSource(readerSource.get(),
                0,
                NULL,
                currentSampleRate);

            clearMarkers();
            clearTrackMarkers();

            currentSong = file.getFullPathName();
            loadedFile = file;

            
            sessionFileLoaded = false;

           
            currentPosition = 0.0;

            setGain(currentVolume);

            return true;
        }
    }
    return false;
}



//file info
juce::String PlayerAudio::getMetadataInfo()
{
    if (!loadedFile.existsAsFile())
        return "No file loaded";

    if (auto* reader = formatManager.createReaderFor(loadedFile)) {
        juce::String info;
        info += "File: " + loadedFile.getFileName() + "\n";
        info += "Size: " + juce::String(loadedFile.getSize() / 1024) + " KB\n";
        info += "Duration: " + formatTime(reader->lengthInSamples / reader->sampleRate) + "\n";

        juce::StringPairArray  metadata = reader->metadataValues;

        juce::String author;
        author = metadata.getValue("artist", "");
        if (author.isEmpty())
            author = metadata.getValue("composer", "");
        if (author.isEmpty())
            author = metadata.getValue("albumartist", "");
        if (author.isEmpty())
            author = metadata.getValue("author", "");

        info += "Author: " + (author.isEmpty() ? "Unknown" : author) + "\n";

        ///If there exist a metadata it will print the following

        if (metadata.size() > 0) {
            info += "Metadata keys:\n";
            juce::StringArray keys = metadata.getAllKeys();

            for (int i = 0; i < keys.size(); i++) {
                juce::String key = keys[i];
                juce::String value = metadata.getValue(key, "EMPTY");
                info += key + " = " + value + "'\n";
            }
        }
        else {
            info += "NO METADATA FOUND \n";
        }
        return info;
    }

    return "Error reading file";
}

//add files
void PlayerAudio::addtoPlaylist(const juce::Array<juce::File>& files) {

    for (auto& f : files) {
        playlist.add(f);
    }

}
//load files
void PlayerAudio::loadFromPlaylist(int i) {
    juce::File file(playlist[i]);
    loadFile(file);
}



void PlayerAudio::play() {
    transportSource.start();
}

void PlayerAudio::stop() {
    currentPosition = 0.0; 
    transportSource.stop();
    transportSource.setPosition(0.0);
}

void PlayerAudio::pause() {
    currentPosition = transportSource.getCurrentPosition(); 
    transportSource.stop();
    transportSource.setPosition(currentPosition);
}

void PlayerAudio::goToEnd() {
    transportSource.setPosition(transportSource.getLengthInSeconds());
}

void PlayerAudio::goToStart() {
    transportSource.setPosition(0.0);
}

void PlayerAudio::restart() {
    transportSource.setPosition(0.0);
    transportSource.start();
}

void PlayerAudio::loop() {
    isLooping = !isLooping;
}

void PlayerAudio::performLoop() {
    if (isABLoopActive()) {
        double len = getLength();
        double markerATime = markerA * len;
        double markerBTime = markerB * len;
        double absPos = transportSource.getCurrentPosition();

        if (absPos >= markerBTime || absPos < markerATime || transportSource.hasStreamFinished()) {
            setPositionNormalized(markerA);
            if (!transportSource.isPlaying())
                transportSource.start();
        }
    }
    else if (isLooping && transportSource.hasStreamFinished()) {
        transportSource.setPosition(0.0);
        transportSource.start();
    }
}

void PlayerAudio::setGain(float gain, bool mute)
{
    if (mute) {
        isMuted = !isMuted;

        if (isMuted) {
            lastGain = transportSource.getGain();
            transportSource.setGain(0.0f);
        }
        else {
            transportSource.setGain(lastGain);
        }
    }
    else {
        if (!isMuted) {
            transportSource.setGain(gain);
            lastGain = gain;

        }
    }
}

void PlayerAudio::setGain(float gain)
{
    setGain(gain, false);
    currentVolume = gain; //edit
}

void PlayerAudio::setPosition(double pos) {
    transportSource.setPosition(pos);
}

double PlayerAudio::getPosition() { 
    if (transportSource.isPlaying())
    {
        currentPosition = transportSource.getCurrentPosition();
    }
    return transportSource.getCurrentPosition();
}

double PlayerAudio::getLength() const {
    return transportSource.getLengthInSeconds();
}

double PlayerAudio::getPositionNormalized() const {
    double len = transportSource.getLengthInSeconds();
    if (len <= 0.0)
        return 0.0;
    return transportSource.getCurrentPosition() / len;
}

void PlayerAudio::setPositionNormalized(double normalizedPos) {
    double len = transportSource.getLengthInSeconds();
    if (len <= 0.0)
        return;
    transportSource.setPosition(normalizedPos * len);
    currentPosition = normalizedPos * len;
}

void PlayerAudio::setSpeed(double speed)
{
    if (readerSource != NULL && currentSampleRate > 0.0) {
        bool playing = transportSource.isPlaying();
        double normalizedPos = getPositionNormalized();

        transportSource.stop();
        transportSource.setSource(NULL);

        double newRate = currentSampleRate * speed;
        transportSource.setSource(readerSource.get(), 0, NULL, newRate);

        setPositionNormalized(normalizedPos);
        if (playing)
            transportSource.start();
    }
}

void PlayerAudio::setMarkerA() {
    markerA = juce::jlimit(0.0, 1.0, getPositionNormalized());
    if (markerB >= 0 && markerA >= markerB)
        markerB = -1.0;
}

void PlayerAudio::setMarkerB() {
    markerB = juce::jlimit(0.0, 1.0, getPositionNormalized());
    if (markerA >= 0 && markerB <= markerA)
        markerA = -1.0;
}

void PlayerAudio::clearMarkers() {
    markerA = -1.0;
    markerB = -1.0;
    isABLoopEnabled = false;
}

void PlayerAudio::toggleABLoop() {
    isABLoopEnabled = !isABLoopEnabled;
    if (isABLoopEnabled && (markerA < 0 || markerB < 0 || markerB <= markerA))
        isABLoopEnabled = false;
}

//new yehiia

double PlayerAudio::getMarkerA() const {
    return markerA;
}

double PlayerAudio::getMarkerB() const {
    return markerB;
}

bool PlayerAudio::isABLoopActive() const {
    return isABLoopEnabled && markerA >= 0 && markerB >= 0 && markerB > markerA;
}

double PlayerAudio::getMarkerATime() const {
    return (markerA >= 0) ? markerA * getLength() : -1.0;
}

double PlayerAudio::getMarkerBTime() const {
    return (markerB >= 0) ? markerB * getLength() : -1.0;
}

void PlayerAudio::addTrackMarker() {
    trackMarkers.add(juce::jlimit(0.0, 1.0, getPositionNormalized()));
    trackMarkers.sort();
}

void PlayerAudio::removeTrackMarker(int index) {
    if (index >= 0 && index < trackMarkers.size())
        trackMarkers.remove(index);
}

void PlayerAudio::jumpToMarker(int index) {
    if (index >= 0 && index < trackMarkers.size()) {
        double normalizedPos = trackMarkers[index];
        setPositionNormalized(normalizedPos);
        if (!transportSource.isPlaying())
            transportSource.start();
    }
}

double PlayerAudio::getMarkerTime(int index) const {
    return (index >= 0 && index < trackMarkers.size()) ? trackMarkers[index] * getLength() : -1.0;
}

void PlayerAudio::clearTrackMarkers() {
    trackMarkers.clear();
}


void PlayerAudio::tenSec(bool forward)
{
    double newPosition = transportSource.getCurrentPosition();

    if (forward)
        newPosition += 10.0;
    else {
        newPosition -= 10.0;
    }

    if (newPosition < 0)
        newPosition = 0;
    if (newPosition > transportSource.getLengthInSeconds())
        newPosition = transportSource.getLengthInSeconds();

    transportSource.setPosition(newPosition);
}

