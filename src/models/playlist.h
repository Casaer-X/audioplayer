#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QString>
#include <QVector>
#include "models/song.h"

class Playlist {
public:
    Playlist();
    explicit Playlist(const QString& name);

    QString name() const;
    void setName(const QString& name);

    QString description() const;
    void setDescription(const QString& description);

    QVector<Song> songs() const;
    void setSongs(const QVector<Song>& songs);
    void addSong(const Song& song);
    void removeSong(int index);
    void insertSong(int index, const Song& song);
    void clear();

    int count() const;
    bool isEmpty() const;

    qint64 totalDuration() const;
    int indexOf(const Song& song) const;
    Song at(int index) const;
    Song operator[](int index) const;

    void move(int from, int to);

private:
    QString m_name;
    QString m_description;
    QVector<Song> m_songs;
};

#endif
