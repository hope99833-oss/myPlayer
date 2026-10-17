#include "player.h"
#include <QStyle>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>
#include <QDebug>


player::player(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Player");
    setMinimumSize(900, 600);
    resize(960, 640);
    setupUI();
    setupConnections();
}

player::~player()
{
}

void player::setupUI()
{
    setStyleSheet(
        "QWidget#playerWindow { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #2a2a2e, stop:1 #1a1a1e); }"
        "QWidget#videoArea { background-color: #000; }"
        "QWidget#controlBar { background-color: #252528; border-top: 1px solid #3a3a3e; }"
        "QWidget#titleBar { background-color: #2a2a2e; border-bottom: 1px solid #3a3a3e; }"

        "QPushButton {"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 4px;"
        "  background: #2d2d32;"
        "}"
        "QPushButton#playBtn:hover { background: #3a3a42; }"
        "QPushButton#playBtn:pressed { background: #4a4a52; }"
        "QPushButton#navBtn:hover { background: #3a3a42; }"
        "QPushButton#navBtn:pressed { background: #4a4a52; }"
        "QPushButton#utilBtn:hover { background: #3a3a42; }"
        "QPushButton#utilBtn:pressed { background: #4a4a52; }"

        "QSlider#progressSlider::groove:horizontal {"
        "  height: 6px;"
        "  background: #3a3a3e;"
        "  border-radius: 3px;"
        "}"
        "QSlider#progressSlider::handle:horizontal {"
        "  background: #fff;"
        "  width: 14px;"
        "  height: 14px;"
        "  margin: -5px 0;"
        "  border-radius: 7px;"
        "  border: 2px solid #0078d4;"
        "}"
        "QSlider#progressSlider::handle:horizontal:hover {"
        "  background: #e8e8e8;"
        "  border: 2px solid #1a8ae8;"
        "}"
        "QSlider#progressSlider::sub-page:horizontal {"
        "  background: #0078d4;"
        "  border-radius: 3px;"
        "}"

        "QSlider#volumeSlider::groove:horizontal {"
        "  height: 4px;"
        "  background: #3a3a3e;"
        "  border-radius: 2px;"
        "}"
        "QSlider#volumeSlider::handle:horizontal {"
        "  background: #ccc;"
        "  width: 10px;"
        "  height: 10px;"
        "  margin: -3px 0;"
        "  border-radius: 5px;"
        "}"
        "QSlider#volumeSlider::sub-page:horizontal {"
        "  background: #888;"
        "  border-radius: 2px;"
        "}"

        "QLabel#timeLabel { color: #aaa; font-size: 12px; font-family: 'Segoe UI', sans-serif; }"
        "QLabel#titleLabel { color: #e0e0e0; font-size: 13px; font-weight: bold; font-family: 'Segoe UI', sans-serif; }"
    );

    setObjectName("playerWindow");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Title bar ──
    QWidget* titleBar = new QWidget(this);
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(36);
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(12, 0, 8, 0);
    titleLayout->setSpacing(0);

    QLabel* titleLabel = new QLabel("MyPlayer", this);
    titleLabel->setObjectName("titleLabel");
    titleLayout->addWidget(titleLabel, 1);

    QPushButton* minBtn = new QPushButton(this);
    minBtn->setText("-");
    minBtn->setFixedSize(36, 36);
    minBtn->setStyleSheet(
        "QPushButton { color: #aaa; font-size: 14px; border: none; background: transparent; }"
        "QPushButton:hover { background: rgba(255,255,255,0.08); color: #fff; }"
    );
    titleLayout->addWidget(minBtn);

    QPushButton* maximizeBtn = new QPushButton(this);
    maximizeBtn->setText("[]");
    maximizeBtn->setFixedSize(36, 36);
    maximizeBtn->setStyleSheet(
        "QPushButton { color: #aaa; font-size: 12px; border: none; background: transparent; }"
        "QPushButton:hover { background: rgba(255,255,255,0.08); color: #fff; }"
    );
    titleLayout->addWidget(maximizeBtn);

    QPushButton* closeBtn = new QPushButton(this);
    closeBtn->setText("");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setStyleSheet(
        "QPushButton { color: #aaa; font-size: 12px; border: none; background: transparent; }"
        "QPushButton:hover { background: #e81123; color: #fff; }"
    );
    titleLayout->addWidget(closeBtn);

    mainLayout->addWidget(titleBar, 0);

    // ── Video area ──
    videoArea = new QWidget(this);
    videoArea->setObjectName("videoArea");
    videoArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(videoArea, 1);

    // ─ Control bar ──
    QWidget* controlBar = new QWidget(this);
    controlBar->setObjectName("controlBar");
    controlBar->setFixedHeight(72);

    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(16, 10, 16, 10);
    controlLayout->setSpacing(6);

    // Navigation buttons
    prevBtn = createIconButton(":/icons/previous.png", "Previous", 34);
    prevBtn->setObjectName("navBtn");
    playBtn = createIconButton(":/icons/play.png", "Play", 42);
    playBtn->setObjectName("playBtn");
    stopBtn = createIconButton(":/icons/stop.png", "Stop", 34);
    stopBtn->setObjectName("navBtn");
    nextBtn = createIconButton(":/icons/next.png", "Next", 34);
    nextBtn->setObjectName("navBtn");

    controlLayout->addWidget(prevBtn);
    controlLayout->addWidget(playBtn);
    controlLayout->addWidget(stopBtn);
    controlLayout->addWidget(nextBtn);

    controlLayout->addSpacing(16);

    // Progress section
    currentTimeLabel = new QLabel("00:00", this);
    currentTimeLabel->setObjectName("timeLabel");
    currentTimeLabel->setFixedWidth(48);
    currentTimeLabel->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(currentTimeLabel);

    progressSlider = new QSlider(Qt::Horizontal, this);
    progressSlider->setObjectName("progressSlider");
    progressSlider->setRange(0, totalSeconds);
    progressSlider->setValue(0);
    controlLayout->addWidget(progressSlider, 1);

    totalTimeLabel = new QLabel(formatTime(totalSeconds), this);
    totalTimeLabel->setObjectName("timeLabel");
    totalTimeLabel->setFixedWidth(48);
    totalTimeLabel->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(totalTimeLabel);

    controlLayout->addSpacing(16);

    // Volume section
    volumeBtn = createIconButton(":/icons/volume.png", "Volume", 28);
    volumeBtn->setObjectName("utilBtn");
    controlLayout->addWidget(volumeBtn);

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setObjectName("volumeSlider");
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(80);
    volumeSlider->setFixedWidth(90);
    controlLayout->addWidget(volumeSlider);

    controlLayout->addSpacing(8);

    fullscreenBtn = createIconButton(":/icons/fullscreen.png", "Fullscreen", 28);
    fullscreenBtn->setObjectName("utilBtn");
    controlLayout->addWidget(fullscreenBtn);

    mainLayout->addWidget(controlBar, 0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &player::updateTimeLabel);

    // Window button connections
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(maximizeBtn, &QPushButton::clicked, this, [this]() {
        isMaximized() ? showNormal() : showMaximized();
    });
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
}

void player::setupConnections()
{
    connect(playBtn, &QPushButton::clicked, this, &player::onPlayClicked);
    connect(stopBtn, &QPushButton::clicked, this, &player::onStopClicked);
    connect(progressSlider, &QSlider::sliderMoved, this, &player::onProgressChanged);
    connect(volumeSlider, &QSlider::valueChanged, this, &player::onVolumeChanged);
    connect(fullscreenBtn, &QPushButton::clicked, this, &player::onFullscreenClicked);
}

QPushButton* player::createIconButton(const QString& iconPath, const QString& tooltip, int size)
{
    QPushButton* btn = new QPushButton(this);
    btn->setToolTip(tooltip);
    btn->setFixedSize(size, size);
    QIcon icon(iconPath);
    if (icon.isNull()) {
        qWarning() << "Icon failed to load:" << iconPath;
    }
    btn->setIcon(icon);
    btn->setIconSize(QSize(size - 4, size - 4));
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QString player::formatTime(int seconds)
{
    int mins = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

void player::onPlayClicked()
{
    isPlaying = !isPlaying;
    if (isPlaying) {
        playBtn->setIcon(QIcon(":/icons/pause.png"));
        playBtn->setToolTip("Pause");
        timer->start(1000);
    } else {
        playBtn->setIcon(QIcon(":/icons/play.png"));
        playBtn->setToolTip("Play");
        timer->stop();
    }
}

void player::onStopClicked()
{
    isPlaying = false;
    playBtn->setIcon(QIcon(":/icons/play.png"));
    playBtn->setToolTip("Play");
    timer->stop();
    currentSeconds = 0;
    progressSlider->setValue(0);
    currentTimeLabel->setText("00:00");
}

void player::onProgressChanged(int value)
{
    currentSeconds = value;
    currentTimeLabel->setText(formatTime(currentSeconds));
}

void player::onVolumeChanged(int value)
{
    (void)value;
}

void player::onFullscreenClicked()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void player::updateTimeLabel()
{
    if (currentSeconds < totalSeconds) {
        currentSeconds++;
        progressSlider->setValue(currentSeconds);
        currentTimeLabel->setText(formatTime(currentSeconds));
    } else {
        onStopClicked();
    }
}
