//
// Created by wangrl2016 on 2022/10/20.
//

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MainContentComponent : public juce::AudioAppComponent {
public:
    MainContentComponent() {
        level_slider_.setRange(0.0, 0.05);
        level_slider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
        level_label_.setText("Noise Level", juce::dontSendNotification);

        addAndMakeVisible(level_slider_);
        addAndMakeVisible(level_label_);

        setSize(600, 100);
        setAudioChannels(2, 2);
    }

    ~MainContentComponent() override {
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {

    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override {
        auto* device = deviceManager.getCurrentAudioDevice();
        auto active_input_channels = device->getActiveInputChannels();
        auto active_output_channels = device->getActiveOutputChannels();
        
    }

private:
    juce::Random random_;
    juce::Slider level_slider_;
    juce::Label level_label_;
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

    std::unique_ptr<MainWindow> window;
};

START_JUCE_APPLICATION (Application)
