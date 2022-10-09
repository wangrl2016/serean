//
// Created by admin on 2022/10/9.
//

#ifndef SEREAN_SYNTH_SOUND_H
#define SEREAN_SYNTH_SOUND_H

#include <array>
#include <atomic>
#include <cmath>

constexpr float kDefaultFrequency = 440.0;
constexpr int32_t kDefaultSampleRate = 48000;
constexpr float kPi = M_PI;
constexpr float kTwoPi = kPi * 2;
constexpr int32_t kNumSineWaves = 5;
constexpr float kSustainMultiplier = 0.99999;
constexpr float kReleaseMultiplier = 0.999;
// Stop playing music below this cutoff
constexpr float kMasterAmplitudeCutOff = 0.01;

namespace cs {
    class SynthSound {
    public:
        SynthSound() = default;

        ~SynthSound() = default;

        void NoteOn() {
            trigger_ = true;    // start a note envelope
            amplitude_scaler_ = kSustainMultiplier;
        }

        void NoteOff() {
            amplitude_scaler_ = kReleaseMultiplier;
        }

        void set_sample_rate(int32_t sample_rate) {
            sample_rate_ = sample_rate;
            UpdatePhaseIncrement();
        }

        void set_frequency(float frequency) {
            frequency_ = frequency;
            UpdatePhaseIncrement();
        }

        // Amplitudes from https://epubs.siam.org/doi/pdf/10.1137/S00361445003822
        inline void setAmplitude(float amplitude) {
            amplitudes_[0] = amplitude * 0.2f;
            amplitudes_[1] = amplitude;
            amplitudes_[2] = amplitude * 0.1f;
            amplitudes_[3] = amplitude * 0.02f;
            amplitudes_[4] = amplitude * 0.15f;
        };

        void RenderAudio(float* audio_data, int32_t num_frames) {
            for (int i = 0; i < num_frames; i++) {
                if (trigger_.exchange(false)) {
                    master_amplitude_ = 1.0;
                    phase_ = 0.0f;
                } else {
                    master_amplitude_ *= amplitude_scaler_;
                }

                audio_data[i] = 0;
                if (master_amplitude_ < kMasterAmplitudeCutOff)
                    continue;

                for (int j = 0; j < kNumSineWaves; j++) {
                    audio_data[i] += sinf(phase_ * float(j + 1)) * amplitudes_[j] * master_amplitude_;
                }

                phase_ += phase_increment_;
                if (phase_ > kTwoPi)
                    phase_ -= kTwoPi;
            }
        }

    private:
        void UpdatePhaseIncrement(){
            // Note how there is a division here. If this file is changed so that updatePhaseIncrement
            // is called more frequently, please cache 1/mSampleRate. This allows this operation to not
            // need divisions.
            phase_increment_ = kTwoPi * frequency_ / static_cast<float>(sample_rate_);
        };

    private:
        std::atomic<bool> trigger_;
        float master_amplitude_ = 0.0f;
        std::atomic<float> amplitude_scaler_;
        std::array<std::atomic<float>, kNumSineWaves> amplitudes_;
        float phase_ = 0.0f;
        std::atomic<float> phase_increment_;
        std::atomic<float> frequency_{kDefaultFrequency};
        std::atomic<int32_t> sample_rate_{kDefaultSampleRate};
    };
}

#endif //SEREAN_SYNTH_SOUND_H
