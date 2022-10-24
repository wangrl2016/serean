//
// Created by wangrl2016 on 2022/10/24.
//

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MainContentComponent : public juce::AudioAppComponent {
public:
    MainContentComponent() {
        addAndMakeVisible(openButton);
        openButton.setButtonText("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible(clearButton);
        clearButton.setButtonText("Clear");
        clearButton.onClick = [this] { clearButtonClicked(); };

        addAndMakeVisible(levelSlider);
        levelSlider.setRange(0.0, 1.0);
        levelSlider.onValueChange = [this] { currentLevel = (float) levelSlider.getValue(); };

        setSize(300, 200);

        formatManager.registerBasicFormats();
    }

    ~MainContentComponent() override {
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {

    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {

    }

    void releaseResources() override {
        fileBuffer.setSize(0, 0);
    }

    void resized() override {
        openButton.setBounds(10, 10, getWidth() - 20, 20);
        clearButton.setBounds(10, 40, getWidth() - 20, 20);
        levelSlider.setBounds(10, 70, getWidth() - 20, 20);
    }

private:
    void openButtonClicked() {

    }

    void clearButtonClicked() {
        shutdownAudio();
    }

private:
    juce::TextButton openButton;
    juce::TextButton clearButton;
    juce::Slider levelSlider;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    juce::AudioSampleBuffer fileBuffer;
    int position = 0;

    float currentLevel = 0.0f, previousLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

class Application : public juce::JUCEApplication {
public:
    Application() = default;

    const juce::String getApplicationName() override { return {}; }

    const juce::String getApplicationVersion() override { return ""; }

    void initialise(const juce::String&) override {
        main_window.reset(new MainWindow("AudioSampleBuffer", new
                MainContentComponent, *this));
    }

    void shutdown() override {
        main_window = nullptr;
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

private:
    std::unique_ptr<MainWindow> main_window;
};


START_JUCE_APPLICATION (Application)
