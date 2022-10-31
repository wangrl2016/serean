//
// Created by wangrl2016 on 2022/10/26.
//

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

class SimpleThumbnailComponent : public juce::Component,
                                 private juce::ChangeListener {
public:
    SimpleThumbnailComponent(int sourceSamplesPerThumbnailSample,
                             juce::AudioFormatManager& formatManager,
                             juce::AudioThumbnailCache& cache)
            : thumbnail(sourceSamplesPerThumbnailSample, formatManager, cache) {
        thumbnail.addChangeListener(this);
    }

    void setFile(const juce::File& file) {
        thumbnail.setSource(new juce::FileInputSource(file));
    }

    void paint(juce::Graphics& g) override {
        if (thumbnail.getNumChannels() == 0)
            paintIfNoFileLoaded(g);
        else
            paintIfFileLoaded(g);
    }

    void paintIfNoFileLoaded(juce::Graphics& g) {
        g.fillAll(juce::Colours::white);
        g.setColour(juce::Colours::darkgrey);
        g.drawFittedText("No File Loaded", getLocalBounds(), juce::Justification::centred, 1);
    }

    void paintIfFileLoaded(juce::Graphics& g) {
        g.fillAll(juce::Colours::white);

        g.setColour(juce::Colours::red);
        thumbnail.drawChannels(g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 1.0f);
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override {
        if (source == &thumbnail)
            thumbnailChanged();
    }

private:
    void thumbnailChanged() {
        repaint();
    }

    juce::AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleThumbnailComponent)
};

class SimplePositionOverlay : public juce::Component,
                              private juce::Timer {
public:
    SimplePositionOverlay(juce::AudioTransportSource& transportSourceToUse)
            : transportSource(transportSourceToUse) {
        startTimer(40);
    }

    void paint(juce::Graphics& g) override {
        auto duration = (float) transportSource.getLengthInSeconds();

        if (duration > 0.0) {
            auto audioPosition = (float) transportSource.getCurrentPosition();
            auto drawPosition = (audioPosition / duration) * (float) getWidth();

            g.setColour(juce::Colours::green);
            g.drawLine(drawPosition, 0.0f, drawPosition, (float) getHeight(), 2.0f);
        }
    }

    void mouseDown(const juce::MouseEvent& event) override {
        auto duration = transportSource.getLengthInSeconds();

        if (duration > 0.0) {
            auto clickPosition = event.position.x;
            auto audioPosition = (clickPosition / (float) getWidth()) * duration;

            transportSource.setPosition(audioPosition);
        }
    }

private:
    void timerCallback() override {
        repaint();
    }

    juce::AudioTransportSource& transportSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimplePositionOverlay)
};

class MainContentComponent : public juce::AudioAppComponent,
                             public juce::ChangeListener {
public:
    MainContentComponent()
            : state(Stopped),
              thumbnailCache(5),
              thumbnailComp(512, formatManager, thumbnailCache),
              positionOverlay(transportSource) {
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

        addAndMakeVisible(&thumbnailComp);
        addAndMakeVisible(&positionOverlay);

        setSize(600, 400);

        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);

        setAudioChannels(2, 2);
    }

    ~MainContentComponent() override {
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        if (readerSource.get() == nullptr)
            bufferToFill.clearActiveBufferRegion();
        else
            transportSource.getNextAudioBlock(bufferToFill);
    }

    void releaseResources() override {
        transportSource.releaseResources();
    }

    void resized() override {
        openButton.setBounds(10, 10, getWidth() - 20, 20);
        playButton.setBounds(10, 40, getWidth() - 20, 20);
        stopButton.setBounds(10, 70, getWidth() - 20, 20);

        juce::Rectangle<int> thumbnailBounds(10, 100, getWidth() - 20, getHeight() - 120);
        thumbnailComp.setBounds(thumbnailBounds);
        positionOverlay.setBounds(thumbnailBounds);
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override {
        if (source == &transportSource)
            transportSourceChanged();
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
                case Stopped:
                    stopButton.setEnabled(false);
                    playButton.setEnabled(true);
                    transportSource.setPosition(0.0);
                    break;

                case Starting:
                    playButton.setEnabled(false);
                    transportSource.start();
                    break;

                case Playing:
                    stopButton.setEnabled(true);
                    break;

                case Stopping:
                    transportSource.stop();
                    break;

                default:
                    jassertfalse;
                    break;
            }
        }
    }

    void transportSourceChanged() {
        if (transportSource.isPlaying())
            changeState(Playing);
        else
            changeState(Stopped);
    }

    void openButtonClicked() {
        chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
                                                      juce::File{},
                                                      "*.wav");
        auto chooserFlags = juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();

            if (file != juce::File{}) {
                auto* reader = formatManager.createReaderFor(file);

                if (reader != nullptr) {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                    playButton.setEnabled(true);
                    thumbnailComp.setFile(file);
                    readerSource.reset(newSource.release());
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
    juce::AudioThumbnailCache thumbnailCache;
    SimpleThumbnailComponent thumbnailComp;
    SimplePositionOverlay positionOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

class Application : public juce::JUCEApplication {
public:
    Application() = default;

    const juce::String getApplicationName() override { return ""; }

    const juce::String getApplicationVersion() override { return ""; }

    void initialise(const juce::String&) override {
        mainWindow.reset(new MainWindow("AudioThumbnailTutorial", new MainContentComponent, *this));
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