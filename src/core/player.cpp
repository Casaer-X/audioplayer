#include "player.h"
#include "transition/livesort_algorithm.h"
#include "transition/audio_analyzer.h"
#include <QRandomGenerator>
#include <QDebug>

Player::Player(QObject* parent)
    : QObject(parent)
    , m_engine(new AudioEngine(this))
    , m_currentIndex(-1)
    , m_liveSortEnabled(true) {

    connect(m_engine, &AudioEngine::aboutToFinish,
            this, &Player::handleAboutToFinish);
    connect(m_engine, &AudioEngine::finished,
            this, &Player::handleFinished);
}

void Player::setPlaylist(const Playlist& playlist) {
    m_playlist = playlist;
    m_currentIndex = -1;

    if (m_liveSortEnabled && m_playlist.songs().size() > 1) {
        sortByLiveSort();
    } else {
        emit playlistChanged(m_playlist);
    }
}

void Player::addToQueue(const Song& song) {
    m_queue.enqueue(song);
    emit queueUpdated(m_queue.size());
}

void Player::clearQueue() {
    m_queue.clear();
    emit queueUpdated(0);
}

void Player::playAt(int index) {
    if (index < 0 || index >= m_playlist.songs().size()) return;

    m_currentIndex = index;
    Song song = m_playlist.songs().at(index);

    if (m_engine->loadFile(song.filepath())) {
        m_engine->play();
        emit currentSongChanged(song);
    }
}

void Player::playSong(const Song& song) {
    int index = m_playlist.indexOf(song);

    if (index >= 0) {
        playAt(index);
    } else {
        addToQueue(song);
        if (m_engine->loadFile(song.filepath())) {
            m_engine->play();
            emit currentSongChanged(song);
        }
    }
}

void Player::playNext() {
    if (!m_queue.isEmpty()) {
        Song nextSong = m_queue.dequeue();
        playSong(nextSong);
        emit queueUpdated(m_queue.size());
        return;
    }

    int nextIndex = getNextIndex();
    if (nextIndex >= 0) {
        playAt(nextIndex);
    }
}

void Player::playPrevious() {
    if (m_engine->currentPosition() > 3.0) {
        m_engine->seek(0);
    } else {
        int prevIndex = getPreviousIndex();
        if (prevIndex >= 0) {
            playAt(prevIndex);
        }
    }
}

void Player::shufflePlaylist() {
    auto songs = m_playlist.songs();

    for (int i = songs.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        std::swap(songs[i], songs[j]);
    }

    m_playlist.setSongs(songs);
    emit playlistChanged(m_playlist);
}

void Player::sortByLiveSort() {
    auto songs = m_playlist.songs();

    if (songs.size() <= 1) {
        emit playlistChanged(m_playlist);
        return;
    }

    std::vector<AudioFeatures> features;
    for (const auto& song : songs) {
        features.push_back(AudioAnalyzer::analyze(song.filepath().toStdString()));
    }

    auto optimized_order = LiveSortAlgorithm::optimizePlaylistOrder(features);

    std::vector<Song> sorted_songs;
    for (size_t idx : optimized_order) {
        sorted_songs.push_back(songs[static_cast<int>(idx)]);
    }

    m_playlist.setSongs(
        QVector<Song>(sorted_songs.begin(), sorted_songs.end())
    );

    emit playlistChanged(m_playlist);
}

void Player::setLiveSortEnabled(bool enabled) {
    m_liveSortEnabled = enabled;
    m_engine->enableLiveSortTransition(enabled);
}

bool Player::isLiveSortEnabled() const {
    return m_liveSortEnabled;
}

AudioEngine* Player::engine() const {
    return m_engine;
}

const Playlist& Player::currentPlaylist() const {
    return m_playlist;
}

int Player::currentIndex() const {
    return m_currentIndex;
}

int Player::getNextIndex() {
    if (m_playlist.songs().isEmpty()) return -1;

    switch (m_engine->playMode()) {
        case PlayMode::Sequential:
            if (m_currentIndex < m_playlist.songs().size() - 1) {
                return m_currentIndex + 1;
            }
            return -1;

        case PlayMode::LoopSingle:
            return m_currentIndex;

        case PlayMode::LoopAll:
            return (m_currentIndex + 1) % m_playlist.songs().size();

        case PlayMode::Shuffle:
            return QRandomGenerator::global()->bounded(
                m_playlist.songs().size()
            );

        default:
            return -1;
    }
}

int Player::getPreviousIndex() {
    if (m_playlist.songs().isEmpty()) return -1;

    switch (m_engine->playMode()) {
        case PlayMode::Sequential:
            if (m_currentIndex > 0) {
                return m_currentIndex - 1;
            }
            return -1;

        case PlayMode::LoopSingle:
            return m_currentIndex;

        case PlayMode::LoopAll:
            return (m_currentIndex - 1 + m_playlist.songs().size()) %
                   m_playlist.songs().size();

        case PlayMode::Shuffle:
            return QRandomGenerator::global()->bounded(
                m_playlist.songs().size()
            );

        default:
            return -1;
    }
}

void Player::handleAboutToFinish() {
    playNext();
}

void Player::handleFinished() {
    if (!m_engine->isLoading()) {
        playNext();
    }
}

void Player::onLoadCompleted(bool success) {
    Q_UNUSED(success)
}
