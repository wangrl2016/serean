//
// Created by wangrl2016 on 2022/11/2.
//

#include <cstdio>
#include <cstdlib>

// 1. 初始化AVFormatContext
// 2. 通过AVFormatContext创建Decoder
// 3. 通过Decoder创建AVCodecContext
// 4. av_read_frame函数读取packet
// 5. avcodec_send_packet解码
// 6. avcodec_receive_frame获取解码后的数据

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

static AVFormatContext* input_fmt_ctx = nullptr;
static const char* src_filename = nullptr;
static int audio_stream_idx = -1;
static AVCodecContext* audio_dec_ctx = nullptr;
static AVStream* audio_stream = nullptr;
static FILE* audio_dst_file = nullptr;
static const char* audio_dst_filename = nullptr;
static AVFrame* frame = nullptr;
static AVPacket* pkt = nullptr;
static int audio_frame_count = 0;

static int get_format_from_sample_fmt(const char** fmt,
                                      enum AVSampleFormat sample_fmt) {
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt;
        const char* fmt_be, * fmt_le;
    } sample_fmt_entries[] = {
            {AV_SAMPLE_FMT_U8,  "u8",    "u8"},
            {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
            {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
            {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
            {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };
    *fmt = nullptr;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry* entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

static int output_audio_frame(AVFrame* fra) {
    size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(
            static_cast<AVSampleFormat>(frame->format));
    printf("audio_frame n:%d nb_samples:%d pts:%s\n",
           audio_frame_count++, frame->nb_samples,
           av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
    fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);

    return 0;

}

static int decode_packet(AVCodecContext* dec, const AVPacket* packet) {
    int ret = 0;

    // Submit the packet to the decoder.
    ret = avcodec_send_packet(dec, packet);
    if (ret < 0) {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        }

        if (dec->codec->type == AVMEDIA_TYPE_AUDIO)
            ret = output_audio_frame(frame);

        av_frame_unref(frame);
        if (ret < 0)
            return ret;
    }
    return 0;
}

static int open_codec_context(int* stream_idx,
                              AVCodecContext** dec_ctx,
                              AVFormatContext* fmt_ctx,
                              enum AVMediaType type) {
    AVStream* st;
    int stream_index;
    const AVCodec* dec = nullptr;
    int ret = av_find_best_stream(fmt_ctx, type, -1, -1, nullptr, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file %s\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        // find decoder for the stream
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        // Allocate a codec context for the decoder
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        // Copy codec parameters from input stream to output codec context.
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }

        // Init the decoders.
        if ((ret = avcodec_open2(*dec_ctx, dec, nullptr)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    int ret = 0;
    enum AVSampleFormat sfmt;
    int n_channels;
    const char* fmt;
    if (argc != 3) {
        fprintf(stderr, "Invalid argument\n");
        exit(1);
    }
    src_filename = argv[1];
    audio_dst_filename = argv[2];

    /* open input file, and allocate format context */
    if (avformat_open_input(&input_fmt_ctx, src_filename, nullptr, nullptr) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(input_fmt_ctx, nullptr) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    // 初始化解码器
    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, input_fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = input_fmt_ctx->streams[audio_stream_idx];
        audio_dst_file = fopen(audio_dst_filename, "wb");
        if (!audio_dst_file) {
            fprintf(stderr, "Could not open destination file %s\n", audio_dst_filename);
            ret = 1;
            goto end;
        }
    }

    if (!audio_stream) {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate packet\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    printf("Demuxing audio from file '%s' into '%s'\n", src_filename, audio_dst_filename);

    // Read frames from the file.
    while (av_read_frame(input_fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == audio_stream_idx)
            ret = decode_packet(audio_dec_ctx, pkt);
        av_packet_unref(pkt);
        if (ret < 0)
            break;
    }

    // flush the decoders
    if (audio_dec_ctx)
        decode_packet(audio_dec_ctx, nullptr);

    sfmt = audio_dec_ctx->sample_fmt;
    n_channels = audio_dec_ctx->channels;

    // 声道分开分布
    if (av_sample_fmt_is_planar(sfmt)) {
        const char* packed = av_get_sample_fmt_name(sfmt);
        printf("Warning: the sample format the decoder produced is planar "
               "(%s). This example will output the first channel only.\n",
               packed ? packed : "?");
        sfmt = av_get_packed_sample_fmt(sfmt);
        n_channels = 1;
    }

    if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
        goto end;

    printf("Play the output audio file with the command:\n"
           "ffplay -f %s -ac %d -ar %d %s\n",
           fmt, n_channels, audio_dec_ctx->sample_rate,
           audio_dst_filename);

    end:
    avcodec_free_context(&audio_dec_ctx);
    avformat_close_input(&input_fmt_ctx);
    if (audio_dst_file)
        fclose(audio_dst_file);
    av_packet_free(&pkt);
    av_frame_free(&frame);
    return ret < 0;
}
