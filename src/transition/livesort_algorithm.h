#ifndef LIVESORT_ALGORITHM_H
#define LIVESORT_ALGORITHM_H

#include "audio_analyzer.h"
#include <vector>
#include <string>

struct TransitionParams {
    double crossfade_duration;
    bool enable_energy_matching;
    bool enable_bpm_matching;
    double energy_weight;
    double bpm_weight;
    double style_weight;
};

struct SongPosition {
    size_t song_index;
    int position;
    double curve_fit_score;
};

class LiveSortAlgorithm {
public:
    static std::vector<size_t> optimizePlaylistOrder(
        const std::vector<AudioFeatures>& songs,
        TransitionParams params = getDefaultParams());

    static std::vector<double> generateEmotionalCurve(size_t length);

    static double calculateTransitionScore(const AudioFeatures& from,
                                           const AudioFeatures& to,
                                           const TransitionParams& params);

    static std::pair<std::vector<float>, std::vector<float>> calculateCrossfadeCurve(
        const AudioFeatures& from,
        const AudioFeatures& to,
        double crossfade_duration = 3.0);

    static TransitionParams getDefaultParams();

private:
    static double calculateEnergyTransition(double from_energy, double to_energy);
    static double calculateBPMCompatibility(double from_bpm, double to_bpm);
    static double calculateStyleConsistency(const AudioFeatures& from,
                                            const AudioFeatures& to);
    static double evaluateCurveFit(double current_value, double target_value);

    static std::vector<double> generateIdealCurve(size_t length);
};

#endif
