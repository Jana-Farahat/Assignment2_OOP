#include "PlayerGui.h"
#include "BinaryData.h"

int ListModel::getNumRows() {
    if (listMode == ListMode::Markers) {
        return playerAudio->getMarkerCount() + 1;
    }
    else {
        return playerAudio->playlist.size() + 1;
    }
}

void ListModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) {
    (void)rowIsSelected;
    if (row == 0) {
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(0, 0, width, height);
        g.setColour(juce::Colours::lightgrey);
        juce::Font headerFont(juce::FontOptions().withHeight(18.0f));
        headerFont.setBold(true);
        g.setFont(headerFont);
        
        if (listMode == ListMode::Playlist) {
            int trackColWidth = (int)(width * 0.4f);
            int durationColWidth = (int)(width * 0.25f);
            g.drawText("Track", 4, 0, trackColWidth, height, juce::Justification::centredLeft);
            g.drawText("Duration", trackColWidth + 4, 0, durationColWidth, height, juce::Justification::centredLeft);
        }
        else {
            int markerColWidth = (int)(width * 0.5f);
            g.drawText("Markers", 4, 0, markerColWidth, height, juce::Justification::centredLeft);
        }
    }
}

juce::Component* ListModel::refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) {
    (void)isRowSelected;

    if (rowNumber == 0) {
        if (existingComponentToUpdate != nullptr)
            delete existingComponentToUpdate;
        return nullptr;
    }
    
    int dataRowNumber = rowNumber - 1;
    int maxRows = (listMode == ListMode::Markers) ? playerAudio->getMarkerCount() : playerAudio->playlist.size();
    if (dataRowNumber < 0 || dataRowNumber >= maxRows) {
        if (existingComponentToUpdate != nullptr)
            existingComponentToUpdate->setVisible(false);
        return existingComponentToUpdate;
    }

    auto createComponent = [this](int row) {
        std::function<void()> callback;
        if (listMode == ListMode::Markers) {
            callback = [this]() { playerGui->updateMarkersList(); };
        }
        else {
            callback = [this]() { playerGui->updatePlaylist(); };
        }
        return new ListRowComponent(playerAudio, playerGui, row, listMode, callback);
    };

    if (auto* rowComp = dynamic_cast<ListRowComponent*>(existingComponentToUpdate)) {
        rowComp->updateIndex(dataRowNumber);
        return existingComponentToUpdate;
    }

    if (existingComponentToUpdate != nullptr)
        delete existingComponentToUpdate;
    return createComponent(dataRowNumber);
}

void ListModel::selectedRowsChanged(int row) {
    (void)row;
}

void ListRowComponent::buttonClicked(juce::Button* button) {
    if (playerAudio == nullptr)
        return;
    if (button == &actionButton) {
        if (rowMode == ListMode::Markers) {
            playerAudio->jumpToMarker(index);
        }
        else {
            playerAudio->loadFromPlaylist(index);
            if (playerGui != nullptr)
                playerGui->updateMetadata();
        }
    }
    else if (button == &removeButton) {
        if (rowMode == ListMode::Markers) {
            playerAudio->removeTrackMarker(index);
        }
        else {
            playerAudio->playlist.remove(index);
        }
        if (updateCallback)
            updateCallback();
    }
}


PlayerGui::PlayerGui() {

    setupIconButton(&playButton, loadIconFromBinary(BinaryData::play_png, BinaryData::play_pngSize));
    setupIconButton(&pauseButton, loadIconFromBinary(BinaryData::pause_png, BinaryData::pause_pngSize));
    setupIconButton(&forward10sButton, loadIconFromBinary(BinaryData::_10forward_png, BinaryData::_10forward_pngSize));
    setupIconButton(&backward10sButton, loadIconFromBinary(BinaryData::_10backward_png, BinaryData::_10backward_pngSize));
    setupIconButton(&goToStartButton, loadIconFromBinary(BinaryData::start_png, BinaryData::start_pngSize));
    setupIconButton(&goToEndButton, loadIconFromBinary(BinaryData::end_png, BinaryData::end_pngSize));
    setupIconButton(&restartButton, loadIconFromBinary(BinaryData::restart_png, BinaryData::restart_pngSize));
    setupIconButton(&stopButton, loadIconFromBinary(BinaryData::stop_png, BinaryData::stop_pngSize));
    setupIconButton(&loopButton, loadIconFromBinary(BinaryData::loop_png, BinaryData::loop_pngSize));
    setupIconButton(&muteButton, loadIconFromBinary(BinaryData::unmuted_png, BinaryData::unmuted_pngSize));
    setupIconButton(&loadButton, loadIconFromBinary(BinaryData::folder_png, BinaryData::folder_pngSize));
    setupIconButton(&setMarkerButton, loadIconFromBinary(BinaryData::markerflag_png, BinaryData::markerflag_pngSize));
  
    for (auto* btn : { &loadButton, &restartButton , &stopButton, &pauseButton , &playButton, &goToEndButton, &loopButton, &muteButton, &goToStartButton, &forward10sButton, &backward10sButton, &setMarkerButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    setupIconButton(&loadFilesButton, loadIconFromBinary(BinaryData::folder_png, BinaryData::folder_pngSize));
    loadFilesButton.addListener(this);
    loadFilesButton.setEnabled(true);
    loadFilesButton.setVisible(true);
    addAndMakeVisible(&loadFilesButton);
    addAndMakeVisible(PlaylistBox);

    fileInfoLabel.setText("Currently playing:\nNo file loaded", juce::dontSendNotification);
    fileInfoLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    fileInfoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fileInfoLabel);

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    positionSlider.setRange(0.0, 1.0, 0.0001);
    positionSlider.setValue(0.0);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    positionSlider.addListener(this);
    addAndMakeVisible(positionSlider);

    addAndMakeVisible(progressBar);
    progressBar.setPercentageDisplay(true);

    speedSlider.setRange(0.5, 2.0, 0.01);
    speedSlider.setValue(1.0);
    speedSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    speedSlider.addListener(this);
    addAndMakeVisible(speedSlider);
    
    speedLabel.setText("1.00x", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    speedLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(speedLabel);

    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    addAndMakeVisible(timeLabel);

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

    setMarkerButton.setEnabled(true);
    setMarkerButton.setVisible(true);
    addAndMakeVisible(&setMarkerButton);
    addAndMakeVisible(markersListBox);

    startTimer(100);

    addAndMakeVisible(forward10sButton);
    addAndMakeVisible(backward10sButton);

    forward10sButton.addListener(this);
    backward10sButton.addListener(this);

}

PlayerGui::~PlayerGui()
{
}


void PlayerGui::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
}

void PlayerGui::resized()
{
    const int buttonSize = 32;
    const int muteButtonSize = 28;
    const int margin = 20;
    
    progressBar.setBounds(margin, 10, getWidth() - 2 * margin, 20);
    
    int buttonAreaStart = (int)(getWidth() * 0.15f);
    int buttonAreaWidth = (int)(getWidth() * 0.70f);
    int buttonY = 60;
    
    int availableWidth = buttonAreaWidth - (9 * buttonSize);
    int spacing = availableWidth / 8;
    
    int currentX = buttonAreaStart;
    
    stopButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    currentX += buttonSize + spacing;
    
    restartButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    currentX += buttonSize + spacing;
    
    goToStartButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    currentX += buttonSize + spacing;
    
    backward10sButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    currentX += buttonSize + spacing;
    
    playButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    pauseButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    currentX += buttonSize + spacing;
    
    forward10sButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    currentX += buttonSize + spacing;
    
    goToEndButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    currentX += buttonSize + spacing;
    
    loopButton.setBounds(currentX, buttonY, buttonSize, buttonSize);
    
    int sliderY = buttonY + buttonSize + 25;
    positionSlider.setBounds(margin, sliderY, getWidth() - 2 * margin, 30);
    
    int controlRowY = sliderY + 40;
    int rowHeight = 25;
    int controlSpacing = 10;
    
    int leftX = margin;
    setMarkerAButton.setBounds(leftX, controlRowY, 60, rowHeight);
    leftX += 65;
    setMarkerBButton.setBounds(leftX, controlRowY, 60, rowHeight);
    leftX += 65;
    abLoopButton.setBounds(leftX, controlRowY, 80, rowHeight);
    leftX += 85;
    clearMarkersButton.setBounds(leftX, controlRowY, 80, rowHeight);
    leftX += 85;
    abMarkersLabel.setBounds(leftX, controlRowY, 200, rowHeight);
    
    int timeLabelWidth = 150;
    int timeLabelX = (getWidth() - timeLabelWidth) / 2 + 20;
    timeLabel.setBounds(timeLabelX, controlRowY, timeLabelWidth, rowHeight);
    
    int rightX = getWidth() - margin;
    int speedSliderWidth = 200;
    int speedSliderHeight = 20;
    int speedLabelWidth = 60;
    int volumeSliderWidth = 200;
    int volumeSliderHeight = 20;
    
    rightX -= muteButtonSize;
    muteButton.setBounds(rightX, controlRowY + (rowHeight - muteButtonSize) / 2, muteButtonSize, muteButtonSize);
    rightX -= controlSpacing;
    
    rightX -= volumeSliderWidth;
    volumeSlider.setBounds(rightX, controlRowY + 2, volumeSliderWidth, volumeSliderHeight);
    rightX -= controlSpacing;
    
    rightX -= speedLabelWidth;
    speedLabel.setBounds(rightX, controlRowY, speedLabelWidth, rowHeight);
    rightX -= 5;
    
    rightX -= speedSliderWidth;
    speedSlider.setBounds(rightX, controlRowY + 2, speedSliderWidth, speedSliderHeight);
    
    int listBottomMargin = 20;
    int listHeight = 300;
    int listY = getHeight() - listHeight - listBottomMargin;
    
    int buttonsY = listY - buttonSize - 15;
    loadFilesButton.setBounds(margin, buttonsY, buttonSize, buttonSize);
    setMarkerButton.setBounds(getWidth() - margin - buttonSize, buttonsY, buttonSize, buttonSize);
    
    loadFilesButton.toFront(false);
    setMarkerButton.toFront(false);
    
    int metadataSpacing = 20;
    int metadataHeight = 100;
    int metadataY = buttonsY - metadataSpacing - metadataHeight + 50;
    fileInfoLabel.setBounds(margin, metadataY, getWidth() - 2 * margin, metadataHeight);
    int totalListWidth = getWidth() - 2 * margin;
    int playlistWidth = (int)(totalListWidth * 0.75f);
    int markersWidth = totalListWidth - playlistWidth;
    
    PlaylistBox.setBounds(margin, listY, playlistWidth, listHeight);
    markersListBox.setBounds(margin + playlistWidth, listY, markersWidth, listHeight);
}

void PlayerGui::buttonClicked(juce::Button* button)
{
    if (playerAudio == NULL)
        return;

    if (button == &loadButton) {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile()) {
                    playerAudio->loadFile(file);
                    updateMetadata();
                }
            });
    }
    else if (button == &restartButton) {
        playerAudio->restart();
    }
    else if (button == &stopButton) {
        playerAudio->stop();
    }
    else if (button == &pauseButton) {
        playerAudio->pause();
    }
    else if (button == &playButton) {
        playerAudio->play();
    }
    else if (button == &goToEndButton) {
        playerAudio->goToEnd();
    }
    else if (button == &goToStartButton) {
        playerAudio->goToStart();
    }
    else if (button == &loopButton) {
        playerAudio->loop();
    }
    else if (button == &muteButton) {
        playerAudio->setGain(0.0f, true);
    }
    else if (button == &setMarkerAButton) {
        playerAudio->setMarkerA();
    }
    else if (button == &setMarkerBButton) {
        playerAudio->setMarkerB();
    }
    else if (button == &abLoopButton) {
        playerAudio->toggleABLoop();
    }
    else if (button == &clearMarkersButton) {
        playerAudio->clearMarkers();
    }
    else if (button == &setMarkerButton) {
        playerAudio->addTrackMarker();
        updateMarkersList();
    }
    else if (button == &loadFilesButton) {
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
                updatePlaylist();
            });
    }

    else if (button == &forward10sButton) {
        playerAudio->tenSec(true);
    }
    else if (button == &backward10sButton) {
        playerAudio->tenSec(false);
    }

}

void PlayerGui::sliderValueChanged(juce::Slider* slider) {
    if (slider == &volumeSlider && playerAudio != NULL) {
        playerAudio->setGain((float)volumeSlider.getValue());
    }
    else if (slider == &positionSlider && playerAudio != NULL) {
        if (isDraggingSlider)
            playerAudio->setPositionNormalized(positionSlider.getValue());
    }
    else if (slider == &speedSlider && playerAudio != NULL) {
        playerAudio->setSpeed(speedSlider.getValue());
        markersListBox.repaint();
    }
}

void PlayerGui::sliderDragStarted(juce::Slider* slider) {
    if (slider == &positionSlider)
        isDraggingSlider = true;
}

void PlayerGui::sliderDragEnded(juce::Slider* slider) {
    if (slider == &positionSlider)
        isDraggingSlider = false;
}

void PlayerGui::timerCallback() {
    if (playerAudio != NULL && !isDraggingSlider) {
        positionSlider.setValue(playerAudio->getPositionNormalized());
        progressValue = playerAudio->getPositionNormalized();

        double currentTime = playerAudio->getPosition();
        double totalTime = playerAudio->getLength();

        juce::String timeText = formatTime(currentTime) + " / " + formatTime(totalTime);
        timeLabel.setText(timeText, juce::dontSendNotification);

        double a = playerAudio->getMarkerATime();
        double b = playerAudio->getMarkerBTime();
        bool active = playerAudio->isABLoopActive();
        juce::String abText = "A-B: ";
        if (a >= 0 && b >= 0 && b > a) {
            abText += "A=" + formatTime(a) + "  B=" + formatTime(b);
            abText += active ? "  LOOPING" : "  OFF";
        }
        else {
            abText += "Not set";
        }
        abMarkersLabel.setText(abText, juce::dontSendNotification);

        double speed = speedSlider.getValue();
        speedLabel.setText(juce::String::formatted("%.2fx", speed), juce::dontSendNotification);

        updateMuteButtonIcon();

        bool isPlaying = playerAudio->isPlaying();
        playButton.setVisible(!isPlaying);
        pauseButton.setVisible(isPlaying);

        markersListBox.repaint();
    }
}

juce::String PlayerGui::formatTime(double seconds) {
    int minutes = (int)(seconds / 60);
    int secs = (int)(seconds) % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}

juce::Image PlayerGui::loadIconFromBinary(const void* data, size_t dataSize) {
    if (data != nullptr && dataSize > 0)
        return juce::ImageFileFormat::loadFrom(data, dataSize);
    return juce::Image();
}

void PlayerGui::setupIconButton(juce::ImageButton* button, const juce::Image& icon) {
    if (icon.isValid()) {
        button->setImages(false, true, true,
            icon, 1.0f, juce::Colours::transparentBlack,
            icon, 1.0f, juce::Colour(0x77ffffff),
            icon, 1.0f, juce::Colour(0xffffffff),
            0.0f);
    }
}

void PlayerGui::updateMuteButtonIcon() {
    bool muted = playerAudio->getIsMuted();
    juce::Image icon = muted ? loadIconFromBinary(BinaryData::muted_png, BinaryData::muted_pngSize) 
                             : loadIconFromBinary(BinaryData::unmuted_png, BinaryData::unmuted_pngSize);
    if (icon.isValid())
        setupIconButton(&muteButton, icon);
}

void PlayerGui::loadSessionState(const juce::String& filePath, float volume)
{
    (void)filePath;
    volumeSlider.setValue(volume, juce::dontSendNotification);
    updateMetadata();
}