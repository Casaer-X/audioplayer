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
    , m_positionTimer(new QTimer(this))
    , m_crossfadeTimer(new QTimer(this))
    , m_liveSortEnabled(true)
    , m_crossfadeDuration(3.0)
    , m_isCrossfading(false) {

    m_transitionParams = LiveSortAlgorithm::getDefaultParams();

    connect(m_positionTimer, &QTimer::timeout, this, &AudioEngine::updatePosition);
    connect(m_crossfadeTimer, &QTimer::timeout, this, &AudioEngine::handleAboutToFinish);

    initializeEngine();
}

AudioEngine::~AudioEngine() {
    cleanupEngine();
}

void AudioEngine::initializeEngine() {
    m_positionTimer->setInterval(100);
    m_crossfadeTimer->setSingleShot(true);
}

void AudioEngine::cleanupEngine() {
    stop();
}

bool AudioEngine::loadFile(const QString& filepath) {
    stop();
    m_currentFile = filepath;

    QMediaPlayer tempPlayer;
    tempPlayer.setSource(QUrl::fromLocalFile(filepath));

    QEventLoop loop;
    connect(&tempPlayer, &QMediaPlayer::mediaStatusChanged,
            [&](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia ||
            status == QMediaPlayer::InvalidMedia) {
            loop.quit();
        }
    });
    loop.exec();

    if (tempPlayer.mediaStatus() == QMediaPlayer::LoadedMedia) {
        m_duration = tempPlayer.duration() / 1000.0;
        emit durationChanged(m_duration);
        return true;
    }

    return false;
}

void AudioEngine::play() {
    if (m_currentFile.isEmpty()) return;

    m_state = PlaybackState::Playing;
    m_positionTimer->start();

    if (m_isCrossfading && m_liveSortEnabled) {
        double remaining = m_duration - m_currentPosition;
        if (remaining <= m_crossfadeDuration && remaining > 0) {
            m_crossfadeTimer->start(static_cast<int>(remaining * 1000));
        }
    }

    emit stateChanged(m_state);
}

void AudioEngine::pause() {
    if (m_state != PlaybackState::Playing) return;

    m_state = PlaybackState::Paused;
    m_positionTimer->stop();

    emit stateChanged(m_state);
}

void AudioEngine::stop() {
    m_state = PlaybackState::Stopped;
    m_currentPosition = 0.0;
    m_positionTimer->stop();
    m_crossfadeTimer->stop();
    m_isCrossfading = false;

    emit stateChanged(m_state);
    emit positionChanged(0.0);
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
    m_currentPosition = std::max(0.0, std::min(position, m_duration));
    emit positionChanged(m_currentPosition);
}

void AudioEngine::setVolume(int volume) {
    m_volume = std::max(0, std::min(100, volume));
    emit volumeChanged(m_volume);
}

int AudioEngine::volume() const {
    return m_muted ? 0 : m_volume;
}

void AudioEngine::setMuted(bool muted) {
    m_muted = muted;
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

void AudioEngine::enableLiveSortTransition(bool enable) {
    m_liveSortEnabled = enable;
}

void AudioEngine::setCrossfadeDuration(double seconds) {
    m_crossfadeDuration = std::max(0.5, std::min(10.0, seconds));
}

void AudioEngine::updatePosition() {
    if (m_state == PlaybackState::Playing) {
        m_currentPosition += 0.1;

        if (m_currentPosition >= m_duration) {
            stop();
            emit finished();
        } else {
            emit positionChanged(m_currentPosition);

            if (m_liveSortEnabled && !m_isCrossfading) {
                double remaining = m_duration - m_currentPosition;
                if (remaining <= m_crossfadeDuration && remaining > 0) {
                    m_isCrossfading = true;
                    m_crossfadeTimer->start(static_cast<int>(remaining * 1000));
                }
            }
        }
    }
}

void AudioEngine::handleAboutToFinish() {
    if (m_state == PlaybackState::Playing || m_isCrossfading) {
        emit aboutToFinish();
        m_isCrossfading = false;
    }
}
