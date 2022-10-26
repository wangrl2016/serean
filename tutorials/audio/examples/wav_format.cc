//
// Created by wangrl2016 on 2022/7/17.
//

#include <cassert>
#include <filesystem>
#include <fstream>

// 使用裸数据生成WAV文件
// 参考文档: http://soundfile.sapp.org/doc/WaveFormat/

int main(int argc, char* argv[]) {
    assert(argc == 3);

    size_t bufferSize = std::filesystem::file_size(argv[1]);  // byte为单位
    void* bufferData = calloc(bufferSize, sizeof(uint8_t));

    std::ifstream ifs;
    ifs.open(argv[1], std::ios::binary);
    ifs.read((char*) bufferData, (long) bufferSize);

    // 使用裸数据生成wav文件，只支持PCM格式
    /**
     * bytes                variable                description
     * 0 - 3                'RIFF'/'RIFX'           Little/Big-endian
     * 4 - 7                wRiffLength             Length of file minus the
     *                                              8 byte riff header
     * 8 - 11               'WAVE'
     * 12 - 15              'fmt'
     * 16 - 19              wFmtSize                Length of format chunk minus
     *                                              8 byte header
     * 20 - 21              wFormatTag              identifies PCM, ULAW etc
     * 22 - 23              wChannels
     * 24 - 27              dwSamplesPerSecond      Samples per second per channel
     * 28 - 31              dwAvgBytesPerSec        Non-trivial for compressed formats
     * 32 - 33              wBlockAlign             Basic block size
     * 34 - 35              wBitsPerSample          Non-trivial for compressed formats
     *
     * PCM formats then go straight to the data chunk:
     * 36 - 39              'data'
     * 40 - 43              dwDataLength            Length of data chunk
     *                                              minus 8 byte header
     * 44 - (dwDataLength + 43)                     the data
     * (* a padding byte if dwDataLength is odd)
     */
    char* buffer = (char*) calloc(bufferSize, bufferSize + 44);

    // Contains the letters "RIFF" in ASCII form
    // (0x52494646 big-endian form).
    // 0 - 3
    std::string ChunkID = "RIFF";

    // Contains the letters "WAVE"
    // (0x57415645 big-endian form).
    // 8 - 11
    std::string Format = "WAVE";

    // Contains the letters "fmt "
    // (0x666d7420 big-endian form).
    // 12 - 15
    std::string Subchunk1ID = "fmt ";

    // 16 for PCM.  This is the size of the
    // rest of the Subchunk which follows this number.
    // 16 - 19
    int32_t Subchunk1Size = 16;

    // PCM = 1 (i.e. Linear quantization)
    // Values other than 1 indicate some
    // form of compression.
    // 20 - 21
    int16_t AudioFormat = 1;

    // Mono = 1, Stereo = 2, etc.
    // 22 - 23
    int16_t NumChannels = 1;

    // 8000, 44100, etc.
    // 24 - 27
    int32_t SampleRate = 16000;

    // 8 bits = 8, 16 bits = 16, etc.
    // 34 - 35
    int16_t BitsPerSample = 16;

    // == SampleRate * NumChannels * BitsPerSample/8
    // 28 - 31
    int ByteRate = SampleRate * NumChannels * BitsPerSample / 8;

    // == NumChannels * BitsPerSample/8
    // The number of bytes for one sample including
    // all channels. I wonder what happens when
    // this number isn't an integer?
    // 32 - 33
    int BlockAlign = NumChannels * BitsPerSample / 8;

    // Contains the letters "data"
    // (0x64617461 big-endian form).
    // 36 - 39
    std::string Subchunk2ID = "data";

    // == NumSamples * NumChannels * BitsPerSample/8
    // This is the number of bytes in the data.
    // You can also think of this as the size
    // of the read of the subchunk following this
    // number.
    // 40 - 43
    auto Subchunk2Size = int32_t(bufferSize);

    // 36 + SubChunk2Size, or more precisely:
    // 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
    // This is the size of the rest of the chunk
    // following this number.  This is the size of the
    // entire file in bytes minus 8 bytes for the
    // two fields not included in this count:
    // ChunkID and ChunkSize.
    // 4 - 7
    int32_t ChunkSize = 36 + Subchunk2Size;

    // 写入文件头
    memcpy(buffer, ChunkID.c_str(), 4);
    memcpy(buffer + 4, &ChunkSize, 4);
    memcpy(buffer + 8, Format.c_str(), 4);
    memcpy(buffer + 12, Subchunk1ID.c_str(), 4);
    memcpy(buffer + 16, &Subchunk1Size, 4);
    memcpy(buffer + 20, &AudioFormat, 2);
    memcpy(buffer + 22, &NumChannels, 2);
    memcpy(buffer + 24, &SampleRate, 4);
    memcpy(buffer + 28, &ByteRate, 4);
    memcpy(buffer + 32, &BlockAlign, 2);
    memcpy(buffer + 34, &BitsPerSample, 2);
    memcpy(buffer + 36, Subchunk2ID.c_str(), 4);
    memcpy(buffer + 40, &Subchunk2Size, 4);

    // 写入文件数据
    memcpy(buffer + 44, bufferData, bufferSize);

    std::ofstream ofs(argv[2],
                      std::ios::out | std::ios::binary);
    ofs.write(buffer, int(bufferSize) + 44);
    ofs.close();

    return EXIT_SUCCESS;
}
