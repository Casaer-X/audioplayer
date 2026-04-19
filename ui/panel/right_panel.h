#ifndef RIGHT_PANEL_H
#define RIGHT_PANEL_H

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "core/player.h"

class RightPanel : public QWidget {
    Q_OBJECT

public:
    explicit RightPanel(Player* player, QWidget* parent = nullptr);

    void updateNowPlaying(const QString& title,
                          const QString& artist,
                          const QString& album);

private slots:
    void onPlayPauseClicked();
    void onPreviousClicked();
    void onNextClicked();

private:
    void setupUI();

    Player* m_player;

    QLabel* m_coverLabel;
    QLabel* m_titleLabel;
    QLabel* m_artistLabel;
    QLabel* m_albumLabel;
};

#endif
