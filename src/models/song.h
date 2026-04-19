#ifndef SONG_H
#define SONG_H

#include <QString>
#include <QPixmap>
#include <QMetaType>

class Song {
public:
    Song();
    Song(const QString& filepath);
    Song(const QString& title, const QString& artist,
         const QString& album, const QString& filepath);

    bool isValid() const;
    bool operator==(const Song& other) const;
    bool operator!=(const Song& other) const;

    QString title() const;
    void setTitle(const QString& title);

    QString artist() const;
    void setArtist(const QString& artist);

    QString album() const;
    void setAlbum(const QString& album);

    int year() const;
    void setYear(int year);

    int trackNumber() const;
    void setTrackNumber(int number);

    QString genre() const;
    void setGenre(const QString& genre);

    qint64 duration() const;
    void setDuration(qint64 duration);

    QString filepath() const;

    QPixmap coverArt() const;
    void setCoverArt(const QPixmap& cover);

    int playCount() const;
    void incrementPlayCount();

    int rating() const;
    void setRating(int rating);

    bool isFavorite() const;
    void setFavorite(bool favorite);

private:
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_filepath;
    QString m_genre;
    int m_year;
    int m_trackNumber;
    qint64 m_duration;
    QPixmap m_coverArt;
    int m_playCount;
    int m_rating;
    bool m_favorite;
};

Q_DECLARE_METATYPE(Song)

#endif
