
#include "PlayerGui.h"
PlayerGui::PlayerGui() {
    for (auto* btn : { &loadButton, &restartButton , &stopButton, &pauseButton , &playButton, &go_to_end_Button })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);
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
    restartButton.setBounds(110, y, 80, 40);
    stopButton.setBounds(210, y, 80, 40);
    pauseButton.setBounds(300, y, 80, 40);
    playButton.setBounds(410, y, 80, 40);
    go_to_end_Button.setBounds(510, y, 80, 40);
    

    volumeSlider.setBounds(20, 100, getWidth() - 40, 30);
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
    else if (button == &go_to_end_Button)
    {
        playerAudio->goToEnd();
    }

}

void PlayerGui::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider && playerAudio != nullptr)
    {
        playerAudio->setGain((float)volumeSlider.getValue());
    }
}