//
// Created by wangrl2016 on 2022/10/18.
//

#ifndef SEREAN_BIT_READER_H
#define SEREAN_BIT_READER_H

#include "media/base/bit_reader_core.h"

namespace media {
    class BitReader : private BitReaderCore::ByteStreamProvider {
    public:
        // Initialize the reader to start reading at |data|, |size| being size
        // of |data| in bytes.
        BitReader(const uint8_t* data, int size);

        BitReader(const BitReader&) = delete;

        BitReader& operator=(const BitReader&) = delete;

        ~BitReader() override;

        template<typename T>
        bool ReadBits(int num_bits, T* out) {
            return bit_reader_core_.ReadBits(num_bits, out);
        }

        bool ReadFlag(bool* flag) {
            return bit_reader_core_.ReadFlag(flag);
        }

        // Read |num_bits| of binary data into |str|. |num_bits| must be a positive
        // multiple of 8. This is not efficient for extracting large strings.
        // If false is returned, |str| may not be valid.
        bool ReadString(int num_bits, std::string* str);

        bool SkipBits(int num_bits) {
            return bit_reader_core_.SkipBits(num_bits);
        }

        int bits_available() const {
            return initial_size_ * 8 - bits_read();
        }

        int bits_read() const {
            return bit_reader_core_.bits_read();
        }

    private:
        // BitReaderCore::ByteStreamProvider implementation.
        int GetBytes(int max_n, const uint8_t** out) override;

        // Total number of bytes that was initially passed to BitReader.
        const int initial_size_;

        // Pointer to the next unread byte in the stream.
        const uint8_t* data_;

        // Bytes left in the stream.
        int bytes_left_;

        BitReaderCore bit_reader_core_;
    };
}

#endif //SEREAN_BIT_READER_H
