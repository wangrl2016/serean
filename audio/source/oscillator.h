//
// Created by wangrl2016 on 2022/10/9.
//

#ifndef SEREAN_OSCILLATOR_H
#define SEREAN_OSCILLATOR_H

#include <cmath>
#include <cstdint>
#include <cstring>
#include <atomic>

namespace cs {
    constexpr double kDefaultFrequency = 440.0;
    constexpr int32_t kDefaultSampleRate = 44100;
    constexpr double kPi = M_PI;
    constexpr double kTwoPi = kPi * 2;

    class Oscillator {
    public:
        ~Oscillator() = default;

        void set_wave_on(bool isWaveOn) {
            is_wave_on_.store(isWaveOn);
        };

        void set_sample_rate(int32_t sampleRate) {
            sample_rate_ = sampleRate;
            updatePhaseIncrement();
        };

        void set_frequency(double frequency) {
            frequency_ = frequency;
            updatePhaseIncrement();
        };

        inline void set_amplitude(float amplitude) {
            amplitude_ = amplitude;
        };

        void RenderAudio(float *audioData, int32_t numFrames);

    private:
        std::atomic<bool> is_wave_on_ {false };
        float phase_ = 0.0;
        std::atomic<float> amplitude_ {1.0 };
        std::atomic<double> phase_increment_ {0.0 };
        double frequency_ = kDefaultFrequency;
        int32_t sample_rate_ = kDefaultSampleRate;

        void updatePhaseIncrement(){
            phase_increment_.store((kTwoPi * frequency_) / static_cast<double>(sample_rate_));
        };
    };
}

#endif //SEREAN_OSCILLATOR_H
