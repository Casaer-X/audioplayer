#include "right_panel.h"
#include <QPushButton>
#include <QSlider>

RightPanel::RightPanel(Player* player, QWidget* parent)
    : QWidget(parent)
    , m_player(player) {
    setupUI();
}

void RightPanel::setupUI() {
    setStyleSheet(R"(
        RightPanel {
            background-color: #252526;
            border-left: 1px solid #3C3C3C;
        }

        QTabWidget::pane {
            border: none;
            background-color: #252526;
        }

        QTabBar::tab {
            background-color: #2D2D2D;
            color: #969696;
            padding: 6px 12px;
            margin-right: 1px;
            font-size: 11px;
        }

        QTabBar::tab:selected {
            background-color: #252526;
            color: #FFFFFF;
            border-top: 2px solid #007ACC;
        }

        QLabel {
            color: #CCCCCC;
            background-color: transparent;
        }

        QPushButton {
            background-color: #0E639C;
            color: #FFFFFF;
            border: none;
            padding: 8px 16px;
            border-radius: 3px;
            font-size: 13px;
        }

        QPushButton:hover {
            background-color: #1177BB;
        }

        QPushButton:pressed {
            background-color: #094771;
        }

        QSlider::groove:horizontal {
            height: 4px;
            background-color: #3C3C3C;
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            width: 14px;
            height: 14px;
            margin: -5px 0;
            background-color: #007ACC;
            border-radius: 7px;
        }

        QSlider::sub-page:horizontal {
            background-color: #007ACC;
            border-radius: 2px;
        }
    )");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    auto* tabWidget = new QTabWidget(this);

    auto* nowPlayingPage = new QWidget();
    auto* nowPlayingLayout = new QVBoxLayout(nowPlayingPage);
    nowPlayingLayout->setAlignment(Qt::AlignCenter);
    nowPlayingLayout->setSpacing(15);

    m_coverLabel = new QLabel("封面");
    m_coverLabel->setFixedSize(180, 180);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #3C3C3C;"
        "   border: 2px solid #007ACC;"
        "   border-radius: 5px;"
        "   color: #666666;"
        "   font-size: 24px;"
        "}"
    );
    nowPlayingLayout->addWidget(m_coverLabel, 0, Qt::AlignCenter);

    m_titleLabel = new QLabel("未在播放");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    nowPlayingLayout->addWidget(m_titleLabel);

    m_artistLabel = new QLabel("");
    m_artistLabel->setAlignment(Qt::AlignCenter);
    m_artistLabel->setStyleSheet("color: #AAAAAA; font-size: 13px;");
    nowPlayingLayout->addWidget(m_artistLabel);

    m_albumLabel = new QLabel("");
    m_albumLabel->setAlignment(Qt::AlignCenter);
    m_albumLabel->setStyleSheet("color: #888888; font-size: 12px;");
    nowPlayingLayout->addWidget(m_albumLabel);

    nowPlayingLayout->addStretch();

    tabWidget->addTab(nowPlayingPage, "正在播放");

    auto* lyricsPage = new QLabel("歌词显示\n功能开发中...");
    lyricsPage->setAlignment(Qt::AlignCenter);
    lyricsPage->setStyleSheet("color: #666666; font-size: 14px;");
    tabWidget->addTab(lyricsPage, "歌词");

    auto* effectsPage = new QLabel("音效设置\n功能开发中...");
    effectsPage->setAlignment(Qt::AlignCenter);
    effectsPage->setStyleSheet("color: #666666; font-size: 14px;");
    tabWidget->addTab(effectsPage, "音效");

    layout->addWidget(tabWidget);

    connect(m_player->engine(), &AudioEngine::currentTrackChanged,
            this, [this](const QString& filepath) {
        Q_UNUSED(filepath)
        updateNowPlaying("", "", "");
    });
}

void RightPanel::updateNowPlaying(const QString& title,
                                   const QString& artist,
                                   const QString& album) {
    m_titleLabel->setText(title.isEmpty() ? "未在播放" : title);
    m_artistLabel->setText(artist);
    m_albumLabel->setText(album);
}

void RightPanel::onPlayPauseClicked() {
    if (m_player->engine()->state() == PlaybackState::Playing) {
        m_player->engine()->pause();
    } else {
        m_player->engine()->play();
    }
}

void RightPanel::onPreviousClicked() {
    m_player->playPrevious();
}

void RightPanel::onNextClicked() {
    m_player->playNext();
}
