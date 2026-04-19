#ifndef FILE_SCANNER_H
#define FILE_SCANNER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDir>
#include "models/song.h"

class FileScanner : public QObject {
    Q_OBJECT

public:
    explicit FileScanner(QObject* parent = nullptr);

    QVector<Song> scanDirectory(const QString& dirPath);
    QVector<Song> scanFiles(const QStringList& filepaths);

    void setSupportedFormats(const QStringList& formats);
    QStringList supportedFormats() const;

signals:
    void scanProgress(int current, int total);
    void songFound(const Song& song);
    void scanFinished(const QVector<Song>& songs);
    void scanError(const QString& error);

private:
    QStringList m_supportedFormats;

    bool isSupportedFormat(const QString& filepath) const;
    Song createSongFromFile(const QString& filepath) const;
    QVector<QString> findAudioFiles(const QDir& directory, bool recursive = true) const;
};

#endif
