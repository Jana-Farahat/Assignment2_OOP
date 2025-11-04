#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerAudio;
class PlayerGui;

enum class ListMode {
    Playlist,
    Markers
};

class BigButtonLookAndFeel : public juce::LookAndFeel_V4 {
public:
    juce::Font getTextButtonFont(juce::TextButton&, int) override {
        return juce::Font(juce::FontOptions().withHeight(18.0f));
    }
};

class ListRowComponent : public juce::Component, public juce::Button::Listener {
private:
    BigButtonLookAndFeel bigButtonLookAndFeel;
public:
    ListRowComponent(PlayerAudio* audio, PlayerGui* gui, int rowIndex, ListMode mode, std::function<void()> onUpdate)
        : playerAudio(audio), playerGui(gui), index(rowIndex), rowMode(mode), updateCallback(onUpdate)
    {
        setVisible(true);
        setEnabled(true);
        actionButton.setButtonText(mode == ListMode::Playlist ? "Play" : "Load");
        removeButton.setButtonText("X");
        actionButton.setLookAndFeel(&bigButtonLookAndFeel);
        removeButton.setLookAndFeel(&bigButtonLookAndFeel);
        actionButton.addListener(this);
        removeButton.addListener(this);
        for (auto* btn : { &actionButton, &removeButton }) {
            btn->setVisible(true);
            btn->setEnabled(true);
            addAndMakeVisible(btn);
        }
    }
    
    ~ListRowComponent() {
        actionButton.setLookAndFeel(nullptr);
        removeButton.setLookAndFeel(nullptr);
    }

    void resized() override {
        int buttonWidth = 100;
        int removeWidth = 60;
        int buttonArea = buttonWidth + removeWidth + 4;
        removeButton.setBounds(getWidth() - removeWidth, 2, removeWidth - 2, getHeight() - 4);
        actionButton.setBounds(getWidth() - removeWidth - buttonWidth - 2, 2, buttonWidth, getHeight() - 4);
        textAreaWidth = getWidth() - buttonArea;
    }

    void paint(juce::Graphics& g) override {
        if (getWidth() <= 0)
            return;

        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions().withHeight(20.0f));
        
        if (rowMode == ListMode::Markers) {
            if (index >= playerAudio->getMarkerCount())
                return;
            double markerTime = playerAudio->getMarkerTime(index);
            if (markerTime < 0)
                return;
            int minutes = (int)(markerTime / 60);
            int secs = (int)(markerTime) % 60;
            juce::String markerText = juce::String::formatted("Marker %d", index + 1);
            juce::String timeText = juce::String::formatted("%02d:%02d", minutes, secs);
            
            int buttonArea = 170;
            int timeColWidth = 80;
            int markerColWidth = getWidth() - buttonArea - timeColWidth - 8;
            
            g.drawText(markerText, 4, 0, markerColWidth, getHeight(), juce::Justification::centredLeft);
            g.drawText(timeText, markerColWidth + 12, 0, timeColWidth, getHeight(), juce::Justification::centredLeft);
        }
        else {
            if (index >= playerAudio->playlist.size())
                return;
            juce::File file = playerAudio->playlist[index];
            juce::String fileName = file.getFileName();
            
            juce::String durationText = "--:--";
            juce::String currentPath = playerAudio->getCurrentSongPath();
            if (currentPath.isNotEmpty() && file.getFullPathName() == currentPath) {
                double duration = playerAudio->getLength();
                if (duration > 0) {
                    int minutes = (int)(duration / 60);
                    int secs = (int)(duration) % 60;
                    durationText = juce::String::formatted("%02d:%02d", minutes, secs);
                }
            }
            else {
                juce::AudioFormatManager formatManager;
                formatManager.registerBasicFormats();
                std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
                if (reader != nullptr) {
                    double duration = reader->lengthInSamples / reader->sampleRate;
                    if (duration > 0) {
                        int minutes = (int)(duration / 60);
                        int secs = (int)(duration) % 60;
                        durationText = juce::String::formatted("%02d:%02d", minutes, secs);
                    }
                }
            }
            
            int trackColWidth = (int)(getWidth() * 0.4f);
            int durationColWidth = (int)(getWidth() * 0.25f);
            g.drawText(fileName, 4, 0, trackColWidth, getHeight(), juce::Justification::centredLeft);
            g.drawText(durationText, trackColWidth + 4, 0, durationColWidth, getHeight(), juce::Justification::centredLeft);
        }
    }

    void buttonClicked(juce::Button* button) override;

    void updateIndex(int newIndex) {
        index = newIndex;
        setVisible(true);
        setEnabled(true);
        for (auto* btn : { &actionButton, &removeButton }) {
            btn->setEnabled(true);
            btn->setVisible(true);
        }
        repaint();
    }

private:
    PlayerAudio* playerAudio = nullptr;
    PlayerGui* playerGui = nullptr;
    int index;
    ListMode rowMode;
    juce::TextButton actionButton;
    juce::TextButton removeButton;
    std::function<void()> updateCallback;
    int textAreaWidth = 0;
};

class ListModel : public juce::ListBoxModel {
public:
    ListModel(PlayerAudio* audio, PlayerGui* gui, ListMode mode) 
        : playerAudio(audio), playerGui(gui), listMode(mode) {}
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void selectedRowsChanged(int lastRowSelected) override;
    PlayerAudio* playerAudio = nullptr;
    PlayerGui* playerGui = nullptr;
    ListMode listMode = ListMode::Playlist;
};


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
        playlistListModel.playerAudio = audio;
        playlistListModel.playerGui = this;
        markersListModel.playerAudio = audio;
        markersListModel.playerGui = this;
        
        PlaylistBox.setModel(&playlistListModel);
        markersListBox.setModel(&markersListModel);
    }

    void updateMarkersList() {
        markersListBox.updateContent();
        markersListBox.repaint();
    }

    void updatePlaylist() {
        PlaylistBox.updateContent();
        PlaylistBox.repaint();
    }
    
    void updateMetadata() {
        if (playerAudio != nullptr) {
            juce::String metadata = "Currently playing:\n" + playerAudio->getMetadataInfo();
            fileInfoLabel.setText(metadata, juce::dontSendNotification);
        }
    }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void loadSessionState(const juce::String& filePath, float volume);

private:
    PlayerAudio* playerAudio = nullptr;

    juce::ImageButton loadButton;
    juce::ImageButton restartButton;
    juce::ImageButton stopButton;
    juce::ImageButton pauseButton;
    juce::ImageButton playButton;
    juce::ImageButton goToEndButton;
    juce::ImageButton goToStartButton;
    juce::ImageButton loopButton;
    juce::Slider volumeSlider;
    juce::Slider positionSlider;
    juce::Slider speedSlider;
    juce::Label speedLabel;
    juce::Label timeLabel;
    juce::ImageButton muteButton;
    bool isDraggingSlider = false;
    juce::Label fileInfoLabel;

    double progressValue = 0.0;
    juce::ProgressBar progressBar{ progressValue };

    juce::TextButton setMarkerAButton{ "Set A" };
    juce::TextButton setMarkerBButton{ "Set B" };
    juce::TextButton abLoopButton{ "A-B Loop" };
    juce::TextButton clearMarkersButton{ "Clear A-B" };
    juce::Label abMarkersLabel;

    juce::ImageButton setMarkerButton;
    juce::ListBox markersListBox;
    ListModel markersListModel{nullptr, this, ListMode::Markers};
    ListModel playlistListModel{nullptr, this, ListMode::Playlist};

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::ImageButton forward10sButton;
    juce::ImageButton backward10sButton;


    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void sliderDragStarted(juce::Slider* slider) override;
    void sliderDragEnded(juce::Slider* slider) override;
    void timerCallback() override;
    juce::String formatTime(double seconds);

    juce::Image loadIconFromBinary(const void* data, size_t dataSize);
    void setupIconButton(juce::ImageButton* button, const juce::Image& icon);
    void updateMuteButtonIcon();

    juce::ListBox PlaylistBox;
    juce::ImageButton loadFilesButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGui)
};