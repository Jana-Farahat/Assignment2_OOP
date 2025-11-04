#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerAudio;
class PlayerGui;

class MarkerRowComponent : public juce::Component, public juce::Button::Listener {
public:
    MarkerRowComponent(PlayerAudio* audio, int markerIndex, std::function<void()> onUpdate): playerAudio(audio), index(markerIndex), updateCallback(onUpdate)
    {
        setVisible(true);
        setEnabled(true);
        loadButton.setButtonText("Load");
        removeButton.setButtonText("X");
        loadButton.addListener(this);
        removeButton.addListener(this);
        for (auto* btn : { &loadButton, &removeButton }) {
            btn->setVisible(true);
            btn->setEnabled(true);
            addAndMakeVisible(btn);
        }
    }

    void resized() override {
        int buttonWidth = 50;
        int removeWidth = 30;
        int buttonArea = buttonWidth + removeWidth + 4;
        removeButton.setBounds(getWidth() - removeWidth, 2, removeWidth - 2, getHeight() - 4);
        loadButton.setBounds(getWidth() - removeWidth - buttonWidth - 2, 2, buttonWidth, getHeight() - 4);
        textAreaWidth = getWidth() - buttonArea;
    }

    void paint(juce::Graphics& g) override {
        if (playerAudio == nullptr || index >= playerAudio->getMarkerCount() || getWidth() <= 0)
            return;

        double markerTime = playerAudio->getMarkerTime(index);
        if (markerTime < 0) return;

        int minutes = (int)(markerTime / 60);
        int secs = (int)(markerTime) % 60;
        juce::String markerText = juce::String::formatted("Marker %d (%02d:%02d)", index + 1, minutes, secs);

        g.setColour(juce::Colours::white);
        g.drawText(markerText, 4, 0, (textAreaWidth > 0 ? textAreaWidth : getWidth() - 90),
            getHeight(), juce::Justification::centredLeft);
    }

    void buttonClicked(juce::Button* button) override {
        if (playerAudio == nullptr) return;
        if (button == &loadButton) {
            playerAudio->jumpToMarker(index);
        }
        else if (button == &removeButton) {
            playerAudio->removeTrackMarker(index);
            if (updateCallback) updateCallback();
        }
    }

    void updateIndex(int newIndex) {
        index = newIndex;
        setVisible(true);
        setEnabled(true);
        for (auto* btn : { &loadButton, &removeButton }) {
            btn->setEnabled(true);
            btn->setVisible(true);
        }
        repaint();
    }

private:
    PlayerAudio* playerAudio = nullptr;
    int index;
    juce::TextButton loadButton;
    juce::TextButton removeButton;
    std::function<void()> updateCallback;
    int textAreaWidth = 0;
};

class MarkerListModel : public juce::ListBoxModel {
public:
    MarkerListModel(PlayerAudio* audio, PlayerGui* gui) : playerAudio(audio), playerGui(gui) {}
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void selectedRowsChanged(int lastRowSelected) override;
    PlayerAudio* playerAudio = nullptr;
    PlayerGui* playerGui = nullptr;
};


class PlayerGui : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer,
    public juce::ListBoxModel
{
public:
    PlayerGui();
    ~PlayerGui() override;

    void setPlayerAudio(PlayerAudio* audio) {
        playerAudio = audio;
        markersListModel.playerAudio = audio;
        markersListModel.playerGui = this;
    }

    void updateMarkersList() {
        markersListBox.updateContent();
        markersListBox.repaint();
    }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void loadSessionState(const juce::String& filePath, float volume);

private:
    PlayerAudio* playerAudio = nullptr;

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

    double progressValue = 0.0;
    juce::ProgressBar progressBar{ progressValue };

    //AB controls
    juce::TextButton setMarkerAButton{ "Set A" };
    juce::TextButton setMarkerBButton{ "Set B" };
    juce::TextButton abLoopButton{ "A-B Loop" };
    juce::TextButton clearMarkersButton{ "Clear A-B" };
    juce::Label abMarkersLabel;

    juce::TextButton setMarkerButton{ "Set Marker" };
    juce::ListBox markersListBox;
    MarkerListModel markersListModel; //edited here

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TextButton forward10sButton{ ">> 10s" };
    juce::TextButton backward10sButton{ "<< 10s" };


    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void sliderDragStarted(juce::Slider* slider) override;
    void sliderDragEnded(juce::Slider* slider) override;
    void timerCallback() override;
    juce::String formatTime(double seconds);


    //
    juce::ListBox PlaylistBox;
    juce::TextButton AddButton{ "Add Files" };
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    void selectedRowsChanged(int lastRowSelected) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGui)
};