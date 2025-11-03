#include "PlayerGui.h"

PlayerGui::PlayerGui() {
    for (auto* btn : { &loadButton, &restartButton , &stopButton, &pauseButton , &playButton, &goToEndButton, &loopButton, &muteButton, &goToStartButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    addAndMakeVisible(AddButton);
    addAndMakeVisible(PlaylistBox);
    AddButton.addListener(this);
    PlaylistBox.setModel(this);

    fileInfoLabel.setText("No file loaded", juce::dontSendNotification);
    fileInfoLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    fileInfoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fileInfoLabel);

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    positionSlider.setRange(0.0, 1.0, 0.0001);
    positionSlider.setValue(0.0);
    positionSlider.addListener(this);
    addAndMakeVisible(positionSlider);

    // progress bar
    addAndMakeVisible(progressBar);
    progressBar.setPercentageDisplay(true);

    speedSlider.setRange(0.5, 2.0, 0.01);
    speedSlider.setValue(1.0);
    speedSlider.setTextValueSuffix("x");
    speedSlider.setNumDecimalPlacesToDisplay(2);
    speedSlider.addListener(this);
    addAndMakeVisible(speedSlider);

    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    addAndMakeVisible(timeLabel);

    //AB controls
    setMarkerAButton.addListener(this);
    setMarkerBButton.addListener(this);
    abLoopButton.addListener(this);
    clearMarkersButton.addListener(this);
    addAndMakeVisible(setMarkerAButton);
    addAndMakeVisible(setMarkerBButton);
    addAndMakeVisible(abLoopButton);
    addAndMakeVisible(clearMarkersButton);

    abMarkersLabel.setText("A-B: Not set", juce::dontSendNotification);
    abMarkersLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    abMarkersLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(abMarkersLabel);

    startTimer(100);
}

int PlayerGui::getNumRows()
{
    if (playerAudio == nullptr)
        return 0;

    return playerAudio->playlist.size();
}

PlayerGui::~PlayerGui()
{
}

void PlayerGui::paintListBoxItem(int row, juce::Graphics& g,
    int width, int height, bool rowIsSelected)
{
    if (playerAudio == nullptr || row >= playerAudio->playlist.size())
        return;

    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);

    g.setColour(juce::Colours::white);
    g.drawText(playerAudio->playlist[row].getFileName(),
        4, 0, width, height, juce::Justification::centredLeft);
}

void PlayerGui::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
}

void PlayerGui::resized()
{
    int y = 20;

    loadButton.setBounds(20, y, 80, 40);
    restartButton.setBounds(120, y, 80, 40);
    stopButton.setBounds(220, y, 80, 40);
    pauseButton.setBounds(320, y, 80, 40);
    playButton.setBounds(420, y, 80, 40);
    goToEndButton.setBounds(520, y, 80, 40);
    goToStartButton.setBounds(620, y, 80, 40);
    loopButton.setBounds(720, y, 80, 40);
    muteButton.setBounds(820, y, 80, 40);

    volumeSlider.setBounds(20, 100, getWidth() - 40, 30);
    progressBar.setBounds(20, 140, getWidth() - 40, 20);
    positionSlider.setBounds(20, 170, getWidth() - 40, 30);
    speedSlider.setBounds(20, 210, getWidth() - 40, 30);
    timeLabel.setBounds(20, 250, getWidth() - 40, 30);

    AddButton.setBounds(20, 290, 120, 30);
    PlaylistBox.setBounds(20, 320, getWidth() - 40, getHeight() - 420);

    fileInfoLabel.setBounds(20, 480, getWidth() - 40, 160);

    int y2 = 70;
    setMarkerAButton.setBounds(20, y2, 80, 30);
    setMarkerBButton.setBounds(120, y2, 80, 30);
    abLoopButton.setBounds(220, y2, 80, 30);
    clearMarkersButton.setBounds(320, y2, 80, 30);

    abMarkersLabel.setBounds(20 + 210, 250, getWidth() - 40 - 210, 30);
}

void PlayerGui::buttonClicked(juce::Button* button)
{
    if (playerAudio == NULL) return;

    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    playerAudio->loadFile(file);
                    fileInfoLabel.setText(playerAudio->getMetadataInfo(), juce::dontSendNotification);
                }
            });
    }
    else if (button == &restartButton)
    {
        playerAudio->restart();
    }
    else if (button == &stopButton)
    {
        playerAudio->stop();
    }
    else if (button == &pauseButton)
    {
        playerAudio->pause();
    }
    else if (button == &playButton)
    {
        playerAudio->play();
    }
    else if (button == &goToEndButton)
    {
        playerAudio->goToEnd();
    }
    else if (button == &goToStartButton) {
        playerAudio->goToStart();
    }
    else if (button == &loopButton)
    {
        playerAudio->loop();
    }
    else if (button == &muteButton)
    {
        playerAudio->setGain(0.0f, true);
    }
    else if (button == &setMarkerAButton)
    {
        playerAudio->setMarkerA();
    }
    else if (button == &setMarkerBButton)
    {
        playerAudio->setMarkerB();
    }
    else if (button == &abLoopButton)
    {
        playerAudio->toggleABLoop();
    }
    else if (button == &clearMarkersButton)
    {
        playerAudio->clearMarkers();
    }
    else if (button == &AddButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select audio files...",
            juce::File{},
            "*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectMultipleItems,
            [this](const juce::FileChooser& fc)
            {
                auto files = fc.getResults();
                playerAudio->addtoPlaylist(files);
                PlaylistBox.updateContent();
                PlaylistBox.repaint();
            });
    }
}
void PlayerGui::selectedRowsChanged(int row)
{
    if (playerAudio != nullptr && row >= 0 && row < playerAudio->playlist.size())
    {
        playerAudio->loadFromPlaylist(row);
        fileInfoLabel.setText(playerAudio->getMetadataInfo(), juce::dontSendNotification);
        PlaylistBox.repaint();
    }
}

void PlayerGui::sliderValueChanged(juce::Slider* slider) {
    if (slider == &volumeSlider && playerAudio != NULL) {
        playerAudio->setGain((float)volumeSlider.getValue());
    }
    else if (slider == &positionSlider && playerAudio != NULL) {
        if (isDraggingSlider) {
            playerAudio->setPositionNormalized(positionSlider.getValue());
        }
    }
    else if (slider == &speedSlider && playerAudio != NULL)
    {
        playerAudio->setSpeed(speedSlider.getValue());
    }
}

void PlayerGui::sliderDragStarted(juce::Slider* slider) {
    if (slider == &positionSlider) {
        isDraggingSlider = true;
    }
}

void PlayerGui::sliderDragEnded(juce::Slider* slider) {
    if (slider == &positionSlider) {
        isDraggingSlider = false;
    }
}

void PlayerGui::timerCallback() {
    if (playerAudio != NULL && !isDraggingSlider) {
        positionSlider.setValue(playerAudio->getPositionNormalized());
        progressValue = playerAudio->getPositionNormalized();

        double currentTime = playerAudio->getPosition();
        double totalTime = playerAudio->getLength();

        juce::String timeText = formatTime(currentTime) + " / " + formatTime(totalTime);
        timeLabel.setText(timeText, juce::dontSendNotification);

        double a = playerAudio->getMarkerA();
        double b = playerAudio->getMarkerB();
        bool active = playerAudio->isABLoopActive();
        juce::String abText = "A-B: ";
        if (a >= 0 && b >= 0 && b > a)
        {
            abText += "A=" + formatTime(a) + "  B=" + formatTime(b);
            abText += active ? "  LOOPING" : "  OFF";
        }
        else
        {
            abText += "Not set";
        }
        abMarkersLabel.setText(abText, juce::dontSendNotification);
    }
}

juce::String PlayerGui::formatTime(double seconds) {
    int minutes = (int)(seconds / 60);
    int secs = (int)(seconds) % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}
