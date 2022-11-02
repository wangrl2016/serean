//
// Created by wangrl2016 on 2022/10/20.
//

#include <memory>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

// 代码位置: https://github.com/wangrl2016/serean.git
// 1. AudioSampleBuffer
// 2. AudioDeviceManager
// 3. Waveform
// 4. MidiSynthesiser

class MainContentComponent : public juce::AudioAppComponent,
                             public juce::ChangeListener {
public:
    MainContentComponent()
            : state(Stopped) {
        addAndMakeVisible(&openButton);
        openButton.setButtonText("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible(&playButton);
        playButton.setButtonText("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        playButton.setEnabled(false);

        addAndMakeVisible(&stopButton);
        stopButton.setButtonText("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        stopButton.setEnabled(false);

        setSize(600, 500);

        formatManager.registerBasicFormats();       // [1]
        transportSource.addChangeListener(this);   // [2]

        setAudioChannels(0, 2);
    }

    ~MainContentComponent() override {
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        if (readerSource.get() == nullptr) {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock(bufferToFill);
    }

    void releaseResources() override {
        transportSource.releaseResources();
    }

    void resized() override {
        openButton.setBounds(10, 10, getWidth() - 20, 60);
        playButton.setBounds(10, 70, getWidth() - 20, 60);
        stopButton.setBounds(10, 130, getWidth() - 20, 60);
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override {
        if (source == &transportSource) {
            if (transportSource.isPlaying())
                changeState(Playing);
            else
                changeState(Stopped);
        }
    }

private:
    enum TransportState {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState(TransportState newState) {
        if (state != newState) {
            state = newState;

            switch (state) {
                case Stopped:                           // [3]
                    stopButton.setEnabled(false);
                    playButton.setEnabled(true);
                    transportSource.setPosition(0.0);
                    break;

                case Starting:                          // [4]
                    playButton.setEnabled(false);
                    transportSource.start();
                    break;

                case Playing:                           // [5]
                    stopButton.setEnabled(true);
                    break;

                case Stopping:                          // [6]
                    transportSource.stop();
                    break;
            }
        }
    }

    void openButtonClicked() {
        chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
                                                      juce::File{},
                                                      "*.wav");                     // [7]
        auto chooserFlags = juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)           // [8]
        {
            auto file = fc.getResult();

            if (file != juce::File{})                                                      // [9]
            {
                auto* reader = formatManager.createReaderFor(file);                 // [10]

                if (reader != nullptr) {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);   // [11]
                    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
                    playButton.setEnabled(true);                                                      // [13]
                    readerSource.reset(newSource.release());                                          // [14]
                }
            }
        });
    }

    void playButtonClicked() {
        changeState(Starting);
    }

    void stopButtonClicked() {
        changeState(Stopping);
    }

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

class Application : public juce::JUCEApplication {
public:
    Application() = default;

    const juce::String getApplicationName() override { return {}; }

    const juce::String getApplicationVersion() override { return {}; }

    void initialise(const juce::String&) override {
        window.reset(new MainWindow("AudioPlayer", new MainContentComponent, *this));
    }

    void shutdown() override {
        window = nullptr;
    }

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
            setResizeLimits(600, 500, 10000, 10000);
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

    std::unique_ptr<MainWindow> window;
};

START_JUCE_APPLICATION (Application)
