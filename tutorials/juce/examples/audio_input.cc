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
        auto max_input_channels = active_input_channels.getHighestBit() + 1;
        auto max_output_channels = active_output_channels.getHighestBit() + 1;

        auto level = (float) level_slider_.getValue();
        for (auto channel = 0; channel < max_output_channels; channel++) {
            if ((!active_output_channels[channel]) || max_input_channels == 0) {
                bufferToFill.buffer->clear(channel,
                                           bufferToFill.startSample,
                                           bufferToFill.numSamples);
            } else {
                auto actual_input_channel = channel % max_input_channels;
                if (!active_input_channels[channel]) {
                    bufferToFill.buffer->clear(channel,
                                               bufferToFill.startSample,
                                               bufferToFill.numSamples);
                } else {
                    auto* in_buffer = bufferToFill.buffer->getReadPointer(actual_input_channel,
                                                                          bufferToFill.startSample);
                    auto* out_buffer = bufferToFill.buffer->getWritePointer(channel,
                                                                            bufferToFill.startSample);

                    for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {
                        auto noise = (random_.nextFloat() * 2.0f) - 1.0f;
                        out_buffer[sample] = in_buffer[sample] + (in_buffer[sample] * noise * level);
                    }
                }
            }
        }
    }

    void releaseResources() override {

    }

    void resized() override {
        level_label_.setBounds (10, 10, 90, 20);
        level_slider_.setBounds (100, 10, getWidth() - 110, 20);
    }

private:
    juce::Random random_;
    juce::Slider level_slider_;
    juce::Label level_label_;

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
