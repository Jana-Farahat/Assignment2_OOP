#include "PlayerGui.h"
#include "BinaryData.h"




void ListRowComponent::paint(juce::Graphics& g) {
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
        int hours = (int)(markerTime / 3600);
        int minutes = (int)((markerTime - hours * 3600) / 60);
        int secs = (int)(markerTime) % 60;
        juce::String markerText = juce::String::formatted("Marker %d", index + 1);
        juce::String timeText = juce::String::formatted("%02d:%02d:%02d", hours, minutes, secs);

        int buttonArea = 110;
        int timeColWidth = 80;
        int markerColWidth = getWidth() - buttonArea - timeColWidth - 8;

        g.drawText(markerText, 4, 0, markerColWidth, getHeight(), juce::Justification::centredLeft);
        g.drawText(timeText, markerColWidth + 12, 0, timeColWidth, getHeight(), juce::Justification::centredLeft);
    }
    else {
        // FIX: Always use playerAudioLeft's playlist for display
        PlayerAudio* targetAudio = playerAudio;
        if (playerGui != nullptr && playerGui->playerAudioLeft != nullptr) {
            targetAudio = playerGui->playerAudioLeft;
        }

        if (targetAudio == nullptr) return;
        if (index >= targetAudio->playlist.size())
            return;
        juce::File file = targetAudio->playlist[index];
        juce::String fileName = file.getFileName();

        juce::String durationText = "--:--";
        juce::String currentPath = targetAudio->getCurrentSongPath();
        if (currentPath.isNotEmpty() && file.getFullPathName() == currentPath) {
            double duration = targetAudio->getLength();
            if (duration > 0) {
                int hours = (int)(duration / 3600);
                int minutes = (int)((duration - hours * 3600) / 60);
                int secs = (int)(duration) % 60;
                durationText = juce::String::formatted("%02d:%02d:%02d", hours, minutes, secs);
            }
        }
        else {
            juce::AudioFormatManager formatManager;
            formatManager.registerBasicFormats();
            std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
            if (reader != nullptr) {
                double duration = reader->lengthInSamples / reader->sampleRate;
                if (duration > 0) {
                    int hours = (int)(duration / 3600);
                    int minutes = (int)((duration - hours * 3600) / 60);
                    int secs = (int)(duration) % 60;
                    durationText = juce::String::formatted("%02d:%02d:%02d", hours, minutes, secs);
                }
            }
        }

        int trackColWidth = (int)(getWidth() * 0.4f);
        int durationColWidth = (int)(getWidth() * 0.25f);
        g.drawText(fileName, 4, 0, trackColWidth, getHeight(), juce::Justification::centredLeft);
        g.drawText(durationText, trackColWidth + 4, 0, durationColWidth, getHeight(), juce::Justification::centredLeft);
        g.setColour(juce::Colours::grey);
        g.drawLine((float)(trackColWidth + 2), 0.0f, (float)(trackColWidth + 2), (float)getHeight(), 1.0f);
        g.drawLine((float)(trackColWidth + durationColWidth + 2), 0.0f, (float)(trackColWidth + durationColWidth + 2), (float)getHeight(), 1.0f);
    }
}

int ListModel::getNumRows() {
    if (listMode == ListMode::Markers) {
        return playerAudio->getMarkerCount() + 1;
    }
    else {
        PlayerAudio* targetAudio = playerAudio;
        if (playerGui != nullptr)
            targetAudio = playerGui->playerAudioLeft;
        return targetAudio->playlist.size() + 1;
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
            g.drawText("Duration (HH:MM:SS)", trackColWidth + 4, 0, durationColWidth, height, juce::Justification::centredLeft);
            g.setColour(juce::Colours::grey);
            g.drawLine((float)(trackColWidth + 2), 0.0f, (float)(trackColWidth + 2), (float)height, 1.0f);
            g.drawLine((float)(trackColWidth + durationColWidth + 2), 0.0f, (float)(trackColWidth + durationColWidth + 2), (float)height, 1.0f);
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
    int maxRows;
    if (listMode == ListMode::Markers) {
        maxRows = playerAudio->getMarkerCount();
    }
    else {
        PlayerAudio* targetAudio = playerAudio;
        if (playerGui != nullptr)
            targetAudio = playerGui->playerAudioLeft;
            maxRows = targetAudio->playlist.size();
    }
    if (dataRowNumber < 0 || dataRowNumber >= maxRows) {
        if (existingComponentToUpdate != nullptr)
            existingComponentToUpdate->setVisible(false);
        return existingComponentToUpdate;
    }

    auto createComponent = [this](int row) {
        std::function<void()> callback;
        if (listMode == ListMode::Markers) {
            if (playerAudio == playerGui->playerAudioLeft)
                callback = [this]() { playerGui->updateMarkersListLeft(); };
            else
                callback = [this]() { playerGui->updateMarkersListRight(); };
        }
        else {
            callback = [this]() { playerGui->updatePlaylist(); };
        }
        PlayerAudio* targetAudioRight = playerAudioRight;
        if (listMode == ListMode::Markers && playerAudio == playerGui->playerAudioLeft)
            targetAudioRight = nullptr;
        return new ListRowComponent(playerAudio, targetAudioRight, playerGui, row, listMode, callback);
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
    if (button == &actionButton) {
        if (rowMode == ListMode::Markers) {
            if (playerAudio != nullptr)
                playerAudio->jumpToMarker(index);
        }
    }
    else if (button == &playLeftButton) {
        if (playerGui != nullptr && playerGui->playerAudioLeft != nullptr) {
            if (index >= 0 && index < playerGui->playerAudioLeft->playlist.size()) {
                juce::File file = playerGui->playerAudioLeft->playlist[index];
                playerGui->playerAudioLeft->loadFile(file);
                playerGui->updateMetadataLeft();
            }
        }
    }
    else if (button == &playRightButton) {
        if (playerGui != nullptr && playerGui->playerAudioRight != nullptr && playerGui->playerAudioLeft != nullptr) {
            if (index >= 0 && index < playerGui->playerAudioLeft->playlist.size()) {
                juce::File file = playerGui->playerAudioLeft->playlist[index];
                playerGui->playerAudioRight->loadFile(file);
                playerGui->updateMetadataRight();
            }
        }
    }
    else if (button == &removeButton) {
        if (rowMode == ListMode::Markers) {
            if (playerAudio != nullptr)
                playerAudio->removeTrackMarker(index);
        }
        else {
            if (playerGui != nullptr && playerGui->playerAudioLeft != nullptr)
                playerGui->playerAudioLeft->playlist.remove(index);
        }
        if (updateCallback)
            updateCallback();
    }
    
    
}


PlayerGui::PlayerGui() {

    setupIconButton(&playButtonLeft, loadIconFromBinary(BinaryData::play_png, BinaryData::play_pngSize));
    setupIconButton(&pauseButtonLeft, loadIconFromBinary(BinaryData::pause_png, BinaryData::pause_pngSize));
    setupIconButton(&forward10sButtonLeft, loadIconFromBinary(BinaryData::_10forward_png, BinaryData::_10forward_pngSize));
    setupIconButton(&backward10sButtonLeft, loadIconFromBinary(BinaryData::_10backward_png, BinaryData::_10backward_pngSize));
    setupIconButton(&goToStartButtonLeft, loadIconFromBinary(BinaryData::start_png, BinaryData::start_pngSize));
    setupIconButton(&goToEndButtonLeft, loadIconFromBinary(BinaryData::end_png, BinaryData::end_pngSize));
    setupIconButton(&restartButtonLeft, loadIconFromBinary(BinaryData::restart_png, BinaryData::restart_pngSize));
    setupIconButton(&stopButtonLeft, loadIconFromBinary(BinaryData::stop_png, BinaryData::stop_pngSize));
    setupIconButton(&loopButtonLeft, loadIconFromBinary(BinaryData::loop_png, BinaryData::loop_pngSize));
    setupIconButton(&muteButtonLeft, loadIconFromBinary(BinaryData::unmuted_png, BinaryData::unmuted_pngSize));
    setupIconButton(&loadButtonLeft, loadIconFromBinary(BinaryData::folder_png, BinaryData::folder_pngSize));
    setupIconButton(&setMarkerButtonLeft, loadIconFromBinary(BinaryData::markerflag_png, BinaryData::markerflag_pngSize));

    for (auto* btn : { &loadButtonLeft, &restartButtonLeft, &stopButtonLeft, &pauseButtonLeft, &playButtonLeft, &goToEndButtonLeft, &loopButtonLeft, &muteButtonLeft, &goToStartButtonLeft, &forward10sButtonLeft, &backward10sButtonLeft, &setMarkerButtonLeft })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    setupIconButton(&playButtonRight, loadIconFromBinary(BinaryData::play_png, BinaryData::play_pngSize));
    setupIconButton(&pauseButtonRight, loadIconFromBinary(BinaryData::pause_png, BinaryData::pause_pngSize));
    setupIconButton(&forward10sButtonRight, loadIconFromBinary(BinaryData::_10forward_png, BinaryData::_10forward_pngSize));
    setupIconButton(&backward10sButtonRight, loadIconFromBinary(BinaryData::_10backward_png, BinaryData::_10backward_pngSize));
    setupIconButton(&goToStartButtonRight, loadIconFromBinary(BinaryData::start_png, BinaryData::start_pngSize));
    setupIconButton(&goToEndButtonRight, loadIconFromBinary(BinaryData::end_png, BinaryData::end_pngSize));
    setupIconButton(&restartButtonRight, loadIconFromBinary(BinaryData::restart_png, BinaryData::restart_pngSize));
    setupIconButton(&stopButtonRight, loadIconFromBinary(BinaryData::stop_png, BinaryData::stop_pngSize));
    setupIconButton(&loopButtonRight, loadIconFromBinary(BinaryData::loop_png, BinaryData::loop_pngSize));
    setupIconButton(&muteButtonRight, loadIconFromBinary(BinaryData::unmuted_png, BinaryData::unmuted_pngSize));
    setupIconButton(&loadButtonRight, loadIconFromBinary(BinaryData::folder_png, BinaryData::folder_pngSize));
    setupIconButton(&setMarkerButtonRight, loadIconFromBinary(BinaryData::markerflag_png, BinaryData::markerflag_pngSize));

    for (auto* btn : { &loadButtonRight, &restartButtonRight, &stopButtonRight, &pauseButtonRight, &playButtonRight, &goToEndButtonRight, &loopButtonRight, &muteButtonRight, &goToStartButtonRight, &forward10sButtonRight, &backward10sButtonRight, &setMarkerButtonRight })
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
    addAndMakeVisible(markersListBoxLeft);
    addAndMakeVisible(markersListBoxRight);


    //new innovative
    resetLeftButton.addListener(this);
    resetRightButton.addListener(this);

    resetLeftButton.setEnabled(true);
    resetRightButton.setEnabled(true);

    resetLeftButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    resetLeftButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    resetRightButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    resetRightButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    resetLeftButton.setLookAndFeel(&myButtonLookAndFeel);
    resetRightButton.setLookAndFeel(&myButtonLookAndFeel);

    addAndMakeVisible(&resetLeftButton);
    addAndMakeVisible(&resetRightButton);



    fileInfoLabelLeft.setText("Currently playing:\nNo file loaded", juce::dontSendNotification);
    fileInfoLabelLeft.setColour(juce::Label::textColourId, juce::Colours::white);
    fileInfoLabelLeft.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fileInfoLabelLeft);


    fileInfoLabelRight.setText("Currently playing:\nNo file loaded", juce::dontSendNotification);
    fileInfoLabelRight.setColour(juce::Label::textColourId, juce::Colours::white);
    fileInfoLabelRight.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fileInfoLabelRight);

    volumeSliderLeft.setRange(0.0, 1.0, 0.01);
    volumeSliderLeft.setValue(1.0);
    volumeSliderLeft.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSliderLeft.addListener(this);
    addAndMakeVisible(volumeSliderLeft);

    positionSliderLeft.setRange(0.0, 1.0, 0.0001);
    positionSliderLeft.setValue(0.0);
    positionSliderLeft.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    positionSliderLeft.addListener(this);
    addAndMakeVisible(positionSliderLeft);

    addAndMakeVisible(progressBarLeft);
    progressBarLeft.setPercentageDisplay(true);

    speedSliderLeft.setRange(0.5, 2.0, 0.01);
    speedSliderLeft.setValue(1.0);
    speedSliderLeft.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    speedSliderLeft.addListener(this);
    addAndMakeVisible(speedSliderLeft);

    speedLabelLeft.setText("1.00x", juce::dontSendNotification);
    speedLabelLeft.setColour(juce::Label::textColourId, juce::Colours::white);
    speedLabelLeft.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(speedLabelLeft);

    timeLabelLeft.setText("00:00:00 / 00:00:00", juce::dontSendNotification);
    addAndMakeVisible(timeLabelLeft);

    setMarkerAButtonLeft.addListener(this);
    setMarkerBButtonLeft.addListener(this);
    abLoopButtonLeft.addListener(this);
    clearMarkersButtonLeft.addListener(this);
    addAndMakeVisible(setMarkerAButtonLeft);
    addAndMakeVisible(setMarkerBButtonLeft);
    addAndMakeVisible(abLoopButtonLeft);
    addAndMakeVisible(clearMarkersButtonLeft);

    abMarkersLabelLeft.setText("A-B: Not set", juce::dontSendNotification);
    abMarkersLabelLeft.setColour(juce::Label::textColourId, juce::Colours::white);
    abMarkersLabelLeft.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(abMarkersLabelLeft);

    setMarkerButtonLeft.setEnabled(true);
    setMarkerButtonLeft.setVisible(true);
    addAndMakeVisible(&setMarkerButtonLeft);

    volumeSliderRight.setRange(0.0, 1.0, 0.01);
    volumeSliderRight.setValue(1.0);
    volumeSliderRight.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSliderRight.addListener(this);
    addAndMakeVisible(volumeSliderRight);

    positionSliderRight.setRange(0.0, 1.0, 0.0001);
    positionSliderRight.setValue(0.0);
    positionSliderRight.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    positionSliderRight.addListener(this);
    addAndMakeVisible(positionSliderRight);

    addAndMakeVisible(progressBarRight);
    progressBarRight.setPercentageDisplay(true);

    speedSliderRight.setRange(0.5, 2.0, 0.01);
    speedSliderRight.setValue(1.0);
    speedSliderRight.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    speedSliderRight.addListener(this);
    addAndMakeVisible(speedSliderRight);

    speedLabelRight.setText("1.00x", juce::dontSendNotification);
    speedLabelRight.setColour(juce::Label::textColourId, juce::Colours::white);
    speedLabelRight.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(speedLabelRight);

    timeLabelRight.setText("00:00:00 / 00:00:00", juce::dontSendNotification);
    addAndMakeVisible(timeLabelRight);

    setMarkerAButtonRight.addListener(this);
    setMarkerBButtonRight.addListener(this);
    abLoopButtonRight.addListener(this);
    clearMarkersButtonRight.addListener(this);
    addAndMakeVisible(setMarkerAButtonRight);
    addAndMakeVisible(setMarkerBButtonRight);
    addAndMakeVisible(abLoopButtonRight);
    addAndMakeVisible(clearMarkersButtonRight);

    abMarkersLabelRight.setText("A-B: Not set", juce::dontSendNotification);
    abMarkersLabelRight.setColour(juce::Label::textColourId, juce::Colours::white);
    abMarkersLabelRight.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(abMarkersLabelRight);

    setMarkerButtonRight.setEnabled(true);
    setMarkerButtonRight.setVisible(true);
    addAndMakeVisible(&setMarkerButtonRight);

    startTimer(100);

}

PlayerGui::~PlayerGui()
{
    
}


void PlayerGui::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::darkgrey);
    int dividerX = getWidth() / 2;
    g.setColour(juce::Colours::grey);
    g.drawLine((float)dividerX, 0.0f, (float)dividerX, (float)getHeight(), 2.0f);
}

void PlayerGui::resized()
{
    const int buttonSize = 24;
    const int muteButtonSize = 20;
    const int margin = 15;
    const int playerWidth = (getWidth() - 3 * margin) / 2;
    const int divider = margin + playerWidth;

    int leftStartX = margin;
    int rightStartX = divider + margin;

    progressBarLeft.setBounds(leftStartX, 10, playerWidth - margin, 18);
    progressBarRight.setBounds(rightStartX, 10, playerWidth - margin, 18);

    int buttonAreaStart = (int)(playerWidth * 0.1f) + 20;
    int buttonAreaWidth = (int)(playerWidth * 0.8f) - 40;
    int buttonY = 50;

    int availableWidth = buttonAreaWidth - (9 * buttonSize);
    int spacing = availableWidth / 10;

    int currentXLeft = leftStartX + buttonAreaStart;
    int currentXRight = rightStartX + buttonAreaStart;

    stopButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    currentXLeft += buttonSize + spacing;
    stopButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);
    currentXRight += buttonSize + spacing;

    restartButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    currentXLeft += buttonSize + spacing;
    restartButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);
    currentXRight += buttonSize + spacing;

    goToStartButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    currentXLeft += buttonSize + spacing;
    goToStartButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);
    currentXRight += buttonSize + spacing;

    backward10sButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    currentXLeft += buttonSize + spacing;
    backward10sButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);
    currentXRight += buttonSize + spacing;

    playButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    pauseButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    currentXLeft += buttonSize + spacing;
    playButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);
    pauseButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);
    currentXRight += buttonSize + spacing;

    forward10sButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    currentXLeft += buttonSize + spacing;
    forward10sButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);
    currentXRight += buttonSize + spacing;

    goToEndButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    currentXLeft += buttonSize + spacing;
    goToEndButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);
    currentXRight += buttonSize + spacing;

    loopButtonLeft.setBounds(currentXLeft, buttonY, buttonSize, buttonSize);
    loopButtonRight.setBounds(currentXRight, buttonY, buttonSize, buttonSize);

    int sliderY = buttonY + buttonSize + 20;
    positionSliderLeft.setBounds(leftStartX, sliderY, playerWidth - margin, 25);
    positionSliderRight.setBounds(rightStartX, sliderY, playerWidth - margin, 25);

    int controlRowY = sliderY + 35;
    int rowHeight = 22;
    int controlSpacing = 8;
    int buttonHeight = 18;

    int timeLabelWidth = 150;
    int abLabelWidth = 150;
    int timeLabelXLeft = leftStartX + (playerWidth - margin - timeLabelWidth) / 2;
    timeLabelLeft.setBounds(timeLabelXLeft, controlRowY, timeLabelWidth, rowHeight);

    int rightXLeft = leftStartX + playerWidth - margin;
    int abButtonsY = controlRowY;
    setMarkerAButtonLeft.setBounds(rightXLeft - 50, abButtonsY, 50, buttonHeight);
    abButtonsY += buttonHeight + 2;
    setMarkerBButtonLeft.setBounds(rightXLeft - 50, abButtonsY, 50, buttonHeight);

    rightXLeft -= 75;
    abButtonsY = controlRowY;
    clearMarkersButtonLeft.setBounds(rightXLeft - 70, abButtonsY, 70, buttonHeight);
    abButtonsY += buttonHeight + 2;
    abLoopButtonLeft.setBounds(rightXLeft - 70, abButtonsY, 70, buttonHeight);

    rightXLeft -= 80;
    abMarkersLabelLeft.setBounds(rightXLeft - abLabelWidth, controlRowY, abLabelWidth, rowHeight);

    int speedSliderWidth = 150;
    int speedSliderHeight = 18;
    int speedLabelWidth = 50;
    int volumeSliderWidth = 150;
    int volumeSliderHeight = 18;

    int leftXSliders = leftStartX;
    muteButtonLeft.setBounds(leftXSliders, controlRowY + (rowHeight - muteButtonSize) / 2, muteButtonSize, muteButtonSize);
    leftXSliders += muteButtonSize + controlSpacing;
    volumeSliderLeft.setBounds(leftXSliders, controlRowY + 2, volumeSliderWidth, volumeSliderHeight);

    int volumeSpeedY = controlRowY + volumeSliderHeight + 4;
    leftXSliders = leftStartX;
    speedLabelLeft.setBounds(leftXSliders, volumeSpeedY, speedLabelWidth, rowHeight);
    leftXSliders += speedLabelWidth + 5;
    speedSliderLeft.setBounds(leftXSliders, volumeSpeedY + 2, speedSliderWidth, speedSliderHeight);

    int leftX = rightStartX;
    abButtonsY = controlRowY;
    setMarkerAButtonRight.setBounds(leftX, abButtonsY, 50, buttonHeight);
    leftX += 55;
    clearMarkersButtonRight.setBounds(leftX, abButtonsY, 70, buttonHeight);
    leftX = rightStartX;
    abButtonsY += buttonHeight + 2;
    setMarkerBButtonRight.setBounds(leftX, abButtonsY, 50, buttonHeight);
    leftX += 55;
    abLoopButtonRight.setBounds(leftX, abButtonsY, 70, buttonHeight);
    leftX += 75;
    abMarkersLabelRight.setBounds(leftX, controlRowY, 150, rowHeight);

    int timeLabelXRight = rightStartX + (playerWidth - margin - timeLabelWidth) / 2;
    timeLabelRight.setBounds(timeLabelXRight, controlRowY, timeLabelWidth, rowHeight);

    int rightX = rightStartX + playerWidth - margin;
    rightX -= muteButtonSize;
    muteButtonRight.setBounds(rightX, controlRowY + (rowHeight - muteButtonSize) / 2, muteButtonSize, muteButtonSize);
    rightX -= controlSpacing;
    rightX -= volumeSliderWidth;
    volumeSliderRight.setBounds(rightX, controlRowY + 2, volumeSliderWidth, volumeSliderHeight);

    volumeSpeedY = controlRowY + volumeSliderHeight + 4;
    rightX = rightStartX + playerWidth - margin;
    rightX -= muteButtonSize;
    rightX -= controlSpacing;
    rightX -= speedSliderWidth;
    speedSliderRight.setBounds(rightX, volumeSpeedY + 2, speedSliderWidth, speedSliderHeight);
    rightX += speedSliderWidth + 5;
    speedLabelRight.setBounds(rightX, volumeSpeedY, speedLabelWidth, rowHeight);

    int listBottomMargin = 15;
    int listHeight = 280;
    int listY = getHeight() - listHeight - listBottomMargin;

    int buttonsY = listY - buttonSize - 12;
    loadFilesButton.setBounds((getWidth() - buttonSize) / 2, buttonsY, buttonSize, buttonSize);

    resetLeftButton.setBounds(leftStartX + 50, buttonsY, 70, 30);  // innovative
    resetRightButton.setBounds(rightStartX + playerWidth - 120, buttonsY, 70, 30);

    setMarkerButtonLeft.setBounds(leftStartX, buttonsY, buttonSize, buttonSize);
    setMarkerButtonRight.setBounds(rightStartX + playerWidth - margin - buttonSize, buttonsY, buttonSize, buttonSize);

    loadFilesButton.toFront(false);
    setMarkerButtonLeft.toFront(false);
    setMarkerButtonRight.toFront(false);

    int metadataSpacing = 15;
    int metadataHeight = 95;
    int metadataY = buttonsY - metadataSpacing - metadataHeight + 40;
    fileInfoLabelLeft.setBounds(leftStartX, metadataY, playerWidth - margin + 10, metadataHeight);
    fileInfoLabelRight.setBounds(rightStartX, metadataY, playerWidth - margin + 10, metadataHeight);

    int totalListWidth = getWidth() - 2 * margin;
    int markersWidth = (int)(totalListWidth * 0.25f);
    int playlistWidth = totalListWidth - 2 * markersWidth;

    markersListBoxLeft.setBounds(margin, listY, markersWidth, listHeight);
    PlaylistBox.setBounds(margin + markersWidth, listY, playlistWidth, listHeight);
    markersListBoxRight.setBounds(margin + markersWidth + playlistWidth, listY, markersWidth, listHeight);
}

void PlayerGui::buttonClicked(juce::Button* button)
{
    if (button == &loadButtonLeft) {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile() && playerAudioLeft != nullptr) {
                    playerAudioLeft->loadFile(file);
                    updateMetadataLeft();
                }
            });
    }
    else if (button == &loadButtonRight) {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile() && playerAudioRight != nullptr) {
                    playerAudioRight->loadFile(file);
                    updateMetadataRight();
                }
            });
    }
    else if (button == &restartButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->restart();
    }
    else if (button == &restartButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->restart();
    }
    else if (button == &stopButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->stop();
    }
    else if (button == &stopButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->stop();
    }
    else if (button == &pauseButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->pause();
    }
    else if (button == &pauseButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->pause();
    }
    else if (button == &playButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->play();
    }
    else if (button == &playButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->play();
    }
    else if (button == &goToEndButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->goToEnd();
    }
    else if (button == &goToEndButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->goToEnd();
    }
    else if (button == &goToStartButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->goToStart();
    }
    else if (button == &goToStartButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->goToStart();
    }
    else if (button == &loopButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->loop();
    }
    else if (button == &loopButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->loop();
    }
    else if (button == &muteButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->setGain(0.0f, true);
    }
    else if (button == &muteButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->setGain(0.0f, true);
    }
    else if (button == &setMarkerAButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->setMarkerA();
    }
    else if (button == &setMarkerAButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->setMarkerA();
    }
    else if (button == &setMarkerBButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->setMarkerB();
    }
    else if (button == &setMarkerBButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->setMarkerB();
    }
    else if (button == &abLoopButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->toggleABLoop();
    }
    else if (button == &abLoopButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->toggleABLoop();
    }
    else if (button == &clearMarkersButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->clearMarkers();
    }
    else if (button == &clearMarkersButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->clearMarkers();
    }
    else if (button == &setMarkerButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->addTrackMarker();
        updateMarkersListLeft();
    }
    else if (button == &setMarkerButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->addTrackMarker();
        updateMarkersListRight();
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
                if (playerAudioLeft != nullptr) {
                    playerAudioLeft->addtoPlaylist(files);
                    updatePlaylist();
                }
            });
    }
    else if (button == &forward10sButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->tenSec(true);
    }
    else if (button == &forward10sButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->tenSec(true);
    }
    else if (button == &backward10sButtonLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->tenSec(false);
    }
    else if (button == &backward10sButtonRight && playerAudioRight != nullptr) {
        playerAudioRight->tenSec(false);
    }
     
    else if (button == &resetLeftButton) {
        DBG("RESET LEFT BUTTON CLICKED");
        resetLeftPlayer();
        }
    else if (button == &resetRightButton)
    {
        DBG("RESET RIGHT BUTTON CLICKED");
        resetRightPlayer();
        }
}

void PlayerGui::sliderValueChanged(juce::Slider* slider) {
    if (slider == &volumeSliderLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->setGain((float)volumeSliderLeft.getValue());
    }
    else if (slider == &volumeSliderRight && playerAudioRight != nullptr) {
        playerAudioRight->setGain((float)volumeSliderRight.getValue());
    }
    else if (slider == &positionSliderLeft && playerAudioLeft != nullptr) {
        if (isDraggingSliderLeft)
            playerAudioLeft->setPositionNormalized(positionSliderLeft.getValue());
    }
    else if (slider == &positionSliderRight && playerAudioRight != nullptr) {
        if (isDraggingSliderRight)
            playerAudioRight->setPositionNormalized(positionSliderRight.getValue());
    }
    else if (slider == &speedSliderLeft && playerAudioLeft != nullptr) {
        playerAudioLeft->setSpeed(speedSliderLeft.getValue());
        markersListBoxLeft.repaint();
    }
    else if (slider == &speedSliderRight && playerAudioRight != nullptr) {
        playerAudioRight->setSpeed(speedSliderRight.getValue());
        markersListBoxRight.repaint();
    }
    
}

void PlayerGui::sliderDragStarted(juce::Slider* slider) {
    if (slider == &positionSliderLeft)
        isDraggingSliderLeft = true;
    else if (slider == &positionSliderRight)
        isDraggingSliderRight = true;
}

void PlayerGui::sliderDragEnded(juce::Slider* slider) {
    if (slider == &positionSliderLeft)
        isDraggingSliderLeft = false;
    else if (slider == &positionSliderRight)
        isDraggingSliderRight = false;
}

void PlayerGui::timerCallback() {
    if (playerAudioLeft != nullptr && !isDraggingSliderLeft) {
        positionSliderLeft.setValue(playerAudioLeft->getPositionNormalized());
        progressValueLeft = playerAudioLeft->getPositionNormalized();

        double currentTime = playerAudioLeft->getPosition();
        double totalTime = playerAudioLeft->getLength();

        juce::String timeText = formatTime(currentTime) + " / " + formatTime(totalTime);
        timeLabelLeft.setText(timeText, juce::dontSendNotification);

        double a = playerAudioLeft->getMarkerATime();
        double b = playerAudioLeft->getMarkerBTime();
        bool active = playerAudioLeft->isABLoopActive();
        juce::String abText = "A-B: ";
        if (a >= 0 && b >= 0 && b > a) {
            abText += "A=" + formatTime(a) + "  B=" + formatTime(b);
            abText += active ? "  LOOPING" : "  OFF";
        }
        else {
            abText += "Not set";
        }
        abMarkersLabelLeft.setText(abText, juce::dontSendNotification);

        double speed = speedSliderLeft.getValue();
        speedLabelLeft.setText(juce::String::formatted("%.2fx", speed), juce::dontSendNotification);

        updateMuteButtonIcon();

        bool isPlaying = playerAudioLeft->isPlaying();
        playButtonLeft.setVisible(!isPlaying);
        pauseButtonLeft.setVisible(isPlaying);

        markersListBoxLeft.repaint();
    }

    if (playerAudioRight != nullptr && !isDraggingSliderRight) {
        positionSliderRight.setValue(playerAudioRight->getPositionNormalized());
        progressValueRight = playerAudioRight->getPositionNormalized();

        double currentTime = playerAudioRight->getPosition();
        double totalTime = playerAudioRight->getLength();

        juce::String timeText = formatTime(currentTime) + " / " + formatTime(totalTime);
        timeLabelRight.setText(timeText, juce::dontSendNotification);

        double a = playerAudioRight->getMarkerATime();
        double b = playerAudioRight->getMarkerBTime();
        bool active = playerAudioRight->isABLoopActive();
        juce::String abText = "A-B: ";
        if (a >= 0 && b >= 0 && b > a) {
            abText += "A=" + formatTime(a) + "  B=" + formatTime(b);
            abText += active ? "  LOOPING" : "  OFF";
        }
        else {
            abText += "Not set";
        }
        abMarkersLabelRight.setText(abText, juce::dontSendNotification);

        double speed = speedSliderRight.getValue();
        speedLabelRight.setText(juce::String::formatted("%.2fx", speed), juce::dontSendNotification);

        bool isPlaying = playerAudioRight->isPlaying();
        playButtonRight.setVisible(!isPlaying);
        pauseButtonRight.setVisible(isPlaying);

        markersListBoxRight.repaint();
    }
}

juce::String PlayerGui::formatTime(double seconds) {
    int hours = (int)(seconds / 3600);
    int minutes = (int)((seconds - hours * 3600) / 60);
    int secs = (int)(seconds) % 60;
    return juce::String::formatted("%02d:%02d:%02d", hours, minutes, secs);
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
    if (playerAudioLeft != nullptr) {
        bool muted = playerAudioLeft->getIsMuted();
        juce::Image icon = muted ? loadIconFromBinary(BinaryData::muted_png, BinaryData::muted_pngSize)
            : loadIconFromBinary(BinaryData::unmuted_png, BinaryData::unmuted_pngSize);
        if (icon.isValid())
            setupIconButton(&muteButtonLeft, icon);
    }
    if (playerAudioRight != nullptr) {
        bool muted = playerAudioRight->getIsMuted();
        juce::Image icon = muted ? loadIconFromBinary(BinaryData::muted_png, BinaryData::muted_pngSize)
            : loadIconFromBinary(BinaryData::unmuted_png, BinaryData::unmuted_pngSize);
        if (icon.isValid())
            setupIconButton(&muteButtonRight, icon);
    }
}

void PlayerGui::loadSessionState(const juce::String& filePath, float volume)
{
    (void)filePath;
    (void)volume;
}

void PlayerGui::restoreGUIFromSession()
{
    if (playerAudioLeft && playerAudioRight)
    {
        // FIX: Update everything
        positionSliderLeft.setValue(playerAudioLeft->getPositionNormalized(), juce::dontSendNotification);
        volumeSliderLeft.setValue(playerAudioLeft->getCurrentVolume(), juce::dontSendNotification);

        positionSliderRight.setValue(playerAudioRight->getPositionNormalized(), juce::dontSendNotification);
        volumeSliderRight.setValue(playerAudioRight->getCurrentVolume(), juce::dontSendNotification);

        
        speedSliderLeft.setValue(1.0, juce::dontSendNotification);
        speedSliderRight.setValue(1.0, juce::dontSendNotification);

        updateMetadataLeft();
        updateMetadataRight();
        updateMarkersListLeft();
        updateMarkersListRight();
        updatePlaylist();

        
        repaint();

        DBG("GUI restored from session");
    }
}

void PlayerGui::resetLeftPlayer()
{
    if (playerAudioLeft != nullptr)
    {
        
        playerAudioLeft->stop();
        playerAudioLeft->setPosition(0.0);

       
        positionSliderLeft.setValue(0.0);
        timeLabelLeft.setText("00:00:00 / 00:00:00", juce::dontSendNotification);

        
        playButtonLeft.setVisible(true);
        pauseButtonLeft.setVisible(false);

        DBG("Left player reset to zero");
    }
}

void PlayerGui::resetRightPlayer()
{
    if (playerAudioRight != nullptr)
    {
       
        playerAudioRight->stop();
        playerAudioRight->setPosition(0.0);

    
        positionSliderRight.setValue(0.0);
        timeLabelRight.setText("00:00:00 / 00:00:00", juce::dontSendNotification);

        playButtonRight.setVisible(true);
        pauseButtonRight.setVisible(false);

        DBG("Right player reset to zero");
    }
}