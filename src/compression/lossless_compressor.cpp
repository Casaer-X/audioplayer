#include "lossless_compressor.h"
#include <fstream>
#include <cmath>
#include <algorithm>

CompressedAudioData LosslessCompressor::compress(const std::vector<int16_t>& pcm_data,
                                                  uint32_t sample_rate,
                                                  uint16_t channels) {
    CompressedAudioData output;
    if (pcm_data.empty()) return output;

    output.sample_rate = sample_rate;
    output.channels = channels;
    output.original_size = pcm_data.size() * sizeof(int16_t);

    output.first_sample = pcm_data[0];

    for (size_t i = 1; i < pcm_data.size(); ++i) {
        int32_t diff = static_cast<int32_t>(pcm_data[i]) - static_cast<int32_t>(pcm_data[i - 1]);

        auto encoded = riceEncode(diff);
        output.residuals.insert(output.residuals.end(), encoded.begin(), encoded.end());
    }

    return output;
}

std::vector<int16_t> LosslessCompressor::decompress(const CompressedAudioData& compressed) {
    std::vector<int16_t> output;
    output.push_back(static_cast<int16_t>(compressed.first_sample));

    size_t pos = 0;
    while (pos < compressed.residuals.size()) {
        int32_t diff = riceDecode(compressed.residuals, pos);
        int16_t next_sample = static_cast<int16_t>(output.back() + diff);
        output.push_back(next_sample);
    }

    return output;
}

double LosslessCompressor::calculateCompressionRatio(const std::vector<int16_t>& original,
                                                     const CompressedAudioData& compressed) {
    size_t compressed_size = sizeof(int32_t) + compressed.residuals.size();
    size_t original_size = original.size() * sizeof(int16_t);

    if (original_size == 0) return 0.0;

    return static_cast<double>(original_size) / static_cast<double>(compressed_size);
}

std::vector<int8_t> LosslessCompressor::riceEncode(int32_t value) {
    std::vector<int8_t> encoded;

    bool is_negative = value < 0;
    uint32_t abs_value = static_cast<uint32_t>(std::abs(value));

    unsigned int k = 4;
    uint32_t q = abs_value >> k;
    uint32_t r = abs_value & ((1 << k) - 1);

    for (unsigned int i = 0; i < q; ++i) {
        encoded.push_back(0x00);
    }
    encoded.push_back(0x01);

    uint8_t remainder_byte = static_cast<uint8_t>(r);
    if (is_negative) {
        remainder_byte |= (1 << k);
    }
    encoded.push_back(static_cast<int8_t>(remainder_byte));

    return encoded;
}

int32_t LosslessCompressor::riceDecode(const std::vector<int8_t>& encoded, size_t& pos) {
    unsigned int k = 4;

    uint32_t q = 0;
    while (pos < encoded.size() && encoded[pos] == 0x00) {
        q++;
        pos++;
    }

    if (pos >= encoded.size()) return 0;
    pos++;

    if (pos >= encoded.size()) return static_cast<int32_t>(q << k);

    uint8_t remainder_byte = static_cast<uint8_t>(encoded[pos]);
    pos++;

    bool is_negative = (remainder_byte & (1 << k)) != 0;
    uint32_t r = remainder_byte & ((1 << k) - 1);

    int32_t value = static_cast<int32_t>((q << k) | r);
    return is_negative ? -value : value;
}

bool LosslessCompressor::saveToFile(const CompressedAudioData& compressed,
                                    const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(&compressed.first_sample), sizeof(int32_t));
    file.write(reinterpret_cast<const char*>(&compressed.sample_rate), sizeof(uint32_t));
    file.write(reinterpret_cast<const char*>(&compressed.channels), sizeof(uint16_t));
    file.write(reinterpret_cast<const char*>(&compressed.original_size), sizeof(uint64_t));

    uint64_t residuals_size = compressed.residuals.size();
    file.write(reinterpret_cast<const char*>(&residuals_size), sizeof(uint64_t));

    file.write(reinterpret_cast<const char*>(compressed.residuals.data()),
               compressed.residuals.size());

    file.close();
    return true;
}

CompressedAudioData LosslessCompressor::loadFromFile(const std::string& filepath) {
    CompressedAudioData compressed;
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return compressed;

    file.read(reinterpret_cast<char*>(&compressed.first_sample), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&compressed.sample_rate), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&compressed.channels), sizeof(uint16_t));
    file.read(reinterpret_cast<char*>(&compressed.original_size), sizeof(uint64_t));

    uint64_t residuals_size = 0;
    file.read(reinterpret_cast<char*>(&residuals_size), sizeof(uint64_t));

    compressed.residuals.resize(residuals_size);
    file.read(reinterpret_cast<char*>(compressed.residuals.data()), residuals_size);

    file.close();
    return compressed;
}
