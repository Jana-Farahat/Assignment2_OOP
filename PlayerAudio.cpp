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

        juce::StringPairArray metadata = reader->metadataValues;

        if (metadata.size() > 0)
        {
            info += "All metadata keys:\n";
            juce::StringArray keys = metadata.getAllKeys();

            for (int i = 0; i < keys.size(); ++i)
            {
                juce::String key = keys[i];
                juce::String value = metadata.getValue(key, "EMPTY");
                info += key + "' = '" + value + "'\n";
            }
        }
        else
        {
            info += "NO METADATA FOUND - File may not contain ID3 tags\n";
        }

        return info;
    }

    return "Error reading file";
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
    if (isLooping && transportSource.hasStreamFinished()) {
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
