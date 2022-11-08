//
// Created by wangrl2016 on 2022/11/8.
//

// 读取多媒体文件中的图片进行保存

#include <fstream>

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

    if (fmt_ctx->iformat->read_header(fmt_ctx) < 0) {
        fprintf(stderr, "No header format");
        exit(1);
    }

    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            AVPacket pkt = fmt_ctx->streams[i]->attached_pic;
            std::ofstream ofs;
            ofs.open(std::to_string(i));
            ofs.write(reinterpret_cast<const char*>(pkt.data), pkt.size);
            ofs.close();
        }
    }
}