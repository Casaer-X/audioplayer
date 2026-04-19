#include "playlist.h"

Playlist::Playlist() {
}

Playlist::Playlist(const QString& name)
    : m_name(name) {
}

QString Playlist::name() const { return m_name; }
void Playlist::setName(const QString& name) { m_name = name; }

QString Playlist::description() const { return m_description; }
void Playlist::setDescription(const QString& description) { m_description = description; }

QVector<Song> Playlist::songs() const { return m_songs; }
void Playlist::setSongs(const QVector<Song>& songs) { m_songs = songs; }

void Playlist::addSong(const Song& song) {
    m_songs.append(song);
}

void Playlist::removeSong(int index) {
    if (index >= 0 && index < m_songs.size()) {
        m_songs.removeAt(index);
    }
}

void Playlist::insertSong(int index, const Song& song) {
    if (index >= 0 && index <= m_songs.size()) {
        m_songs.insert(index, song);
    } else {
        m_songs.append(song);
    }
}

void Playlist::clear() {
    m_songs.clear();
}

int Playlist::count() const {
    return m_songs.size();
}

bool Playlist::isEmpty() const {
    return m_songs.isEmpty();
}

qint64 Playlist::totalDuration() const {
    qint64 total = 0;
    for (const auto& song : m_songs) {
        total += song.duration();
    }
    return total;
}

int Playlist::indexOf(const Song& song) const {
    return m_songs.indexOf(song);
}

Song Playlist::at(int index) const {
    if (index >= 0 && index < m_songs.size()) {
        return m_songs.at(index);
    }
    return Song();
}

Song Playlist::operator[](int index) const {
    return at(index);
}

void Playlist::move(int from, int to) {
    if (from < 0 || from >= m_songs.size() ||
        to < 0 || to >= m_songs.size()) {
        return;
    }

    Song song = m_songs.takeAt(from);
    m_songs.insert(to, song);
}
