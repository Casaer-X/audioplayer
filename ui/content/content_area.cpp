#include "content_area.h"
#include <QHeaderView>
#include <QLabel>
#include <QFont>

ContentArea::ContentArea(Player* player, QWidget* parent)
    : QWidget(parent)
    , m_player(player) {
    setupUI();
}

void ContentArea::setupUI() {
    setStyleSheet(R"(
        ContentArea {
            background-color: #1E1E1E;
        }

        QTabWidget::pane {
            border: none;
            background-color: #1E1E1E;
        }

        QTabBar::tab {
            background-color: #2D2D2D;
            color: #969696;
            padding: 8px 16px;
            margin-right: 1px;
            border-top-left-radius: 3px;
            border-top-right-radius: 3px;
            font-size: 12px;
        }

        QTabBar::tab:selected {
            background-color: #1E1E1E;
            color: #FFFFFF;
            border-top: 2px solid #007ACC;
        }

        QTabBar::tab:hover:!selected {
            background-color: #353535;
        }

        QTableWidget {
            background-color: #1E1E1E;
            color: #CCCCCC;
            gridline-color: #3C3C3C;
            border: none;
            selection-background-color: #094771;
            selection-color: #FFFFFF;
        }

        QTableWidget::item {
            padding: 5px;
        }

        QTableWidget::item:selected {
            background-color: #094771;
        }

        QHeaderView::section {
            background-color: #252526;
            color: #CCCCCC;
            padding: 6px 10px;
            border: none;
            border-right: 1px solid #3C3C3C;
            border-bottom: 1px solid #3C3C3C;
            font-weight: bold;
            font-size: 11px;
        }
    )");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    auto* songListPage = new QWidget();
    auto* songListLayout = new QVBoxLayout(songListPage);
    songListLayout->setContentsMargins(0, 0, 0, 0);

    setupTable();

    songListLayout->addWidget(m_songTable);
    m_tabWidget->addTab(songListPage, "歌曲列表");

    auto* lyricsPage = new QLabel("歌词显示\n功能开发中...");
    lyricsPage->setAlignment(Qt::AlignCenter);
    lyricsPage->setStyleSheet("color: #666666; font-size: 16px;");
    m_tabWidget->addTab(lyricsPage, "歌词");

    auto* visualizerPage = new QLabel("音频可视化\n功能开发中...");
    visualizerPage->setAlignment(Qt::AlignCenter);
    visualizerPage->setStyleSheet("color: #666666; font-size: 16px;");
    m_tabWidget->addTab(visualizerPage, "可视化");

    layout->addWidget(m_tabWidget);

    connect(m_songTable, &QTableWidget::cellDoubleClicked,
            this, &ContentArea::onSongDoubleClicked);
}

void ContentArea::setupTable() {
    m_songTable = new QTableWidget(this);
    m_songTable->setColumnCount(5);
    m_songTable->setHorizontalHeaderLabels({"#", "曲目", "艺术家", "专辑", "时长"});
    m_songTable->horizontalHeader()->setStretchLastSection(true);
    m_songTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_songTable->verticalHeader()->setVisible(false);
    m_songTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_songTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_songTable->setAlternatingRowColors(false);
    m_songTable->setShowGrid(false);

    m_songTable->setColumnWidth(0, 50);
    m_songTable->setColumnWidth(1, 300);
    m_songTable->setColumnWidth(2, 150);
    m_songTable->setColumnWidth(3, 200);
}

void ContentArea::updateSongList(const QVector<Song>& songs) {
    m_songs = songs;

    m_songTable->setRowCount(songs.size());

    for (int i = 0; i < songs.size(); ++i) {
        const Song& song = songs[i];

        auto* numItem = new QTableWidgetItem(QString::number(i + 1));
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setFlags(numItem->flags() & ~Qt::ItemIsEditable);
        m_songTable->setItem(i, 0, numItem);

        auto* titleItem = new QTableWidgetItem(song.title());
        titleItem->setFlags(titleItem->flags() & ~Qt::ItemIsEditable);
        m_songTable->setItem(i, 1, titleItem);

        auto* artistItem = new QTableWidgetItem(song.artist());
        artistItem->setFlags(artistItem->flags() & ~Qt::ItemIsEditable);
        m_songTable->setItem(i, 2, artistItem);

        auto* albumItem = new QTableWidgetItem(song.album());
        albumItem->setFlags(albumItem->flags() & ~Qt::ItemIsEditable);
        m_songTable->setItem(i, 3, albumItem);

        QString durationText;
        if (song.duration() > 0) {
            int seconds = song.duration() / 1000;
            int minutes = seconds / 60;
            int secs = seconds % 60;
            durationText = QString("%1:%2")
                .arg(minutes)
                .arg(secs, 2, 10, QChar('0'));
        } else {
            durationText = "--:--";
        }

        auto* durationItem = new QTableWidgetItem(durationText);
        durationItem->setTextAlignment(Qt::AlignCenter);
        durationItem->setFlags(durationItem->flags() & ~Qt::ItemIsEditable);
        m_songTable->setItem(i, 4, durationItem);
    }
}

void ContentArea::setCurrentSong(const Song& song) {
    for (int i = 0; i < m_songs.size(); ++i) {
        if (m_songs[i] == song) {
            m_songTable->selectRow(i);
            break;
        }
    }
}

void ContentArea::onSongDoubleClicked(int row, int column) {
    Q_UNUSED(column)

    if (row >= 0 && row < m_songs.size()) {
        emit songDoubleClicked(m_songs[row]);
    }
}
