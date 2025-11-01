#include "PlayerGui.h"
PlayerGui::PlayerGui() {
    for (auto* btn : { &loadButton, &restartButton , &stopButton, &pauseButton , &playButton, &goToEndButton, &loopButton,&muteButton, &goToStartButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }
    
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

    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    addAndMakeVisible(timeLabel);
    startTimer(100);
}
PlayerGui::~PlayerGui()
{

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
    
    auto area = getLocalBounds();
    fileInfoLabel.setBounds(area.removeFromBottom(200));

    volumeSlider.setBounds(20, 100, getWidth() - 40, 30);

    positionSlider.setBounds(20, 150, getWidth() - 40, 30);
    timeLabel.setBounds(20, 180, getWidth() - 40, 30);
}

void PlayerGui::buttonClicked(juce::Button* button)
{
    if (playerAudio == nullptr) return;

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
}

void PlayerGui::sliderValueChanged(juce::Slider* slider) {
    if (slider == &volumeSlider && playerAudio != nullptr) {
        playerAudio->setGain((float)volumeSlider.getValue());
    }
    else if (slider == &positionSlider && playerAudio != nullptr) {
        if (isDraggingSlider) {
            playerAudio->setPositionNormalized(positionSlider.getValue());
        }
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
    if (playerAudio != nullptr && !isDraggingSlider) {
        // Update position slider
        positionSlider.setValue(playerAudio->getPositionNormalized());

        // Update time display
        double currentTime = playerAudio->getPosition();
        double totalTime = playerAudio->getLength();

        juce::String timeText = formatTime(currentTime) + " / " + formatTime(totalTime);
        timeLabel.setText(timeText, juce::dontSendNotification);
    }
}

juce::String PlayerGui::formatTime(double seconds) {
    int minutes = (int)(seconds / 60);
    int secs = (int)(seconds) % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}