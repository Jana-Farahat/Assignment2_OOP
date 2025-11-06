#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerAudio;
class PlayerGui;

enum class ListMode {
    Playlist,
    Markers,
    PlaylistLeft,
    PlaylistRight
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
    ListRowComponent(PlayerAudio* audio, PlayerAudio* audioRight, PlayerGui* gui, int rowIndex, ListMode mode, std::function<void()> onUpdate)
        : playerAudio(audio), playerAudioRight(audioRight), playerGui(gui), index(rowIndex), rowMode(mode), updateCallback(onUpdate)
    {
        setVisible(true);
        setEnabled(true);
        if (mode == ListMode::Playlist || mode == ListMode::PlaylistLeft || mode == ListMode::PlaylistRight) {
            actionButton.setVisible(false);
            actionButton.setEnabled(false);
            playLeftButton.setButtonText("Play L");
            playRightButton.setButtonText("Play R");
            playLeftButton.setLookAndFeel(&bigButtonLookAndFeel);
            playRightButton.setLookAndFeel(&bigButtonLookAndFeel);
            playLeftButton.addListener(this);
            playRightButton.addListener(this);
            playLeftButton.setVisible(true);
            playLeftButton.setEnabled(true);
            playRightButton.setVisible(true);
            playRightButton.setEnabled(true);
            addAndMakeVisible(&playLeftButton);
            addAndMakeVisible(&playRightButton);
        }
        else {
            actionButton.setButtonText("Load");
            actionButton.setVisible(true);
            actionButton.setEnabled(true);
        }
        removeButton.setButtonText("X");
        actionButton.setLookAndFeel(&bigButtonLookAndFeel);
        removeButton.setLookAndFeel(&bigButtonLookAndFeel);
        actionButton.addListener(this);
        removeButton.addListener(this);
        if (mode != ListMode::Playlist && mode != ListMode::PlaylistLeft && mode != ListMode::PlaylistRight) {
            addAndMakeVisible(&actionButton);
        }
        addAndMakeVisible(&removeButton);
        for (auto* btn : { &removeButton }) {
            btn->setVisible(true);
            btn->setEnabled(true);
        }
    }

    ~ListRowComponent() {
        actionButton.setLookAndFeel(nullptr);
        removeButton.setLookAndFeel(nullptr);
    }

    void resized() override {
        int buttonWidth = 60;
        int removeWidth = getHeight() - 4;
        int playButtonWidth = 70;
        int playButtonHeight = getHeight() - 4;
        int buttonArea;
        if (rowMode == ListMode::Playlist || rowMode == ListMode::PlaylistLeft || rowMode == ListMode::PlaylistRight) {
            buttonArea = playButtonWidth + playButtonWidth + removeWidth + 6;
            removeButton.setBounds(getWidth() - removeWidth, 2, removeWidth, removeWidth);
            playRightButton.setBounds(getWidth() - removeWidth - playButtonWidth - 2, 2, playButtonWidth, playButtonHeight);
            playLeftButton.setBounds(getWidth() - removeWidth - playButtonWidth - playButtonWidth - 4, 2, playButtonWidth, playButtonHeight);
        }
        else {
            buttonArea = buttonWidth + removeWidth + 4;
            removeButton.setBounds(getWidth() - removeWidth, 2, removeWidth, removeWidth);
            actionButton.setBounds(getWidth() - removeWidth - buttonWidth - 2, 2, buttonWidth, getHeight() - 4);
        }
        textAreaWidth = getWidth() - buttonArea;
    }

    void paint(juce::Graphics& g) override;

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
    PlayerAudio* playerAudioRight = nullptr;
    PlayerGui* playerGui = nullptr;
    int index;
    ListMode rowMode;
    juce::TextButton actionButton;
    juce::TextButton playLeftButton;
    juce::TextButton playRightButton;
    juce::TextButton removeButton;
    std::function<void()> updateCallback;
    int textAreaWidth = 0;
};

class ListModel : public juce::ListBoxModel {
public:
    ListModel(PlayerAudio* audio, PlayerAudio* audioRight, PlayerGui* gui, ListMode mode)
        : playerAudio(audio), playerAudioRight(audioRight), playerGui(gui), listMode(mode) {
    }
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void selectedRowsChanged(int lastRowSelected) override;
    PlayerAudio* playerAudio = nullptr;
    PlayerAudio* playerAudioRight = nullptr;
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

    void setPlayerAudio(PlayerAudio* audioLeft, PlayerAudio* audioRight) {
        playerAudioLeft = audioLeft;
        playerAudioRight = audioRight;
        playlistListModel.playerAudio = audioLeft;
        playlistListModel.playerAudioRight = audioRight;
        playlistListModel.playerGui = this;
        markersListModelLeft.playerAudio = audioLeft;
        markersListModelLeft.playerGui = this;
        markersListModelRight.playerAudio = audioRight;
        markersListModelRight.playerGui = this;

        PlaylistBox.setModel(&playlistListModel);
        markersListBoxLeft.setModel(&markersListModelLeft);
        markersListBoxRight.setModel(&markersListModelRight);
    }

    void updateMarkersListLeft() {
        markersListBoxLeft.updateContent();
        markersListBoxLeft.repaint();
    }

    void updateMarkersListRight() {
        markersListBoxRight.updateContent();
        markersListBoxRight.repaint();
    }

    void updatePlaylist() {
        PlaylistBox.updateContent();
        PlaylistBox.repaint();
    }

    void updateMetadataLeft() {
        if (playerAudioLeft != nullptr) {
            juce::String metadata = "Currently playing:\n" + playerAudioLeft->getMetadataInfo();
            fileInfoLabelLeft.setText(metadata, juce::dontSendNotification);
        }
    }

    void updateMetadataRight() {
        if (playerAudioRight != nullptr) {
            juce::String metadata = "Currently playing:\n" + playerAudioRight->getMetadataInfo();
            fileInfoLabelRight.setText(metadata, juce::dontSendNotification);
        }
    }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void loadSessionState(const juce::String& filePath, float volume);

    PlayerAudio* playerAudioLeft = nullptr;
    PlayerAudio* playerAudioRight = nullptr;

    void restoreGUIFromSession();

private:

    juce::ImageButton loadButtonLeft;
    juce::ImageButton restartButtonLeft;
    juce::ImageButton stopButtonLeft;
    juce::ImageButton pauseButtonLeft;
    juce::ImageButton playButtonLeft;
    juce::ImageButton goToEndButtonLeft;
    juce::ImageButton goToStartButtonLeft;
    juce::ImageButton loopButtonLeft;
    juce::Slider volumeSliderLeft;
    juce::Slider positionSliderLeft;
    juce::Slider speedSliderLeft;
    juce::Label speedLabelLeft;
    juce::Label timeLabelLeft;
    juce::ImageButton muteButtonLeft;
    bool isDraggingSliderLeft = false;
    juce::Label fileInfoLabelLeft;
    double progressValueLeft = 0.0;
    juce::ProgressBar progressBarLeft{ progressValueLeft };
    juce::TextButton setMarkerAButtonLeft{ "Set A" };
    juce::TextButton setMarkerBButtonLeft{ "Set B" };
    juce::TextButton abLoopButtonLeft{ "A-B Loop" };
    juce::TextButton clearMarkersButtonLeft{ "Clear A-B" };
    juce::Label abMarkersLabelLeft;
    juce::ImageButton setMarkerButtonLeft;
    juce::ImageButton forward10sButtonLeft;
    juce::ImageButton backward10sButtonLeft;

    juce::Slider mixSlider;


    juce::ImageButton loadButtonRight;
    juce::ImageButton restartButtonRight;
    juce::ImageButton stopButtonRight;
    juce::ImageButton pauseButtonRight;
    juce::ImageButton playButtonRight;
    juce::ImageButton goToEndButtonRight;
    juce::ImageButton goToStartButtonRight;
    juce::ImageButton loopButtonRight;
    juce::Slider volumeSliderRight;
    juce::Slider positionSliderRight;
    juce::Slider speedSliderRight;
    juce::Label speedLabelRight;
    juce::Label timeLabelRight;
    juce::ImageButton muteButtonRight;
    bool isDraggingSliderRight = false;
    juce::Label fileInfoLabelRight;
    double progressValueRight = 0.0;
    juce::ProgressBar progressBarRight{ progressValueRight };
    juce::TextButton setMarkerAButtonRight{ "Set A" };
    juce::TextButton setMarkerBButtonRight{ "Set B" };
    juce::TextButton abLoopButtonRight{ "A-B Loop" };
    juce::TextButton clearMarkersButtonRight{ "Clear A-B" };
    juce::Label abMarkersLabelRight;
    juce::ImageButton setMarkerButtonRight;
    juce::ImageButton forward10sButtonRight;
    juce::ImageButton backward10sButtonRight;

    juce::ImageButton loadFilesButton;
    juce::ListBox PlaylistBox;
    juce::ListBox markersListBoxLeft;
    juce::ListBox markersListBoxRight;
    ListModel markersListModelLeft{ nullptr, nullptr, this, ListMode::Markers };
    ListModel markersListModelRight{ nullptr, nullptr, this, ListMode::Markers };
    ListModel playlistListModel{ nullptr, nullptr, this, ListMode::Playlist };


    juce::TextButton resetLeftButton{ "RESET L" };
    juce::TextButton resetRightButton{ "RESET R" };
    void resetLeftPlayer();
    void resetRightPlayer();
    
    BigButtonLookAndFeel myButtonLookAndFeel;

    std::unique_ptr<juce::FileChooser> fileChooser;


    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void sliderDragStarted(juce::Slider* slider) override;
    void sliderDragEnded(juce::Slider* slider) override;
    void timerCallback() override;
    juce::String formatTime(double seconds);

    juce::Image loadIconFromBinary(const void* data, size_t dataSize);
    void setupIconButton(juce::ImageButton* button, const juce::Image& icon);
    void updateMuteButtonIcon();

    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGui)
};