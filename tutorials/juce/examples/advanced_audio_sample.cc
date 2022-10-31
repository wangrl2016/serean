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
            auto bufferSamplesRemaining = currentAudioSampleBuffer->getNumSamples() - position;
            auto samplesThisTime = juce::jmin(outputSamplesRemaining, bufferSamplesRemaining);

            for (auto channel = 0; channel < numOutputChannels; ++channel) {
                bufferToFill.buffer->copyFrom(channel,
                                              bufferToFill.startSample + outputSamplesOffset,
                                              *currentAudioSampleBuffer,
                                              channel % numInputChannels,
                                              position,
                                              samplesThisTime);
            }

            outputSamplesRemaining -= samplesThisTime;
            outputSamplesOffset += samplesThisTime;
            position += samplesThisTime;

            if (position == currentAudioSampleBuffer->getNumSamples())
                position = 0;
        }

        retainedCurrentBuffer->position = position;
    }

    void releaseResources() override {
        const juce::SpinLock::ScopedLockType lock(mutex);
        currentBuffer = nullptr;
    }

    void resized() override {
        openButton.setBounds(10, 10, getWidth() - 20, 20);
        clearButton.setBounds(10, 40, getWidth() - 20, 20);
    }

private:
    void run() override {
        while (!threadShouldExit()) {
            checkForPathToOpen();
            checkForBuffersToFree();
            wait(500);
        }
    }

    void checkForBuffersToFree() {
        for (auto i = buffers.size(); --i >= 0;) {
            ReferenceCountedBuffer::Ptr buffer(buffers.getUnchecked(i));

            if (buffer->getReferenceCount() == 2)
                buffers.remove(i);
        }
    }

    void checkForPathToOpen() {
        juce::String pathToOpen;
        {
            const juce::ScopedLock lock(pathMutex);
            pathToOpen.swapWith(chosenPath);
        }

        if (pathToOpen.isNotEmpty()) {
            juce::File file(pathToOpen);
            std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

            if (reader.get() != nullptr) {
                auto duration = (float) reader->lengthInSamples / reader->sampleRate;

                if (duration < 2) {
                    ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(file.getFileName(),
                                                                                       (int) reader->numChannels,
                                                                                       (int) reader->lengthInSamples);

                    reader->read(newBuffer->getAudioSampleBuffer(), 0, (int) reader->lengthInSamples, 0, true, true);

                    {
                        const juce::SpinLock::ScopedLockType lock(mutex);
                        currentBuffer = newBuffer;
                    }

                    buffers.add(newBuffer);
                } else {
                    // handle the error that the file is 2 seconds or longer..
                }
            }
        }
    }

    void openButtonClicked() {
        chooser = std::make_unique<juce::FileChooser>("Select a Wave file shorter than 2 seconds to play...",
                                                      juce::File{},
                                                      "*.wav");
        auto chooserFlags = juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();

            if (file == juce::File{})
                return;

            auto path = file.getFullPathName();


            {
                const juce::ScopedLock lock(pathMutex);
                chosenPath.swapWith(path);
            }

            notify();
        });
    }

    void clearButtonClicked() {
        const juce::SpinLock::ScopedLockType lock(mutex);
        currentBuffer = nullptr;
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
