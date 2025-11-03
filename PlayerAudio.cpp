#include "PlayerAudio.h"

juce::File loadedFile;

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

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    transportSource.getNextAudioBlock(bufferToFill);
}

void PlayerAudio::releaseResources() {
    transportSource.releaseResources();
}

bool PlayerAudio::loadFile(const juce::File& file) {
    if (file.existsAsFile())
    {
        if (auto* reader = formatManager.createReaderFor(file))
        {
            transportSource.stop();
            transportSource.setSource(NULL);
            readerSource.reset();

            readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

            currentSampleRate = reader->sampleRate;

            transportSource.setSource(readerSource.get(),
                0,
                NULL,
                currentSampleRate);

            // Clear A-B markers on new file load
            clearMarkers();

            loadedFile = file;
            return true;
        }
    }
    return false;
}

juce::String PlayerAudio::getMetadataInfo()
{
    if (!loadedFile.existsAsFile())
        return "No file loaded";

    if (auto* reader = formatManager.createReaderFor(loadedFile))
    {
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

        if (metadata.size() > 0)
        {
            info += "Metadata keys:\n";
            juce::StringArray keys = metadata.getAllKeys();

            for (int i = 0; i < keys.size(); i++)
            {
                juce::String key = keys[i];
                juce::String value = metadata.getValue(key, "EMPTY");
                info += key + " = " + value + "'\n";
            }
        }
        else
        {
            info += "NO METADATA FOUND \n";
        }
        return info;
    }

    return "Error reading file";
}

void PlayerAudio::addtoPlaylist(const juce::Array<juce::File>& files) {

    for (auto& f : files) {
        playlist.add(f);
    }

}

void PlayerAudio::loadFromPlaylist(int i) {
    juce::File file(playlist[i]);
    loadFile(file);
}



void PlayerAudio::play() {
    transportSource.start();
}

void PlayerAudio::stop() {
    transportSource.stop();
    transportSource.setPosition(0.0);
}

void PlayerAudio::pause() {
    transportSource.stop();
    transportSource.setPosition(transportSource.getCurrentPosition());
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
    //AB loop has priority over regular loop
    if (isABLoopActive()) {
        double currentPos = transportSource.getCurrentPosition();
        if (currentPos >= markerB || currentPos < markerA || transportSource.hasStreamFinished())
        {
            transportSource.setPosition(markerA);
            if (!transportSource.isPlaying())
            {
                transportSource.start();
            }
        }
    }
    else if (isLooping && transportSource.hasStreamFinished()) {
        transportSource.setPosition(0.0);
        transportSource.start();
    }
}

void PlayerAudio::setGain(float gain, bool mute)
{
    if (mute)
    {
        isMuted = !isMuted;

        if (isMuted)
        {
            lastGain = transportSource.getGain();
            transportSource.setGain(0.0f);
        }
        else
        {
            transportSource.setGain(lastGain);
        }
    }
    else
    {
        if (!isMuted)
        {
            transportSource.setGain(gain);
            lastGain = gain;
        }
    }
}

void PlayerAudio::setGain(float gain)
{
    setGain(gain, false);
}

void PlayerAudio::setPosition(double pos) {
    transportSource.setPosition(pos);
}

double PlayerAudio::getPosition() const {
    return transportSource.getCurrentPosition();
}

double PlayerAudio::getLength() const {
    return transportSource.getLengthInSeconds();
}

double PlayerAudio::getPositionNormalized() const {
    double len = transportSource.getLengthInSeconds();
    if (len <= 0.0) return 0.0;
    return transportSource.getCurrentPosition() / len;
}

void PlayerAudio::setPositionNormalized(double normalizedPos) {
    double len = transportSource.getLengthInSeconds();
    if (len <= 0.0) return;
    transportSource.setPosition(normalizedPos * len);
}

void PlayerAudio::setSpeed(double speed)
{
    if (readerSource != NULL && currentSampleRate > 0.0)
    {
        bool playing = transportSource.isPlaying();
        double pos = transportSource.getCurrentPosition();

        transportSource.stop();
        transportSource.setSource(NULL);

        double newRate = currentSampleRate * speed;
        transportSource.setSource(readerSource.get(),
            0,
            NULL,
            newRate);

        transportSource.setPosition(pos);
        if (playing)
            transportSource.start();
    }
}

// A-B loop methods
void PlayerAudio::setMarkerA() {
    markerA = transportSource.getCurrentPosition();
    if (markerA < 0) markerA = 0.0;
    double length = transportSource.getLengthInSeconds();
    if (markerA > length) markerA = length;
    if (markerB >= 0 && markerA >= markerB) {
        markerB = -1.0;
    }
}

void PlayerAudio::setMarkerB() {
    markerB = transportSource.getCurrentPosition();
    if (markerB < 0) 
    markerB = 0.0;
    double length = transportSource.getLengthInSeconds();
    if (markerB > length) 
    markerB = length;
    if (markerA >= 0 && markerB <= markerA) {
        markerA = -1.0;
    }
}

void PlayerAudio::clearMarkers() {
    markerA = -1.0;
    markerB = -1.0;
    isABLoopEnabled = false;
}

void PlayerAudio::toggleABLoop() {
    isABLoopEnabled = !isABLoopEnabled;
    if (isABLoopEnabled && (markerA < 0 || markerB < 0 || markerB <= markerA)) {
        isABLoopEnabled = false;
    }
}