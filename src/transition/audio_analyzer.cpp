#include "audio_analyzer.h"
#include <cmath>
#include <algorithm>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioFeatures AudioAnalyzer::analyze(const std::string& filepath) {
    AudioFeatures features;
    features.filepath = filepath;

    return features;
}

AudioFeatures AudioAnalyzer::analyzeFromPCM(const std::vector<int16_t>& pcm_data,
                                             uint32_t sample_rate,
                                             uint16_t channels) {
    AudioFeatures features;
    features.duration = static_cast<double>(pcm_data.size() / channels) / sample_rate;

    size_t total_samples = pcm_data.size() / channels;
    size_t start_segment = 0;
    size_t end_segment_start = total_samples * 9 / 10;

    if (total_samples > 0) {
        std::vector<int16_t> mono(total_samples);
        for (size_t i = 0; i < total_samples; ++i) {
            int32_t sum = 0;
            for (uint16_t c = 0; c < channels; ++c) {
                sum += pcm_data[i * channels + c];
            }
            mono[i] = static_cast<int16_t>(sum / channels);
        }

        features.bpm = calculateBPM(mono, sample_rate);

        features.energy = calculateEnergy(mono, 0, total_samples);
        features.brightness = calculateBrightness(mono, 0, total_samples);

        size_t segment_size = std::min(static_cast<size_t>(sample_rate * 5), total_samples / 10);

        if (segment_size > 0 && total_samples > segment_size * 2) {
            features.start_energy = calculateEnergy(mono, start_segment, start_segment + segment_size);
            features.end_energy = calculateEnergy(mono, end_segment_start,
                                                  std::min(end_segment_start + segment_size, total_samples));
            features.start_bpm = calculateBPM(
                std::vector<int16_t>(mono.begin() + start_segment,
                                     mono.begin() + start_segment + segment_size),
                sample_rate);
            features.end_bpm = calculateBPM(
                std::vector<int16_t>(mono.begin() + end_segment_start,
                                     mono.begin() + std::min(end_segment_start + segment_size, total_samples)),
                sample_rate);
        } else {
            features.start_energy = features.energy;
            features.end_energy = features.energy;
            features.start_bpm = features.bpm;
            features.end_bpm = features.bpm;
        }

        features.danceability = calculateDanceability(features.bpm, features.energy);
        features.valence = calculateValence(features.energy, features.brightness);
    }

    return features;
}

double AudioAnalyzer::calculateBPM(const std::vector<int16_t>& pcm, uint32_t sample_rate) {
    auto envelope = calculateEnvelope(pcm, 1024);

    if (envelope.size() < 256) return 120.0;

    std::vector<double> onsets = extractOnsets(envelope, sample_rate / 1024);

    if (onsets.size() < 2) return 120.0;

    std::vector<double> intervals;
    for (size_t i = 1; i < onsets.size(); ++i) {
        double interval = onsets[i] - onsets[i-1];
        if (interval > 0.2 && interval < 2.0) {
            intervals.push_back(interval);
        }
    }

    if (intervals.empty()) return 120.0;

    double avg_interval = std::accumulate(intervals.begin(), intervals.end(), 0.0) /
                          intervals.size();

    double bpm = 60.0 / avg_interval;

    if (bpm < 60) bpm *= 2;
    else if (bpm > 180) bpm /= 2;

    bpm = std::max(60.0, std::min(200.0, bpm));

    return bpm;
}

double AudioAnalyzer::calculateEnergy(const std::vector<int16_t>& pcm,
                                      size_t start, size_t end) {
    if (start >= pcm.size() || end <= start || end > pcm.size()) return 0.0;

    double sum_squares = 0.0;
    size_t count = end - start;

    for (size_t i = start; i < end; ++i) {
        double sample = static_cast<double>(pcm[i]) / 32768.0;
        sum_squares += sample * sample;
    }

    double rms = std::sqrt(sum_squares / count);

    double energy_db = 20.0 * std::log10(rms + 1e-10);
    energy_db = std::max(-60.0, std::min(0.0, energy_db));

    return (energy_db + 60.0) / 60.0;
}

double AudioAnalyzer::calculateBrightness(const std::vector<int16_t>& pcm,
                                           size_t start, size_t end) {
    if (start >= pcm.size() || end <= start || end > pcm.size()) return 0.5;

    size_t length = end - start;
    if (length < 512) return 0.5;

    std::vector<double> spectrum(1024, 0.0);
    size_t fft_size = 1024;

    for (size_t i = 0; i < fft_size && (start + i) < end; ++i) {
        double sample = static_cast<double>(pcm[start + i]);
        for (size_t k = 0; k < fft_size; ++k) {
            double t = static_cast<double>(i) / fft_size;
            spectrum[k] += sample * std::cos(2.0 * M_PI * k * t);
        }
    }

    double low_energy = 0.0;
    double high_energy = 0.0;
    size_t mid_point = fft_size / 2;

    for (size_t k = 1; k < mid_point; ++k) {
        double mag = std::abs(spectrum[k]);
        if (k < mid_point / 4) {
            low_energy += mag;
        } else {
            high_energy += mag;
        }
    }

    if (low_energy == 0.0) return 0.5;

    double brightness = high_energy / (low_energy + high_energy);

    return std::max(0.0, std::min(1.0, brightness));
}

double AudioAnalyzer::calculateDanceability(double bpm, double energy) {
    double ideal_bpm_range = 1.0 - std::abs(bpm - 124.0) / 100.0;
    ideal_bpm_range = std::max(0.0, ideal_bpm_range);

    return (ideal_bpm_range * 0.6 + energy * 0.4);
}

double AudioAnalyzer::calculateValence(double energy, double brightness) {
    return (energy * 0.7 + brightness * 0.3);
}

std::vector<double> AudioAnalyzer::extractOnsets(const std::vector<double>& envelope,
                                                 uint32_t hop_rate) {
    std::vector<double> onsets;
    const double threshold = 0.15;

    for (size_t i = 1; i < envelope.size() - 1; ++i) {
        double diff = envelope[i] - envelope[i-1];

        if (diff > threshold &&
            envelope[i] > envelope[i-1] &&
            envelope[i] >= envelope[i+1]) {

            double time = static_cast<double>(i) / hop_rate;
            onsets.push_back(time);
        }
    }

    return onsets;
}

std::vector<double> AudioAnalyzer::calculateEnvelope(const std::vector<int16_t>& pcm,
                                                      int window_size) {
    std::vector<double> envelope;

    for (size_t i = 0; i < pcm.size(); i += window_size / 2) {
        size_t end = std::min(i + window_size, pcm.size());

        double sum = 0.0;
        for (size_t j = i; j < end; ++j) {
            double sample = static_cast<double>(pcm[j]);
            sum += sample * sample;
        }

        double rms = std::sqrt(sum / (end - i));
        envelope.push_back(rms / 32768.0);
    }

    return envelope;
}
