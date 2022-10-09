//
// Created by wangrl2016 on 2022/10/9.
//

#include "audio/source/oscillator.h"

namespace cs {
    void Oscillator::RenderAudio(float* audioData, int32_t numFrames) {
        if (is_wave_on_){
            for (int i = 0; i < numFrames; ++i) {

                // Sine wave (sinf)
                audioData[i] = sinf(phase_) * amplitude_;

                // Square wave
                // if (mPhase <= kPi){
                //     audioData[i] = -mAmplitude;
                // } else {
                //     audioData[i] = mAmplitude;
                // }

                phase_ += float(phase_increment_);
                if (phase_ > kTwoPi) phase_ -= kTwoPi;
            }
        } else {
            memset(audioData, 0, sizeof(float) * numFrames);
        }
    }
}