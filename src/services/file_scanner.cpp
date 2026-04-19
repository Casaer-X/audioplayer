#include "file_scanner.h"
#include <QFileInfo>
#include <QFileInfoList>
#include <QDebug>

FileScanner::FileScanner(QObject* parent)
    : QObject(parent) {
    m_supportedFormats = {
        "mp3", "flac", "wav", "ogg", "m4a",
        "aac", "wma", "ape", "opus", "aiff"
    };
}

QVector<Song> FileScanner::scanDirectory(const QString& dirPath) {
    QVector<Song> songs;

    QDir directory(dirPath);
    if (!directory.exists()) {
        emit scanError("目录不存在: " + dirPath);
        return songs;
    }

    auto audioFiles = findAudioFiles(directory);

    int total = audioFiles.size();
    for (int i = 0; i < total; ++i) {
        Song song = createSongFromFile(audioFiles[i]);
        if (song.isValid()) {
            songs.append(song);
            emit songFound(song);
        }

        if (i % 10 == 0) {
            emit scanProgress(i + 1, total);
        }
    }

    emit scanProgress(total, total);
    emit scanFinished(songs);

    return songs;
}

QVector<Song> FileScanner::scanFiles(const QStringList& filepaths) {
    QVector<Song> songs;

    int total = filepaths.size();
    for (int i = 0; i < total; ++i) {
        const QString& filepath = filepaths[i];

        if (isSupportedFormat(filepath)) {
            Song song = createSongFromFile(filepath);
            if (song.isValid()) {
                songs.append(song);
                emit songFound(song);
            }
        }

        emit scanProgress(i + 1, total);
    }

    emit scanFinished(songs);

    return songs;
}

void FileScanner::setSupportedFormats(const QStringList& formats) {
    m_supportedFormats = formats;
}

QStringList FileScanner::supportedFormats() const {
    return m_supportedFormats;
}

bool FileScanner::isSupportedFormat(const QString& filepath) const {
    QFileInfo info(filepath);
    return m_supportedFormats.contains(info.suffix().toLower());
}

Song FileScanner::createSongFromFile(const QString& filepath) const {
    QFileInfo info(filepath);

    if (!info.exists() || !info.isFile()) {
        return Song();
    }

    Song song(filepath);
    song.setTitle(info.baseName());

    return song;
}

QVector<QString> FileScanner::findAudioFiles(const QDir& directory,
                                              bool recursive) const {
    QVector<QString> audioFiles;

    QFileInfoList entries = directory.entryInfoList(
        QDir::NoDotAndDotDot | QDir::AllEntries
    );

    for (const QFileInfo& entry : entries) {
        if (entry.isDir() && recursive) {
            QDir subDir(entry.absoluteFilePath());
            auto subFiles = findAudioFiles(subDir, recursive);
            audioFiles.append(subFiles);
        } else if (entry.isFile()) {
            if (isSupportedFormat(entry.absoluteFilePath())) {
                audioFiles.append(entry.absoluteFilePath());
            }
        }
    }

    return audioFiles;
}
