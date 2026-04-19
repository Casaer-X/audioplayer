#include "song.h"
#include <QFileInfo>
#include <QDateTime>

Song::Song()
    : m_year(0)
    , m_trackNumber(0)
    , m_duration(0)
    , m_playCount(0)
    , m_rating(0)
    , m_favorite(false) {
}

Song::Song(const QString& filepath)
    : m_filepath(filepath)
    , m_title(QFileInfo(filepath).baseName())
    , m_year(0)
    , m_trackNumber(0)
    , m_duration(0)
    , m_playCount(0)
    , m_rating(0)
    , m_favorite(false) {
}

Song::Song(const QString& title, const QString& artist,
           const QString& album, const QString& filepath)
    : m_title(title)
    , m_artist(artist)
    , m_album(album)
    , m_filepath(filepath)
    , m_year(0)
    , m_trackNumber(0)
    , m_duration(0)
    , m_playCount(0)
    , m_rating(0)
    , m_favorite(false) {
}

bool Song::isValid() const {
    return !m_filepath.isEmpty() && QFileInfo::exists(m_filepath);
}

bool Song::operator==(const Song& other) const {
    return m_filepath == other.m_filepath;
}

bool Song::operator!=(const Song& other) const {
    return !(*this == other);
}

QString Song::title() const { return m_title; }
void Song::setTitle(const QString& title) { m_title = title; }

QString Song::artist() const { return m_artist; }
void Song::setArtist(const QString& artist) { m_artist = artist; }

QString Song::album() const { return m_album; }
void Song::setAlbum(const QString& album) { m_album = album; }

int Song::year() const { return m_year; }
void Song::setYear(int year) { m_year = year; }

int Song::trackNumber() const { return m_trackNumber; }
void Song::setTrackNumber(int number) { m_trackNumber = number; }

QString Song::genre() const { return m_genre; }
void Song::setGenre(const QString& genre) { m_genre = genre; }

qint64 Song::duration() const { return m_duration; }
void Song::setDuration(qint64 duration) { m_duration = duration; }

QString Song::filepath() const { return m_filepath; }

QPixmap Song::coverArt() const { return m_coverArt; }
void Song::setCoverArt(const QPixmap& cover) { m_coverArt = cover; }

int Song::playCount() const { return m_playCount; }
void Song::incrementPlayCount() { ++m_playCount; }

int Song::rating() const { return m_rating; }
void Song::setRating(int rating) { m_rating = qBound(0, rating, 5); }

bool Song::isFavorite() const { return m_favorite; }
void Song::setFavorite(bool favorite) { m_favorite = favorite; }
