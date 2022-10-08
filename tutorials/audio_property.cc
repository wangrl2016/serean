//
// Created by wangrl2016 on 2022/9/22.
//

// 1. 安装Conan管理FFmpeg库
// 安装：pip3 install conan
// export PATH=${PATH}:/Users/wr/Library/Python/3.9/bin
// source ~/.zprofile
// 验证：conan --veriosn
// 2. 编写conanfile.txt文件
// 编译目录执行：conan install .. --build=missing
// 3. 修改CMakeList.txt文件引入FFmpeg库
// 增加${CONAN_LIBS}
// include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
// conan_basic_setup()
// 4. 引入FFmpeg函数验证安装成功
// 6. 音频声道
// 7. 采样率（重采样）
// 8. 采样格式

#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    // 声道
    int channel_count = av_get_channel_layout_nb_channels(
            AV_CH_LAYOUT_STEREO);
    std::cout << "AV_CH_LAYOUT_STEREO channels " << channel_count << std::endl;

    int64_t channel_layout = av_get_default_channel_layout(2);
    std::cout << channel_layout << ", " << AV_CH_LAYOUT_STEREO << std::endl;

    // 采样率44100/48000

    // 采样格式
    const char* fmt_name = av_get_sample_fmt_name(
            AV_SAMPLE_FMT_FLT);
    std::cout << fmt_name << std::endl;

    return 0;
}
