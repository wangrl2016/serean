//
// Created by wangrl2016 on 2022/10/26.
//

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MainContentComponent : public juce::AudioAppComponent,
                             public juce::ChangeListener,
                             private juce::Timer {
public:
    MainContentComponent()
            : audioSetupComp(deviceManager,
                             0,     // minimum input channels
                             256,   // maximum input channels
                             0,     // minimum output channels
                             256,   // maximum output channels
                             false, // ability to select midi inputs
                             false, // ability to select midi output device
                             false, // treat channels as stereo pairs
                             false) // hide advanced options
    {
        addAndMakeVisible(audioSetupComp);
        addAndMakeVisible(diagnosticsBox);

        diagnosticsBox.setMultiLine(true);
        diagnosticsBox.setReturnKeyStartsNewLine(true);
        diagnosticsBox.setReadOnly(true);
        diagnosticsBox.setScrollbarsShown(true);
        diagnosticsBox.setCaretVisible(false);
        diagnosticsBox.setPopupMenuEnabled(true);
        diagnosticsBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
        diagnosticsBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
        diagnosticsBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));

        cpuUsageLabel.setText("CPU Usage", juce::dontSendNotification);
        cpuUsageText.setJustificationType(juce::Justification::right);
        addAndMakeVisible(&cpuUsageLabel);
        addAndMakeVisible(&cpuUsageText);

        setSize(760, 360);

        setAudioChannels(2, 2);
        deviceManager.addChangeListener(this);

        startTimer(50);
    }

    ~MainContentComponent() override {
        deviceManager.removeChangeListener(this);
        shutdownAudio();
    }

    void prepareToPlay(int, double) override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        auto* device = deviceManager.getCurrentAudioDevice();

        auto activeInputChannels = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();

        auto maxInputChannels = activeInputChannels.countNumberOfSetBits();
        auto maxOutputChannels = activeOutputChannels.countNumberOfSetBits();

        for (auto channel = 0; channel < maxOutputChannels; ++channel) {
            if ((!activeOutputChannels[channel]) || maxInputChannels == 0) {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            } else {
                auto actualInputChannel = channel % maxInputChannels;

                if (!activeInputChannels[channel]) {
                    bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
                } else {
                    auto* inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannel,
                                                                         bufferToFill.startSample);
                    auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

                    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                        outBuffer[sample] = inBuffer[sample] * random.nextFloat() * 0.25f;
                }
            }
        }
    }

    void releaseResources() override {}

    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colours::grey);
        g.fillRect(getLocalBounds().removeFromRight(proportionOfWidth(0.4f)));
    }

    void resized() override {
        auto rect = getLocalBounds();

        audioSetupComp.setBounds(rect.removeFromLeft(proportionOfWidth(0.6f)));
        rect.reduce(10, 10);

        auto topLine(rect.removeFromTop(20));
        cpuUsageLabel.setBounds(topLine.removeFromLeft(topLine.getWidth() / 2));
        cpuUsageText.setBounds(topLine);
        rect.removeFromTop(20);

        diagnosticsBox.setBounds(rect);
    }

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override {
        dumpDeviceInfo();
    }

    static juce::String getListOfActiveBits(const juce::BigInteger& b) {
        juce::StringArray bits;

        for (auto i = 0; i <= b.getHighestBit(); ++i)
            if (b[i])
                bits.add(juce::String(i));

        return bits.joinIntoString(", ");
    }

    void timerCallback() override {
        auto cpu = deviceManager.getCpuUsage() * 100;
        cpuUsageText.setText(juce::String(cpu, 6) + " %", juce::dontSendNotification);
    }

    void dumpDeviceInfo() {
        logMessage("Current audio device type: " + (deviceManager.getCurrentDeviceTypeObject() != nullptr
                                                    ? deviceManager.getCurrentDeviceTypeObject()->getTypeName()
                                                    : "<none>"));

        if (auto* device = deviceManager.getCurrentAudioDevice()) {
            logMessage("Current audio device: " + device->getName().quoted());
            logMessage("Sample rate: " + juce::String(device->getCurrentSampleRate()) + " Hz");
            logMessage("Block size: " + juce::String(device->getCurrentBufferSizeSamples()) + " samples");
            logMessage("Bit depth: " + juce::String(device->getCurrentBitDepth()));
            logMessage("Input channel names: " + device->getInputChannelNames().joinIntoString(", "));
            logMessage("Active input channels: " + getListOfActiveBits(device->getActiveInputChannels()));
            logMessage("Output channel names: " + device->getOutputChannelNames().joinIntoString(", "));
            logMessage("Active output channels: " + getListOfActiveBits(device->getActiveOutputChannels()));
        } else {
            logMessage("No audio device open");
        }
    }

    void logMessage(const juce::String& m) {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret(m + juce::newLine);
    }

    juce::Random random;
    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::Label cpuUsageLabel;
    juce::Label cpuUsageText;
    juce::TextEditor diagnosticsBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


class Application : public juce::JUCEApplication {
public:
    Application() = default;

    const juce::String getApplicationName() override { return "AudioDeviceManagerTutorial"; }

    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise(const juce::String&) override {
        mainWindow.reset(new MainWindow("AudioDeviceManagerTutorial", new MainContentComponent, *this));
    }

    void shutdown() override { mainWindow = nullptr; }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(const juce::String& name, juce::Component* c, JUCEApplication& a)
                : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel()
                                         .findColour(ResizableWindow::backgroundColourId),
                                 juce::DocumentWindow::allButtons),
                  app(a) {
            setUsingNativeTitleBar(true);
            setContentOwned(c, true);

            setResizable(true, false);
            setResizeLimits(300, 250, 10000, 10000);
            centreWithSize(getWidth(), getHeight());

            setVisible(true);
        }

        void closeButtonPressed() override {
            app.systemRequestedQuit();
        }

    private:
        JUCEApplication& app;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (Application)
