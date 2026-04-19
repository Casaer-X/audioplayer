#include "status_bar.h"
#include <QHBoxLayout>
#include <QFileInfo>

StatusBar::StatusBar(AudioEngine* engine, QWidget* parent)
    : QStatusBar(parent)
    , m_engine(engine) {
    setupUI();

    connect(m_engine, &AudioEngine::positionChanged,
            this, &StatusBar::updatePosition);
    connect(m_engine, &AudioEngine::durationChanged,
            this, &StatusBar::updateDuration);
    connect(m_engine, &AudioEngine::stateChanged,
            this, &StatusBar::updateState);
    connect(m_engine, &AudioEngine::currentTrackChanged,
            this, &StatusBar::updateCurrentTrack);

    m_playPauseButton->connect(m_playPauseButton, &QPushButton::clicked,
                               this, &StatusBar::onPlayPauseClicked);
    m_prevButton->connect(m_prevButton, &QPushButton::clicked,
                          this, &StatusBar::onPreviousClicked);
    m_nextButton->connect(m_nextButton, &QPushButton::clicked,
                          this, &StatusBar::onNextClicked);
    m_volumeSlider->connect(m_volumeSlider, &QSlider::valueChanged,
                            this, &StatusBar::onVolumeChanged);
}

void StatusBar::setupUI() {
    setStyleSheet(R"(
        QStatusBar {
            background-color: #007ACC;
            color: #FFFFFF;
            font-size: 12px;
            padding: 2px 5px;
        }

        QLabel {
            color: #FFFFFF;
            background-color: transparent;
            padding: 0px 8px;
        }

        QPushButton {
            background-color: transparent;
            color: #FFFFFF;
            border: none;
            padding: 4px 8px;
            font-size: 14px;
            min-width: 20px;
        }

        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.1);
            border-radius: 3px;
        }

        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }

        QSlider::groove:horizontal {
            height: 4px;
            background-color: rgba(255, 255, 255, 0.3);
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            width: 12px;
            height: 12px;
            margin: -4px 0;
            background-color: #FFFFFF;
            border-radius: 6px;
        }

        QSlider::sub-page:horizontal {
            background-color: #FFFFFF;
            border-radius: 2px;
        }
    )");

    setContentsMargins(0, 0, 0, 0);

    auto* container = new QWidget(this);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(5, 2, 5, 2);
    layout->setSpacing(3);

    m_trackInfoLabel = new QLabel("▶ 未在播放", container);
    layout->addWidget(m_trackInfoLabel);

    layout->addSpacing(10);

    m_prevButton = new QPushButton("⏮", container);
    m_prevButton->setToolTip("上一首 (Ctrl+←)");
    layout->addWidget(m_prevButton);

    m_playPauseButton = new QPushButton("▶", container);
    m_playPauseButton->setToolTip("播放/暂停 (空格)");
    layout->addWidget(m_playPauseButton);

    m_nextButton = new QPushButton("⏭", container);
    m_nextButton->setToolTip("下一首 (Ctrl+→)");
    layout->addWidget(m_nextButton);

    layout->addSpacing(10);

    m_seekSlider = new QSlider(Qt::Horizontal, container);
    m_seekSlider->setRange(0, 1000);
    m_seekSlider->setValue(0);
    m_seekSlider->setMinimumWidth(200);
    m_seekSlider->setMaximumWidth(400);
    m_seekSlider->setToolTip("拖动调整播放位置");
    layout->addWidget(m_seekSlider);

    m_positionLabel = new QLabel("0:00 / 0:00", container);
    m_positionLabel->setMinimumWidth(90);
    layout->addWidget(m_positionLabel);

    layout->addStretch();

    m_volumeSlider = new QSlider(Qt::Horizontal, container);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(80);
    m_volumeSlider->setFixedWidth(100);
    m_volumeSlider->setToolTip("音量调节");
    layout->addWidget(m_volumeSlider);

    m_modeLabel = new QLabel("🔀 随机", container);
    layout->addWidget(m_modeLabel);

    addWidget(container, 1);
}

void StatusBar::onPlayPauseClicked() {
    if (m_engine->state() == PlaybackState::Playing) {
        m_engine->pause();
    } else {
        m_engine->play();
    }
}

void StatusBar::onPreviousClicked() {
    emit previousRequested();
}

void StatusBar::onNextClicked() {
    emit nextRequested();
}

void StatusBar::onVolumeChanged(int value) {
    m_engine->setVolume(value);
}

void StatusBar::updatePosition(double position) {
    if (m_engine->duration() > 0 && !m_seekSlider->isSliderDown()) {
        int sliderValue = static_cast<int>((position / m_engine->duration()) * 1000);
        m_seekSlider->blockSignals(true);
        m_seekSlider->setValue(sliderValue);
        m_seekSlider->blockSignals(false);
    }

    int currentSecs = static_cast<int>(position);
    int currentMins = currentSecs / 60;
    currentSecs = currentSecs % 60;

    int totalSecs = static_cast<int>(m_engine->duration());
    int totalMins = totalSecs / 60;
    totalSecs = totalSecs % 60;

    QString posText = QString("%1:%2 / %3:%4")
        .arg(currentMins)
        .arg(currentSecs, 2, 10, QChar('0'))
        .arg(totalMins)
        .arg(totalSecs, 2, 10, QChar('0'));

    m_positionLabel->setText(posText);
}

void StatusBar::updateDuration(double duration) {
    Q_UNUSED(duration)

    if (m_engine->duration() > 0) {
        m_seekSlider->setEnabled(true);
    }
}

void StatusBar::updateState(PlaybackState state) {
    switch (state) {
        case PlaybackState::Playing:
            m_playPauseButton->setText("❚❚");
            m_trackInfoLabel->setText("▶ " + QFileInfo(m_engine->currentFile()).baseName());
            break;

        case PlaybackState::Paused:
            m_playPauseButton->setText("▶");
            m_trackInfoLabel->setText("❚❚ " + QFileInfo(m_engine->currentFile()).baseName());
            break;

        case PlaybackState::Stopped:
            m_playPauseButton->setText("▶");
            m_trackInfoLabel->setText("■ 未在播放");
            m_seekSlider->setValue(0);
            m_positionLabel->setText("0:00 / 0:00");
            break;
    }
}

void StatusBar::updateCurrentTrack(const QString& filepath) {
    Q_UNUSED(filepath)
}
