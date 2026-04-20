#include "audio_engine.h"
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QDebug>
#include <cmath>
#include <algorithm>

AudioEngine::AudioEngine(QObject* parent)
    : QObject(parent)
    , m_state(PlaybackState::Stopped)
    , m_playMode(PlayMode::Sequential)
    , m_volume(80)
    , m_muted(false)
    , m_currentPosition(0.0)
    , m_duration(0.0)
    , m_mediaPlayer(nullptr)
    , m_audioOutput(nullptr)
    , m_positionTimer(new QTimer(this))
    , m_crossfadeTimer(new QTimer(this))
    , m_liveSortEnabled(true)
    , m_crossfadeDuration(3.0)
    , m_isCrossfading(false)
    , m_isLoading(false)
    , m_pendingPlay(false) {

    m_transitionParams = LiveSortAlgorithm::getDefaultParams();

    connect(m_positionTimer, &QTimer::timeout, this, &AudioEngine::updatePosition);
    connect(m_crossfadeTimer, &QTimer::timeout, this, &AudioEngine::handleAboutToFinish);

    initializeEngine();
}

AudioEngine::~AudioEngine() {
    cleanupEngine();
}

void AudioEngine::initializeEngine() {
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);

    m_mediaPlayer->setAudioOutput(m_audioOutput);

    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &AudioEngine::onMediaPlayerStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
            this, &AudioEngine::onMediaPlayerPositionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged,
            this, &AudioEngine::onMediaPlayerDurationChanged);
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &AudioEngine::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred,
            this, &AudioEngine::onErrorOccurred);

    m_audioOutput->setVolume(m_volume / 100.0);

    m_positionTimer->setInterval(100);
    m_crossfadeTimer->setSingleShot(true);
}

void AudioEngine::cleanupEngine() {
    m_pendingPlay = false;
    stop();
}

bool AudioEngine::loadFile(const QString& filepath) {
    if (m_isLoading) return false;

    m_isLoading = true;
    m_pendingPlay = false;
    m_currentFile = filepath;
    m_currentPosition = 0.0;
    m_duration = 0.0;
    m_isCrossfading = false;

    m_crossfadeTimer->stop();
    m_positionTimer->stop();

    m_mediaPlayer->setSource(QUrl::fromLocalFile(filepath));
    emit currentTrackChanged(filepath);

    return true;
}

void AudioEngine::doPendingPlay() {
    m_pendingPlay = false;
    if (!m_currentFile.isEmpty() && m_isLoading) {
        m_mediaPlayer->play();
    }
}

void AudioEngine::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::LoadedMedia) {
        m_duration = m_mediaPlayer->duration() / 1000.0;
        if (m_duration > 0) {
            emit durationChanged(m_duration);
        }
        m_isLoading = false;
        emit loadCompleted(true);

        if (m_pendingPlay) {
            QTimer::singleShot(0, this, &AudioEngine::doPendingPlay);
        }
    } else if (status == QMediaPlayer::InvalidMedia) {
        m_isLoading = false;
        m_pendingPlay = false;
        emit loadCompleted(false);
        emit errorOccurred("无法加载媒体文件: " + m_currentFile);
    }
}

void AudioEngine::onErrorOccurred(QMediaPlayer::Error error, const QString& errorString) {
    Q_UNUSED(error)
    m_isLoading = false;
    m_pendingPlay = false;
    emit errorOccurred(errorString);
    emit loadCompleted(false);
}

void AudioEngine::play() {
    if (m_currentFile.isEmpty()) return;

    if (m_isLoading) {
        m_pendingPlay = true;
    } else {
        m_mediaPlayer->play();
    }
}

void AudioEngine::pause() {
    if (m_state != PlaybackState::Playing) return;
    m_pendingPlay = false;
    m_mediaPlayer->pause();
}

void AudioEngine::stop() {
    m_pendingPlay = false;
    m_crossfadeTimer->stop();
    m_isCrossfading = false;
    m_positionTimer->stop();

    if (m_mediaPlayer) {
        m_mediaPlayer->stop();
    }
}

void AudioEngine::next() {
    emit aboutToFinish();
}

void AudioEngine::previous() {
    if (m_currentPosition > 3.0) {
        seek(0);
    } else {
        emit aboutToFinish();
    }
}

void AudioEngine::seek(double position) {
    if (m_mediaPlayer && m_duration > 0) {
        qint64 posMs = static_cast<qint64>(position * 1000);
        m_mediaPlayer->setPosition(posMs);
    }
    m_currentPosition = std::max(0.0, std::min(position, m_duration));
    emit positionChanged(m_currentPosition);
}

void AudioEngine::setVolume(int volume) {
    m_volume = std::max(0, std::min(100, volume));
    if (m_audioOutput) {
        m_audioOutput->setVolume(m_muted ? 0 : m_volume / 100.0);
    }
    emit volumeChanged(m_volume);
}

int AudioEngine::volume() const {
    return m_muted ? 0 : m_volume;
}

void AudioEngine::setMuted(bool muted) {
    m_muted = muted;
    if (m_audioOutput) {
        m_audioOutput->setVolume(muted ? 0 : m_volume / 100.0);
    }
    emit volumeChanged(volume());
}

bool AudioEngine::isMuted() const {
    return m_muted;
}

void AudioEngine::setPlayMode(PlayMode mode) {
    m_playMode = mode;
}

PlayMode AudioEngine::playMode() const {
    return m_playMode;
}

double AudioEngine::currentPosition() const {
    return m_currentPosition;
}

double AudioEngine::duration() const {
    return m_duration;
}

PlaybackState AudioEngine::state() const {
    return m_state;
}

QString AudioEngine::currentFile() const {
    return m_currentFile;
}

bool AudioEngine::isLoading() const {
    return m_isLoading;
}

void AudioEngine::enableLiveSortTransition(bool enable) {
    m_liveSortEnabled = enable;
}

void AudioEngine::setCrossfadeDuration(double seconds) {
    m_crossfadeDuration = std::max(0.5, std::min(10.0, seconds));
}

void AudioEngine::onMediaPlayerStateChanged(QMediaPlayer::PlaybackState newState) {
    switch (newState) {
        case QMediaPlayer::PlayingState:
            m_state = PlaybackState::Playing;
            m_positionTimer->start();
            break;

        case QMediaPlayer::PausedState:
            m_state = PlaybackState::Paused;
            m_positionTimer->stop();
            break;

        case QMediaPlayer::StoppedState:
            m_state = PlaybackState::Stopped;
            m_currentPosition = 0.0;
            m_positionTimer->stop();
            m_crossfadeTimer->stop();
            m_isCrossfading = false;
            m_pendingPlay = false;
            emit stateChanged(m_state);
            emit positionChanged(0.0);
            if (!m_isLoading) {
                emit finished();
            }
            return;
    }

    emit stateChanged(m_state);
}

void AudioEngine::onMediaPlayerPositionChanged(qint64 position) {
    m_currentPosition = position / 1000.0;

    if (m_liveSortEnabled && !m_isCrossfading && m_duration > 0) {
        double remaining = m_duration - m_currentPosition;
        if (remaining <= m_crossfadeDuration && remaining > 0) {
            m_isCrossfading = true;
            m_crossfadeTimer->start(static_cast<int>(remaining * 1000));
        }
    }
}

void AudioEngine::onMediaPlayerDurationChanged(qint64 duration) {
    m_duration = duration / 1000.0;
    emit durationChanged(m_duration);
}

void AudioEngine::updatePosition() {
    if (m_state == PlaybackState::Playing && m_mediaPlayer) {
        m_currentPosition = m_mediaPlayer->position() / 1000.0;
        emit positionChanged(m_currentPosition);
    }
}

void AudioEngine::handleAboutToFinish() {
    if ((m_state == PlaybackState::Playing || m_isCrossfading) && !m_isLoading) {
        emit aboutToFinish();
        m_isCrossfading = false;
    }
}
