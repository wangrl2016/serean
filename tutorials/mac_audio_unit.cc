//
// Created by wangrl2016 on 2022/9/22.
//

#ifdef __APPLE__

#include <AudioUnit/AudioUnit.h>
#include <thread>

#endif

#include "audio/source/oscillator.h"

// 节拍器
// 1. 波形生成器
// 正弦波：周期、振幅
// 2. 按照固定的周期播放波形
// 代码地址：https://github.com/wangrl2016/serean.git

constexpr int kDefaultSampleRate = 44100;
constexpr int kHighFrequency = 800;
constexpr int kLowFrequency = 400;
constexpr int kMaxFrameNum = 4096;

// 波形生成器
static cs::Oscillator oscillator;
static float* buffer = nullptr;
static int count = 0;

OSStatus RenderCallback([[maybe_unused]] void* in_ref_con,
                        [[maybe_unused]] AudioUnitRenderActionFlags* io_action_flags,
                        [[maybe_unused]] const AudioTimeStamp* in_time_stamp,
                        [[maybe_unused]] UInt32 in_bus_number,
                        UInt32 in_number_frames,
                        AudioBufferList* __nullable io_data) {
    if (count % 120 == 0) {
        oscillator.set_frequency(kHighFrequency);
        oscillator.RenderAudio(buffer, int32_t(in_number_frames));
    } else if (count % 30 == 0) {
        oscillator.set_frequency(kLowFrequency);
        oscillator.RenderAudio(buffer, int32_t(in_number_frames));
    } else {
        memset(buffer, 0, kMaxFrameNum * sizeof(float));
    }

    auto* out = (float*) io_data->mBuffers[0].mData;
    memcpy(out, buffer, in_number_frames * sizeof(float));

    count++;
    return noErr;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
#ifdef __APPLE__
    OSStatus status;
    AudioUnit audio_unit;

    AudioComponentDescription acd;
    acd.componentType = kAudioUnitType_Output;
    acd.componentSubType = kAudioUnitSubType_DefaultOutput;
    acd.componentFlags = 0;
    acd.componentFlagsMask = 0;
    acd.componentManufacturer = kAudioUnitManufacturer_Apple;

    AudioComponent component = AudioComponentFindNext(nullptr, &acd);
    status = AudioComponentInstanceNew(component, &audio_unit);
    if (status != noErr)
        return status;

    UInt32 max_frame_per_slice = kMaxFrameNum;
    status = AudioUnitSetProperty(audio_unit,
                                  kAudioUnitProperty_MaximumFramesPerSlice,
                                  kAudioUnitScope_Global,
                                  0,
                                  &max_frame_per_slice,
                                  sizeof(UInt32));
    if (status != noErr)
        return status;

    // 设置输入流的属性
    AudioStreamBasicDescription description;
    memset(&description, 0, sizeof(description));
    description.mSampleRate = kDefaultSampleRate;
    description.mFormatID = kAudioFormatLinearPCM;
    description.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked |
                               kAudioFormatFlagsNativeEndian;
    // 单声道
    description.mChannelsPerFrame = 1;
    // 每个包的帧数
    description.mFramesPerPacket = 1;
    // 每一帧的字节数
    description.mBytesPerFrame = sizeof(Float32) * description.mChannelsPerFrame;
    // 每一包的字节数
    description.mBytesPerPacket = description.mFramesPerPacket
            * description.mBytesPerFrame;
    description.mBitsPerChannel = 8 * sizeof(Float32);
    description.mReserved = 0;

    status = AudioUnitSetProperty(audio_unit,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  0,
                                  &description,
                                  sizeof(AudioStreamBasicDescription));
    if (status != noErr)
        return status;

    AURenderCallbackStruct callback;
    callback.inputProc = RenderCallback;
    callback.inputProcRefCon = nullptr;
    status = AudioUnitSetProperty(audio_unit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Global,
                                  0,
                                  &callback,
                                  sizeof(AURenderCallbackStruct));
    if (status != noErr)
        return status;

    status = AudioUnitInitialize(audio_unit);
    if (status != noErr)
        return status;

    oscillator.set_wave_on(true);
    oscillator.set_sample_rate(kDefaultSampleRate);
    oscillator.set_amplitude(1.0);
    // 分配临时内存
    buffer = static_cast<float*>(malloc(kMaxFrameNum * sizeof(float)));
    if (buffer == nullptr)
        return EXIT_FAILURE;

    // 开启新线程播放
    status = AudioOutputUnitStart(audio_unit);
    if (status != noErr) {
        free(buffer);
        return status;
    }

    // 当前线程休眠10s
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    status = AudioOutputUnitStop(audio_unit);
    if (status != noErr) {
        free(buffer);
        return status;
    }

    status = AudioUnitUninitialize(audio_unit);
    if (status != noErr) {
        free(buffer);
        return status;
    }

    free(buffer);
#endif
    return 0;
}
