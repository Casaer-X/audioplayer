#ifndef CONTENT_AREA_H
#define CONTENT_AREA_H

#include <QWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include "core/player.h"
#include "models/song.h"

class ContentArea : public QWidget {
    Q_OBJECT

public:
    explicit ContentArea(Player* player, QWidget* parent = nullptr);

    void updateSongList(const QVector<Song>& songs);
    void setCurrentSong(const Song& song);

signals:
    void songDoubleClicked(const Song& song);
    void songSelected(int index);

private slots:
    void onSongDoubleClicked(int row, int column);

private:
    void setupUI();
    void setupTable();

    Player* m_player;
    QTabWidget* m_tabWidget;
    QTableWidget* m_songTable;
    QVector<Song> m_songs;
};

#endif
