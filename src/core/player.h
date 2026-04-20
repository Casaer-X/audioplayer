#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QString>
#include <QQueue>
#include "core/audio_engine.h"
#include "models/song.h"
#include "models/playlist.h"

class Player : public QObject {
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr);

    void setPlaylist(const Playlist& playlist);
    void addToQueue(const Song& song);
    void clearQueue();

    void playAt(int index);
    void playSong(const Song& song);
    void playNext();
    void playPrevious();

    void shufflePlaylist();
    void sortByLiveSort();
    void setLiveSortEnabled(bool enabled);
    bool isLiveSortEnabled() const;

    AudioEngine* engine() const;
    const Playlist& currentPlaylist() const;
    int currentIndex() const;

signals:
    void currentSongChanged(const Song& song);
    void playlistChanged(const Playlist& playlist);
    void queueUpdated(int queueSize);

private slots:
    void handleAboutToFinish();
    void handleFinished();
    void onLoadCompleted(bool success);

private:
    AudioEngine* m_engine;
    Playlist m_playlist;
    QQueue<Song> m_queue;
    int m_currentIndex;
    bool m_liveSortEnabled;

    int getNextIndex();
    int getPreviousIndex();
};

#endif
