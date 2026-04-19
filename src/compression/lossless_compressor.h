#ifndef LOSSLESS_COMPRESSOR_H
#define LOSSLESS_COMPRESSOR_H

#include <vector>
#include <cstdint>
#include <string>

struct CompressedAudioData {
    int32_t first_sample;
    std::vector<int8_t> residuals;
    uint32_t sample_rate;
    uint16_t channels;
    uint64_t original_size;
};

class LosslessCompressor {
public:
    static CompressedAudioData compress(const std::vector<int16_t>& pcm_data,
                                        uint32_t sample_rate = 44100,
                                        uint16_t channels = 2);
    static std::vector<int16_t> decompress(const CompressedAudioData& compressed);
    static double calculateCompressionRatio(const std::vector<int16_t>& original,
                                           const CompressedAudioData& compressed);
    static bool saveToFile(const CompressedAudioData& compressed,
                          const std::string& filepath);
    static CompressedAudioData loadFromFile(const std::string& filepath);

private:
    static std::vector<int8_t> riceEncode(int32_t value);
    static int32_t riceDecode(const std::vector<int8_t>& encoded, size_t& pos);
};

#endif
