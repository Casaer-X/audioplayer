#include "livesort_algorithm.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <limits>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TransitionParams LiveSortAlgorithm::getDefaultParams() {
    TransitionParams params;
    params.crossfade_duration = 3.0;
    params.enable_energy_matching = true;
    params.enable_bpm_matching = true;
    params.energy_weight = 0.4;
    params.bpm_weight = 0.35;
    params.style_weight = 0.25;
    return params;
}

std::vector<size_t> LiveSortAlgorithm::optimizePlaylistOrder(
    const std::vector<AudioFeatures>& songs,
    TransitionParams params) {

    if (songs.size() <= 1) {
        std::vector<size_t> result(songs.size());
        std::iota(result.begin(), result.end(), 0);
        return result;
    }

    auto ideal_curve = generateIdealCurve(songs.size());

    std::vector<size_t> ordered_indices(songs.size());
    std::iota(ordered_indices.begin(), ordered_indices.end(), 0);

    std::vector<bool> used(songs.size(), false);
    std::vector<size_t> result(songs.size());
    std::vector<double> position_scores(songs.size(), 0.0);

    for (size_t pos = 0; pos < songs.size(); ++pos) {
        double target_energy = ideal_curve[pos];

        double best_score = -std::numeric_limits<double>::max();
        size_t best_song = 0;

        for (size_t i = 0; i < songs.size(); ++i) {
            if (used[i]) continue;

            double curve_score = evaluateCurveFit(songs[i].energy, target_energy);
            double total_score = curve_score * 0.5 + position_scores[i] * 0.1;

            if (pos > 0) {
                size_t prev_song = result[pos - 1];
                double transition_score = calculateTransitionScore(
                    songs[prev_song], songs[i], params);

                double bpm_diff = std::abs(songs[prev_song].end_bpm - songs[i].start_bpm);
                double bpm_penalty = bpm_diff > 20.0 ? (bpm_diff - 20.0) / 100.0 : 0.0;

                total_score += transition_score * 0.4 - bpm_penalty;
            }

            if (total_score > best_score) {
                best_score = total_score;
                best_song = i;
            }
        }

        used[best_song] = true;
        result[pos] = best_song;

        for (size_t i = 0; i < songs.size(); ++i) {
            if (!used[i]) {
                double transition = calculateTransitionScore(
                    songs[best_song], songs[i], params);
                position_scores[i] += transition * 0.05;
            }
        }
    }

    return result;
}

std::vector<size_t> LiveSortAlgorithm::generateEmotionalCurve(size_t length) {
    return generateIdealCurve(length);
}

double LiveSortAlgorithm::calculateTransitionScore(const AudioFeatures& from,
                                                    const AudioFeatures& to,
                                                    const TransitionParams& params) {
    double energy_score = calculateEnergyTransition(from.end_energy, to.start_energy);
    double bpm_score = calculateBPMCompatibility(from.end_bpm, to.start_bpm);
    double style_score = calculateStyleConsistency(from, to);

    double total_score = energy_score * params.energy_weight +
                        bpm_score * params.bpm_weight +
                        style_score * params.style_weight;

    return total_score;
}

std::pair<std::vector<float>, std::vector<float>>
LiveSortAlgorithm::calculateCrossfadeCurve(const AudioFeatures& from,
                                            const AudioFeatures& to,
                                            double crossfade_duration) {

    int num_samples = static_cast<int>(crossfade_duration * 44100);
    std::vector<float> fade_out(num_samples);
    std::vector<float> fade_in(num_samples);

    double energy_ratio = (to.start_energy + 0.01) / (from.end_energy + 0.01);
    energy_ratio = std::max(0.3, std::min(3.0, energy_ratio));

    double bpm_ratio = (to.start_bpm + 1.0) / (from.end_bpm + 1.0);
    bpm_ratio = std::max(0.7, std::min(1.4, bpm_ratio));

    bool is_energetic_transition = from.end_energy > 0.6 && to.start_energy > 0.6;

    for (int i = 0; i < num_samples; ++i) {
        double t = static_cast<double>(i) / num_samples;

        double smooth_t = t * t * (3.0 - 2.0 * t);

        if (is_energetic_transition) {
            double compressed_t = std::pow(t, 0.6);
            fade_out[i] = static_cast<float>(1.0 - compressed_t);
            fade_in[i] = static_cast<float>(compressed_t);
        } else {
            fade_out[i] = static_cast<float>(1.0 - smooth_t);
            fade_in[i] = static_cast<float>(smooth_t);
        }

        fade_out[i] *= static_cast<float>(std::sqrt(energy_ratio));
        fade_in[i] /= static_cast<float>(std::sqrt(energy_ratio));
    }

    return {fade_out, fade_in};
}

double LiveSortAlgorithm::calculateEnergyTransition(double from_energy,
                                                     double to_energy) {
    double diff = std::abs(from_energy - to_energy);
    double max_allowed_jump = 0.3;

    if (diff <= max_allowed_jump) {
        return 1.0 - (diff / max_allowed_jump) * 0.3;
    } else {
        return 0.7 - (diff - max_allowed_jump) * 0.5;
    }
}

double LiveSortAlgorithm::calculateBPMCompatibility(double from_bpm, double to_bpm) {
    double ratio = std::max(from_bpm, to_bpm) / (std::min(from_bpm, to_bpm) + 1e-6);

    if (ratio <= 1.1) {
        return 1.0;
    } else if (ratio <= 1.3) {
        return 1.0 - (ratio - 1.1) * 1.5;
    } else if (ratio <= 2.0) {
        double harmonic_compatibility = std::abs(std::log2(ratio) - std::round(std::log2(ratio)));
        return harmonic_compatibility * 0.6;
    } else {
        return 0.1;
    }
}

double LiveSortAlgorithm::calculateStyleConsistency(const AudioFeatures& from,
                                                     const AudioFeatures& to) {
    double brightness_diff = std::abs(from.brightness - to.brightness);
    double valence_diff = std::abs(from.valence - to.valence);
    double danceability_diff = std::abs(from.danceability - to.danceability);

    double style_distance = brightness_diff * 0.3 +
                           valence_diff * 0.4 +
                           danceability_diff * 0.3;

    return 1.0 - style_distance;
}

double LiveSortAlgorithm::evaluateCurveFit(double current_value, double target_value) {
    double diff = std::abs(current_value - target_value);
    double tolerance = 0.15;

    if (diff <= tolerance) {
        return 1.0;
    } else {
        return 1.0 / (1.0 + std::pow((diff - tolerance) * 5.0, 2));
    }
}

std::vector<double> LiveSortAlgorithm::generateIdealCurve(size_t length) {
    std::vector<double> curve(length);

    if (length == 0) return curve;

    double warmup_end = 0.15;
    double build_start = 0.55;
    double climax_point = 0.85;

    for (size_t i = 0; i < length; ++i) {
        double t = static_cast<double>(i) / (length - 1);

        double value;

        if (t < warmup_end) {
            double progress = t / warmup_end;
            value = 0.25 + progress * 0.20;
        } else if (t < build_start) {
            double progress = (t - warmup_end) / (build_start - warmup_end);
            value = 0.45 - progress * 0.10;
        } else if (t < climax_point) {
            double progress = (t - build_start) / (climax_point - build_start);
            value = 0.35 + progress * 0.50;
        } else {
            double progress = (t - climax_point) / (1.0 - climax_point);
            value = 0.85 - progress * 0.15;
        }

        curve[i] = std::max(0.1, std::min(1.0, value));
    }

    return curve;
}
