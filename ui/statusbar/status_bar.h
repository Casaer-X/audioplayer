#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <QStatusBar>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include "core/audio_engine.h"

class StatusBar : public QStatusBar {
    Q_OBJECT

public:
    explicit StatusBar(AudioEngine* engine, QWidget* parent = nullptr);

    void updateCurrentTrack(const QString& filepath);

signals:
    void previousRequested();
    void nextRequested();

private slots:
    void onPlayPauseClicked();
    void onPreviousClicked();
    void onNextClicked();
    void onVolumeChanged(int value);
    void updatePosition(double position);
    void updateDuration(double duration);
    void updateState(PlaybackState state);
    void updatePlayMode(PlayMode mode);

private:
    void setupUI();

    AudioEngine* m_engine;

    QLabel* m_trackInfoLabel;
    QPushButton* m_playPauseButton;
    QPushButton* m_prevButton;
    QPushButton* m_nextButton;
    QSlider* m_seekSlider;
    QSlider* m_volumeSlider;
    QLabel* m_positionLabel;
    QLabel* m_modeLabel;
};

#endif
