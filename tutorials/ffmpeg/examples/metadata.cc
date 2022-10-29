//
// Created by wangrl2016 on 2022/10/29.
//

#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

int main(int argc, char* argv[]) {
    AVFormatContext* fmt_ctx = nullptr;
    const AVDictionaryEntry* tag = nullptr;
    int ret;

    if (argc != 2) {
        std::cout << "Usage: %s <input_file>" << std::endl;
        std::cout << "Example program to demonstrate the use of the"
                     "libavformat metadata API" << std::endl;
        return EXIT_FAILURE;
    }

    if ((ret = avformat_open_input(&fmt_ctx,
                                   argv[1], nullptr, nullptr)))
        return ret;

    if ((ret = avformat_find_stream_info(fmt_ctx, nullptr)) < 0) {
        std::cout << "Cannot find stream information" << std::endl;
        return ret;
    }

    // av_dump_format(fmt_ctx, 0, argv[1], 0);

    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        std::cout << tag->key << ", " << tag->value << std::endl;

    avformat_close_input(&fmt_ctx);
    return 0;
}