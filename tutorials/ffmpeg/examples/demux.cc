//
// Created by wangrl2016 on 2022/11/1.
//

#include <cstdio>
#include <cstdlib>

extern "C" {
#include <libavformat/avformat.h>
}

static AVFormatContext *fmt_ctx = nullptr;
static const char *src_filename = nullptr;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid argument\n");
        exit(1);
    }
    src_filename = argv[1];

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, src_filename, nullptr, nullptr) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, src_filename, 0);

    avformat_close_input(&fmt_ctx);

    return EXIT_SUCCESS;
}