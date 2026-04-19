#include "sidebar.h"
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent) {
    setupUI();
}

void Sidebar::setupUI() {
    setStyleSheet(R"(
        Sidebar {
            background-color: #252526;
            border-right: 1px solid #3C3C3C;
        }

        QLabel {
            color: #BBBBBB;
            font-size: 11px;
            font-weight: bold;
            padding: 10px 5px 5px 10px;
            text-transform: uppercase;
        }

        QTreeWidget, QListWidget {
            background-color: #252526;
            color: #CCCCCC;
            border: none;
            outline: none;
            padding: 2px;
        }

        QTreeWidget::item, QListWidget::item {
            padding: 4px 8px;
            border-radius: 3px;
        }

        QTreeWidget::item:selected, QListWidget::item:selected {
            background-color: #094771;
            color: #FFFFFF;
        }

        QTreeWidget::item:hover, QListWidget::item:hover {
            background-color: #2A2D2E;
        }

        QPushButton {
            background-color: transparent;
            color: #CCCCCC;
            border: 1px solid transparent;
            text-align: left;
            padding: 6px 10px;
            font-size: 12px;
        }

        QPushButton:hover {
            background-color: #2A2D2E;
            border: 1px solid #3C3C3C;
        }
    )");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* explorerLabel = new QLabel("音乐库", this);
    layout->addWidget(explorerLabel);

    m_libraryTree = new QTreeWidget(this);
    m_libraryTree->setHeaderHidden(true);
    m_libraryTree->setRootIsDecorated(false);

    QStringList libraryItems = {"最近添加", "最常播放", "收藏", "艺术家", "专辑", "文件夹", "所有歌曲"};
    for (const auto& item : libraryItems) {
        auto* treeItem = new QTreeWidgetItem(m_libraryTree);
        treeItem->setText(0, item);
        treeItem->setIcon(0, QIcon(":/icons/folder.png"));
    }

    layout->addWidget(m_libraryTree);

    auto* playlistLabel = new QLabel("播放列表", this);
    layout->addWidget(playlistLabel);

    m_playlistList = new QListWidget(this);
    m_playlistList->addItem("我喜欢的音乐");
    m_playlistList->addItem("工作BGM");
    m_playlistList->addItem("运动歌单");
    layout->addWidget(m_playlistList);

    m_addPlaylistButton = new QPushButton("+ 新建播放列表", this);
    layout->addWidget(m_addPlaylistButton);

    layout->addStretch();

    connect(m_addPlaylistButton, &QPushButton::clicked,
            this, &Sidebar::addPlaylistRequested);
}
