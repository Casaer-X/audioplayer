#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QPushButton>

class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);

signals:
    void libraryRequested(const QString& category);
    void playlistRequested(int index);
    void settingsRequested();
    void addPlaylistRequested();

private:
    void setupUI();
    void onLibraryItemClicked(QTreeWidgetItem* item, int column);
    void onPlaylistItemClicked(QListWidgetItem* item);

    QTreeWidget* m_libraryTree;
    QListWidget* m_playlistList;
    QPushButton* m_addPlaylistButton;
};

#endif
