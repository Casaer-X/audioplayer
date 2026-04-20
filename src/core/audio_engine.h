#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
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
    bool isLoading() const;

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
    void loadCompleted(bool success);
    void errorOccurred(const QString& errorMsg);

private slots:
    void onMediaPlayerStateChanged(QMediaPlayer::PlaybackState newState);
    void onMediaPlayerPositionChanged(qint64 position);
    void onMediaPlayerDurationChanged(qint64 duration);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onErrorOccurred(QMediaPlayer::Error error, const QString& errorString);
    void updatePosition();
    void handleAboutToFinish();
    void doPendingPlay();

private:
    void initializeEngine();
    void cleanupEngine();

    PlaybackState m_state;
    PlayMode m_playMode;
    int m_volume;
    bool m_muted;

    double m_currentPosition;
    double m_duration;
    QString m_currentFile;

    QMediaPlayer* m_mediaPlayer;
    QAudioOutput* m_audioOutput;

    QTimer* m_positionTimer;
    QTimer* m_crossfadeTimer;

    bool m_liveSortEnabled;
    double m_crossfadeDuration;
    TransitionParams m_transitionParams;

    std::vector<float> m_fadeOutCurve;
    std::vector<float> m_fadeInCurve;
    bool m_isCrossfading;

    bool m_isLoading;
    bool m_pendingPlay;
};

#endif
