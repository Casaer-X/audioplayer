#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <vector>
#include "compression/lossless_compressor.h"
#include "transition/livesort_algorithm.h"

enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

enum class PlayMode {
    Sequential,
    LoopSingle,
    LoopAll,
    Shuffle
};

class AudioEngine : public QObject {
    Q_OBJECT

public:
    explicit AudioEngine(QObject* parent = nullptr);
    ~AudioEngine();

    bool loadFile(const QString& filepath);
    void play();
    void pause();
    void stop();
    void next();
    void previous();

    void seek(double position);
    void setVolume(int volume);
    int volume() const;
    void setMuted(bool muted);
    bool isMuted() const;

    void setPlayMode(PlayMode mode);
    PlayMode playMode() const;

    double currentPosition() const;
    double duration() const;
    PlaybackState state() const;
    QString currentFile() const;

    void enableLiveSortTransition(bool enable);
    void setCrossfadeDuration(double seconds);

signals:
    void stateChanged(PlaybackState state);
    void positionChanged(double position);
    void durationChanged(double duration);
    void volumeChanged(int volume);
    void currentTrackChanged(const QString& filepath);
    void aboutToFinish();
    void finished();

private slots:
    void updatePosition();
    void handleAboutToFinish();

private:
    PlaybackState m_state;
    PlayMode m_playMode;
    int m_volume;
    bool m_muted;

    double m_currentPosition;
    double m_duration;
    QString m_currentFile;

    QTimer* m_positionTimer;
    QTimer* m_crossfadeTimer;

    bool m_liveSortEnabled;
    double m_crossfadeDuration;
    TransitionParams m_transitionParams;

    std::vector<float> m_fadeOutCurve;
    std::vector<float> m_fadeInCurve;
    bool m_isCrossfading;

    void initializeEngine();
    void cleanupEngine();
};

#endif
