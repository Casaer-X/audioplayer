#ifndef AUDIO_ANALYZER_H
#define AUDIO_ANALYZER_H

#include <vector>
#include <cstdint>
#include <string>

struct AudioFeatures {
    double bpm;
    double energy;
    double brightness;
    double danceability;
    double valence;

    double start_energy;
    double end_energy;
    double start_bpm;
    double end_bpm;

    double duration;
    std::string filepath;
};

class AudioAnalyzer {
public:
    static AudioFeatures analyze(const std::string& filepath);
    static AudioFeatures analyzeFromPCM(const std::vector<int16_t>& pcm_data,
                                        uint32_t sample_rate,
                                        uint16_t channels);

private:
    static double calculateBPM(const std::vector<int16_t>& pcm, uint32_t sample_rate);
    static double calculateEnergy(const std::vector<int16_t>& pcm,
                                  size_t start, size_t end);
    static double calculateBrightness(const std::vector<int16_t>& pcm,
                                      size_t start, size_t end);
    static double calculateDanceability(double bpm, double energy);
    static double calculateValence(double energy, double brightness);
    static std::vector<double> extractOnsets(const std::vector<double>& envelope,
                                             uint32_t sample_rate);
    static std::vector<double> calculateEnvelope(const std::vector<int16_t>& pcm,
                                                  int window_size = 1024);
};

#endif
