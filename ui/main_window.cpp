#include "main_window.h"
#include "sidebar/sidebar.h"
#include "content/content_area.h"
#include "panel/right_panel.h"
#include "statusbar/status_bar.h"
#include "services/file_scanner.h"

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QKeyEvent>
#include <QShortcut>
#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_player(new Player(this)) {

    setupUI();
    setupMenuBar();
    setupShortcuts();
    applyTheme();
    loadSettings();

    setWindowTitle("MusicCode");
    resize(1280, 800);

    setMinimumSize(1024, 600);
}

MainWindow::~MainWindow() {
    saveSettings();
}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_contentSplitter = new QSplitter(Qt::Horizontal, this);

    m_sidebar = new Sidebar(this);
    m_contentArea = new ContentArea(m_player, this);
    m_rightPanel = new RightPanel(m_player, this);
    m_statusBar = new StatusBar(m_player->engine(), this);

    m_contentSplitter->addWidget(m_contentArea);
    m_contentSplitter->addWidget(m_rightPanel);
    m_contentSplitter->setStretchFactor(0, 3);
    m_contentSplitter->setStretchFactor(1, 1);

    m_mainSplitter->addWidget(m_sidebar);
    m_mainSplitter->addWidget(m_contentSplitter);
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(m_mainSplitter);

    setStatusBar(m_statusBar);

    QList<int> mainSizes;
    mainSizes << 250 << (width() - 250 - 300);
    m_mainSplitter->setSizes(mainSizes);

    QList<int> contentSizes;
    contentSizes << (width() - 550) << 300;
    m_contentSplitter->setSizes(contentSizes);

    connect(m_contentArea, &ContentArea::songDoubleClicked,
            this, [this](const Song& song) {
        m_player->playSong(song);
        m_contentArea->setCurrentSong(song);
    });

    connect(m_statusBar, &StatusBar::previousRequested,
            m_player, &Player::playPrevious);
    connect(m_statusBar, &StatusBar::nextRequested,
            m_player, &Player::playNext);

    connect(m_player, &Player::currentSongChanged,
            this, [this](const Song& song) {
        m_contentArea->setCurrentSong(song);
        QString title = song.title().isEmpty()
            ? QFileInfo(song.filepath()).completeBaseName()
            : song.title();
        m_rightPanel->updateNowPlaying(title, song.artist(), song.album());
        m_statusBar->updateCurrentTrack(song.filepath());
    });

    connect(m_sidebar, &Sidebar::libraryRequested,
            this, [](const QString& category) {
                qDebug() << "Library category clicked:" << category;
            });
    connect(m_sidebar, &Sidebar::playlistRequested,
            this, [](int index) {
                qDebug() << "Playlist index clicked:" << index;
            });
    connect(m_sidebar, &Sidebar::addPlaylistRequested,
            this, []() {
                qDebug() << "Add playlist requested";
            });
}

void MainWindow::setupMenuBar() {
    auto* menuBar = this->menuBar();

    auto* fileMenu = menuBar->addMenu("文件(&F)");
    fileMenu->addAction("打开文件...", QKeySequence::Open, this, &MainWindow::onOpenFile);
    fileMenu->addAction("打开文件夹...", this, &MainWindow::onOpenFolder);
    fileMenu->addSeparator();
    fileMenu->addAction("退出", QKeySequence::Quit, qApp, &QApplication::quit);

    auto* editMenu = menuBar->addMenu("编辑(&E)");
    editMenu->addAction("撤销", QKeySequence::Undo);
    editMenu->addAction("重做", QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction("剪切", QKeySequence::Cut);
    editMenu->addAction("复制", QKeySequence::Copy);
    editMenu->addAction("粘贴", QKeySequence::Paste);
    editMenu->addAction("全选", QKeySequence::SelectAll);

    auto* viewMenu = menuBar->addMenu("视图(&V)");
    viewMenu->addAction("切换侧边栏", Qt::CTRL | Qt::Key_B,
                        this, &MainWindow::onToggleSidebar);
    viewMenu->addAction("切换面板", Qt::CTRL | Qt::Key_J,
                        this, &MainWindow::onTogglePanel);

    auto* playbackMenu = menuBar->addMenu("播放(&P)");
    playbackMenu->addAction("播放/暂停", Qt::Key_Space,
                            [this]() { if (m_player->engine()->state() == PlaybackState::Playing)
                                          m_player->engine()->pause();
                                      else m_player->engine()->play(); });
    playbackMenu->addAction("上一首", Qt::Key_Left, [this]() { m_player->playPrevious(); });
    playbackMenu->addAction("下一首", Qt::Key_Right, [this]() { m_player->playNext(); });

    auto* helpMenu = menuBar->addMenu("帮助(&H)");
    helpMenu->addAction("设置", Qt::Key_Comma, this, &MainWindow::onSettings);
    helpMenu->addSeparator();
    helpMenu->addAction("关于 MusicCode", this, &MainWindow::onAbout);
}

void MainWindow::setupShortcuts() {
    auto* commandPalette = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P),
                                         this, this, &MainWindow::onCommandPalette);
    commandPalette->setContext(Qt::ApplicationShortcut);

    auto* quickOpen = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_P),
                                    this, this, &MainWindow::onQuickOpen);
    quickOpen->setContext(Qt::ApplicationShortcut);
}

void MainWindow::applyTheme() {
    QString styleSheet = R"(
        QMainWindow {
            background-color: #1E1E1E;
            color: #CCCCCC;
            font-family: 'Segoe UI', 'Consolas', sans-serif;
            font-size: 13px;
        }

        QMenuBar {
            background-color: #3C3C3C;
            color: #CCCCCC;
            padding: 2px;
            border-bottom: 1px solid #252526;
        }

        QMenuBar::item:selected {
            background-color: #094771;
            color: #FFFFFF;
        }

        QMenu {
            background-color: #252526;
            color: #CCCCCC;
            border: 1px solid #3C3C3C;
            padding: 5px;
        }

        QMenu::item:selected {
            background-color: #094771;
        }

        QSplitter::handle {
            background-color: #252526;
            width: 1px;
            height: 1px;
        }

        QSplitter::handle:hover {
            background-color: #007ACC;
        }

        QStatusBar {
            background-color: #007ACC;
            color: #FFFFFF;
            font-size: 12px;
            padding: 2px;
        }

        QWidget {
            outline: none;
        }
    )";

    setStyleSheet(styleSheet);
}

void MainWindow::loadSettings() {
    QSettings settings("MusicCode", "MusicCode");

    restoreGeometry(settings.value("window/geometry").toByteArray());
    restoreState(settings.value("window/state").toByteArray());

    int volume = settings.value("player/volume", 80).toInt();
    m_player->engine()->setVolume(volume);
}

void MainWindow::saveSettings() {
    QSettings settings("MusicCode", "MusicCode");

    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());
    settings.setValue("player/volume", m_player->engine()->volume());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Space:
            if (m_player->engine()->state() == PlaybackState::Playing) {
                m_player->engine()->pause();
            } else {
                m_player->engine()->play();
            }
            break;

        case Qt::Key_Left:
            if (event->modifiers() & Qt::ControlModifier) {
                m_player->playPrevious();
            } else {
                double newPos = m_player->engine()->currentPosition() - 5.0;
                m_player->engine()->seek(qMax(0.0, newPos));
            }
            break;

        case Qt::Key_Right:
            if (event->modifiers() & Qt::ControlModifier) {
                m_player->playNext();
            } else {
                double newPos = m_player->engine()->currentPosition() + 5.0;
                m_player->engine()->seek(newPos);
            }
            break;

        case Qt::Key_Up:
            m_player->engine()->setVolume(m_player->engine()->volume() + 5);
            break;

        case Qt::Key_Down:
            m_player->engine()->setVolume(m_player->engine()->volume() - 5);
            break;

        default:
            QMainWindow::keyPressEvent(event);
            return;
    }

    event->accept();
}

void MainWindow::onOpenFile() {
    QStringList filepaths = QFileDialog::getOpenFileNames(
        this,
        "选择音乐文件",
        QString(),
        "音频文件 (*.mp3 *.flac *.wav *.ogg *.m4a *.aac *.wma *.ape);;所有文件 (*)"
    );

    if (!filepaths.isEmpty()) {
        FileScanner scanner;
        QVector<Song> songs = scanner.scanFiles(filepaths);
        Playlist playlist("临时播放列表");
        for (const auto& song : songs) {
            playlist.addSong(song);
        }
        m_player->setPlaylist(playlist);

        if (!songs.isEmpty()) {
            m_player->playAt(0);
        }
    }
}

void MainWindow::onOpenFolder() {
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        "选择音乐文件夹",
        QString(),
        QFileDialog::ShowDirsOnly
    );

    if (!dirPath.isEmpty()) {
        FileScanner scanner;
        QVector<Song> songs = scanner.scanDirectory(dirPath);

        Playlist playlist(dirPath.section('/', -1));
        for (const auto& song : songs) {
            playlist.addSong(song);
        }
        m_player->setPlaylist(playlist);

        m_contentArea->updateSongList(songs);

        if (!songs.isEmpty()) {
            m_player->playAt(0);
        }
    }
}

void MainWindow::onCommandPalette() {
    QMessageBox::information(this, "命令面板",
                             "Ctrl+Shift+P 命令面板\n功能开发中...");
}

void MainWindow::onQuickOpen() {
    QMessageBox::information(this, "快速打开",
                             "Ctrl+P 快速打开\n功能开发中...");
}

void MainWindow::onToggleSidebar() {
    m_sidebar->setVisible(!m_sidebar->isVisible());
}

void MainWindow::onTogglePanel() {
    m_rightPanel->setVisible(!m_rightPanel->isVisible());
}

void MainWindow::onSettings() {
    QMessageBox::information(this, "设置",
                             "设置对话框\n功能开发中...");
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "关于 MusicCode",
        "<h2>MusicCode v1.0</h2>"
        "<p>VSCode风格的本地音乐播放器</p>"
        "<p>特性:</p>"
        "<ul>"
        "<li>LiveSort 智能过渡算法</li>"
        "<li>无损音频压缩</li>"
        "<li>BPM/能量分析</li>"
        "</ul>"
        "<p>基于 Qt 6 开发</p>");
}
