#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "core/player.h"

class Sidebar;
class ContentArea;
class RightPanel;
class StatusBar;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onOpenFile();
    void onOpenFolder();
    void onCommandPalette();
    void onQuickOpen();
    void onToggleSidebar();
    void onTogglePanel();
    void onSettings();
    void onAbout();

private:
    void setupUI();
    void setupMenuBar();
    void setupShortcuts();
    void applyTheme();
    void loadSettings();
    void saveSettings();

    Player* m_player;

    QSplitter* m_mainSplitter;
    QSplitter* m_contentSplitter;

    Sidebar* m_sidebar;
    ContentArea* m_contentArea;
    RightPanel* m_rightPanel;
    StatusBar* m_statusBar;
};

#endif
