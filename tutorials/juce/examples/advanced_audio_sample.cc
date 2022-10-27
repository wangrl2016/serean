//
// Created by wangrl2016 on 2022/10/26.
//

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MainContentComponent : public juce::AudioAppComponent, private juce::Thread {
public:
    class ReferenceCountedBuffer : public juce::ReferenceCountedObject {
    public:
        typedef juce::ReferenceCountedObjectPtr<ReferenceCountedBuffer> Ptr;

        ReferenceCountedBuffer(const juce::String& nameToUse,
                               int numChannels,
                               int numSamples) :
                name(nameToUse),
                buffer(numChannels, numSamples) {
            DBG (juce::String("Buffer named '") + name + "' constructed. numChannels = "
                 + juce::String(numChannels) + ", numSamples = " + juce::String(numSamples));
        }

        ~ReferenceCountedBuffer() {
            DBG (juce::String("Buffer named '") + name + "' destroyed");
        }

        juce::AudioSampleBuffer* getAudioSampleBuffer() {
            return &buffer;
        }

        int position = 0;

    private:
        juce::String name;
        juce::AudioSampleBuffer buffer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedBuffer)
    };

    MainContentComponent() : Thread("Background Thread") {
        addAndMakeVisible(openButton);
        openButton.setButtonText("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible(clearButton);
        clearButton.setButtonText("Clear");
        clearButton.onClick = [this] { clearButtonClicked(); };

        setSize(300, 200);

        formatManager.registerBasicFormats();
        setAudioChannels(0, 2);

        startThread();
    }

    ~MainContentComponent() override {
        stopThread(4000);
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {

    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        auto retainedCurrentBuffer = [&]() -> ReferenceCountedBuffer::Ptr {
           const juce::SpinLock::ScopedTryLockType lock(mutex);

           if (lock.isLocked())
               return currentBuffer;

           return nullptr;
        }();

        if (retainedCurrentBuffer == nullptr) {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        auto* currentAudioSampleBuffer = retainedCurrentBuffer->getAudioSampleBuffer();
        auto position = retainedCurrentBuffer->position;

        auto numInputChannels = currentAudioSampleBuffer->getNumChannels();
        auto numOutputChannels = bufferToFill.buffer->getNumChannels();

        auto outputSamplesRemaining = bufferToFill.numSamples;
        auto outputSamplesOffset = 0;

        while (outputSamplesRemaining > 0) {
            
        }
    }

    void releaseResources() override {
    }

    void resized() override {
    }

private:
    void run() override {

    }

    void openButtonClicked() {

    }

    void clearButtonClicked() {
        shutdownAudio();
    }

private:

    juce::TextButton openButton;
    juce::TextButton clearButton;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    juce::ReferenceCountedArray<ReferenceCountedBuffer> buffers;

    juce::SpinLock mutex;
    ReferenceCountedBuffer::Ptr currentBuffer;
    juce::CriticalSection pathMutex;
    juce::String chosenPath;

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
