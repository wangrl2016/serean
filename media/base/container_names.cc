//
// Created by wangrl2016 on 2022/10/18.
//

#include <glog/logging.h>
#include "media/base/container_names.h"
#include "media/base/bit_reader.h"

namespace media::container_names {

#define TAG(a, b, c, d)                                     \
  ((static_cast<uint32_t>(static_cast<uint8_t>(a)) << 24) | \
   (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 16) | \
   (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 8) |  \
   (static_cast<uint32_t>(static_cast<uint8_t>(d))))

#define RCHECK(x) do { if (!(x)) return false; } while(0)

    // Helper function to read 2 bytes (16 bits, big endian) from a buffer.
    // 大端高位保存在低地址
    static int Read16(const uint8_t* p) {
        return p[0] << 8 | p[1];
    }

    // Helper function to read 3 bytes (24 bits, big endian) from a buffer.
    static uint32_t Read24(const uint8_t* p) {
        return p[0] << 16 | p[1] << 8 | p[2];
    }

    // Helper function to read 4 bytes (32 bits, big endian) from a buffer.
    static uint32_t Read32(const uint8_t* p) {
        return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    }

    // Helper function to read 4 bytes (32 bits, little endian) from a buffer.
    static uint32_t Read32LE(const uint8_t* p) {
        return p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0];
    }

    // Helper function to do buffer comparisons with a string without going off the
    // end of the buffer.
    static bool StartsWith(const uint8_t* buffer,
                           size_t buffer_size,
                           const char* prefix) {
        size_t prefix_size = strlen(prefix);
        return (prefix_size <= buffer_size &&
                memcmp(buffer, prefix, prefix_size) == 0);
    }

    // Helper function to do buffer comparisons with another buffer (to allow for
    // embedded \0 in the comparison) without going off the end of the buffer.
    static bool StartsWith(const uint8_t* buffer,
                           size_t buffer_size,
                           const uint8_t* prefix,
                           size_t prefix_size) {
        return (prefix_size <= buffer_size &&
                memcmp(buffer, prefix, prefix_size) == 0);
    }

    // Helper function to read up to 64 bits from a bit stream.
    // TODO: Delete this helper and replace with direct calls to
    // reader that handle read failure. As-is, we hide failure because returning 0
    // is valid for both a successful and failed read.
    static uint64_t ReadBits(BitReader* reader, int num_bits) {
        DCHECK_GE(reader->bits_available(), num_bits);
        DCHECK((num_bits > 0) && (num_bits <= 64));
        uint64_t value = 0;

        if (!reader->ReadBits(num_bits, &value))
            return 0;

        return value;
    }

    // Additional checks for a MOV/QuickTime/MPEG4 container.
    static bool CheckMov(const uint8_t* buffer, int buffer_size) {
        RCHECK(buffer_size > 0);

        int offset = 0;
        int valid_top_level_boxes = 0;
        while (offset + 8 < buffer_size) {
            uint32_t atom_size = Read32(buffer + offset);
            uint32_t atom_type = Read32(buffer + offset + 4);

            // Only need to check for atoms that are valid at the top level. However,
            // "Boxes with an unrecognized type shall be ignored and skipped." So
            // simply make sure that at least two recognized top level boxes are found.
            // This list matches BoxReader::IsValidTopLevelBox().
            switch (atom_type) {
                case TAG('f', 't', 'y', 'p'):
                case TAG('p', 'd', 'i', 'n'):
                case TAG('b', 'l', 'o', 'c'):
                case TAG('m', 'o', 'o', 'v'):
                case TAG('m', 'o', 'o', 'f'):
                case TAG('m', 'f', 'r', 'a'):
                case TAG('m', 'd', 'a', 't'):
                case TAG('f', 'r', 'e', 'e'):
                case TAG('s', 'k', 'i', 'p'):
                case TAG('m', 'e', 't', 'a'):
                case TAG('m', 'e', 'c', 'o'):
                case TAG('s', 't', 'y', 'p'):
                case TAG('s', 'i', 'd', 'x'):
                case TAG('s', 's', 'i', 'x'):
                case TAG('p', 'r', 'f', 't'):
                case TAG('u', 'u', 'i', 'd'):
                case TAG('e', 'm', 's', 'g'):
                    ++valid_top_level_boxes;
                    break;
            }

            if (atom_size == 1) {
                // Indicates that the length is the next 64bits.
                if (offset + 16 > buffer_size)
                    break;

                if (Read32(buffer + offset + 8) != 0)
                    break;  // offset is way past buffer size.
                atom_size = Read32(buffer + offset + 12);
            }

            if (atom_size == 0 || atom_size > static_cast<size_t>(buffer_size))
                break;  // indicates the last atom or length too big.
            offset += atom_size;
        }

        return valid_top_level_boxes >= 2;
    }

    // Read a Matroska Element Id.
    static int GetElementId(BitReader* reader) {
        // Element ID is coded with the leading zero bits (max 3) determining size.
        // If it is an invalid encoding or the end of the buffer is reached,
        // return -1 as a tag that won't be expected.
        if (reader->bits_available() >= 8) {
            int num_bits_to_read = 0;
            static int prefix[] = {0x80, 0x4000, 0x200000, 0x10000000};
            for (int i = 0; i < 4; i++) {
                num_bits_to_read += 7;
                if (ReadBits(reader, 1) == 1) {
                    if (reader->bits_available() < num_bits_to_read)
                        break;
                    // prefix[] adds back the bits read individually.
                    return ReadBits(reader, num_bits_to_read) | prefix[i];
                }
            }
        }
        // Invalid encoding, return something not expected.
        return -1;
    }

    // Read a Matroska Unsigned Integer (VINT).
    static uint64_t GetVint(BitReader* reader) {
        // Values are coded with the leading zero bits (max 7) determining size.
        // If it is an invalid coding or the end of the buffer is reached,
        // return something that will go off the end of the buffer.
        if (reader->bits_available() >= 8) {
            int num_bits_to_read = 0;
            for (int i = 0; i < 8; ++i) {
                num_bits_to_read += 7;
                if (ReadBits(reader, 1) == 1) {
                    if (reader->bits_available() < num_bits_to_read)
                        break;
                    return ReadBits(reader, num_bits_to_read);
                }
            }
        }
        // Incorrect format (more than 7 leading 0's) or off the end of the buffer.
        // Since the return value is used as a byte size, return a value that will
        // cause a failure when used.
        return (reader->bits_available() / 8) + 2;
    }

    static bool CheckWebm(const uint8_t* buffer, int buffer_size) {
        // Reference: http://www.matroska.org/technical/specs/index.html
        RCHECK(buffer_size > 12);

        BitReader reader(buffer, buffer_size);

        // Verify starting Element Id.
        RCHECK(GetElementId(&reader) == 0x1a45dfa3);

        int header_size = GetVint(&reader);
        RCHECK(reader.bits_available() / 8 >= header_size);

        // Loop through the header.
        while (reader.bits_available() > 0) {
            int tag = GetElementId(&reader);
            int tag_size = GetVint(&reader);
            switch (tag) {
                case 0x4286:  // EBMLVersion
                case 0x42f7:  // EBMLReadVersion
                case 0x42f2:  // EBMLMaxIdLength
                case 0x42f3:  // EBMLMaxSizeLength
                case 0x4287:  // DocTypeVersion
                case 0x4285:  // DocTypeReadVersion
                case 0xec:    // void
                case 0xbf:    // CRC32
                    RCHECK(reader.bits_available() / 8 >= tag_size);
                    RCHECK(reader.SkipBits(tag_size * 8));
                    break;

                case 0x4282:  // EBMLDocType
                    // Need to see "webm" or "matroska" next.
                    RCHECK(reader.bits_available() >= 32);
                    switch (ReadBits(&reader, 32)) {
                        case TAG('w', 'e', 'b', 'm') :
                            return true;
                        case TAG('m', 'a', 't', 'r') :
                            RCHECK(reader.bits_available() >= 32);
                            return (ReadBits(&reader, 32) == TAG('o', 's', 'k', 'a'));
                    }
                    return false;

                default:  // Unrecognized tag
                    return false;
            }
        }
        return false;
    }

    // For some formats the signature is a bunch of characters. They are defined
    // below. Note that the first 4 characters of the string may be used as a TAG
    // in LookupContainerByFirst4. For signatures that contain embedded \0, use
    // uint8_t[].
    static const char kAmrSignature[] = "#!AMR";
    static const uint8_t kAsfSignature[] = {0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66,
                                            0xcf, 0x11, 0xa6, 0xd9, 0x00, 0xaa,
                                            0x00, 0x62, 0xce, 0x6c};
    static const char kAssSignature[] = "[Script Info]";

    // Attempt to determine the container type from the buffer provided. This is
    // a simple pass, that uses the first 4 bytes of the buffer as an index to get
    // a rough idea of the container format.
    static MediaContainerName LookupContainerByFirst4(const uint8_t* buffer,
                                                      int buffer_size) {
        // Minimum size that the code expects to exist without checking size.
        if (buffer_size < kMinimumContainerSize)
            return CONTAINER_UNKNOWN;

        uint32_t first4 = Read32(buffer);
        switch (first4) {
            case 0x1a45dfa3:
                if (CheckWebm(buffer, buffer_size))
                    return CONTAINER_WEBM;
                break;
        }

        return CONTAINER_UNKNOWN;
    }

    MediaContainerName DetermineContainer(const uint8_t* buffer, int buffer_size) {
        DCHECK(buffer);

        // Since MOV/QuickTime/MPEG4 streams are common, check for them first.
        if (CheckMov(buffer, buffer_size))
            return CONTAINER_MOV;

        // Next attempt the simple checks, that typically look at just the
        // first few bytes of the file.
        MediaContainerName result = LookupContainerByFirst4(buffer, buffer_size);

        return CONTAINER_UNKNOWN;
    }
}
