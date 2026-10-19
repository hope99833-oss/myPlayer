#include "player.h"
#include "ffplayer.h"
#include <QStyle>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QMetaObject>
#include <QDateTime>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static void playerCallback(void* userdata, int state, double position, double duration) {
    player* self = static_cast<player*>(userdata);
    QMetaObject::invokeMethod(self, [self, state, position, duration]() {
        self->onPlayerProgress(state, position, duration);
    }, Qt::QueuedConnection);
}


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
    if (ffPlayer) {
        ffPlayer->stop();
        delete ffPlayer;
        ffPlayer = nullptr;
    }
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
    progressSlider->setRange(0, totalSeconds * 1000);
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
    connect(progressSlider, &QSlider::valueChanged, this, &player::onProgressChanged);
    connect(progressSlider, &QSlider::sliderPressed, this, [this]() { isSeeking = true; });
    connect(progressSlider, &QSlider::sliderReleased, this, [this]() {
        isSeeking = false;
        if (ffPlayer) {
            // Seek once to the final release position (slider is in ms). Seeking
            // during the drag floods the op queue and stream_seek drops all but
            // the first, so the final target would be lost.
            seekTargetMs = progressSlider->value();
            lastSeekTime = QDateTime::currentMSecsSinceEpoch();
            ffPlayer->seek(seekTargetMs / 1000.0);
        }
    });
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

void player::setProgressMs(int ms)
{
    // Programmatic updates must not reach onProgressChanged, otherwise the
    // playback callback would trigger seeks.
    progressSlider->blockSignals(true);
    progressSlider->setValue(ms);
    progressSlider->blockSignals(false);
    currentTimeLabel->setText(formatTime(ms / 1000));
}

void player::onPlayClicked()
{
    if (!ffPlayer) {
        // Create FFPlayer instance with video file and embed in videoArea
        const char* filename = "C:\\Users\\admin\\Downloads\\校园门口交通乱象.mp4";
#ifdef Q_OS_WIN
        void* hwnd = (void*)videoArea->winId();
#else
        void* hwnd = nullptr;
#endif
        int w = videoArea->width();
        int h = videoArea->height();

        ffPlayer = new FFPlayer(filename, hwnd, w, h);
        ffPlayer->setCallback(playerCallback, this);
        int ret = ffPlayer->play();
        if (ret != 0) {
            qWarning() << "Failed to start playback";
            delete ffPlayer;
            ffPlayer = nullptr;
            return;
        }
    }

    isPlaying = !isPlaying;
    if (isPlaying) {
        if (ffPlayer && ffPlayer->isPaused()) {
            ffPlayer->resume();
        }
        playBtn->setIcon(QIcon(":/icons/pause.png"));
        playBtn->setToolTip("Pause");
    } else {
        if (ffPlayer && ffPlayer->isRunning()) {
            ffPlayer->pause();
        }
        playBtn->setIcon(QIcon(":/icons/play.png"));
        playBtn->setToolTip("Play");
    }
}

void player::onStopClicked()
{
    if (ffPlayer) {
        ffPlayer->stop();
        delete ffPlayer;
        ffPlayer = nullptr;
    }
    isPlaying = false;
    isSeeking = false;
    seekTargetMs = -1;
    playBtn->setIcon(QIcon(":/icons/play.png"));
    playBtn->setToolTip("Play");
    currentSeconds = 0;
    setProgressMs(0);
}

void player::onProgressChanged(int value)
{
    // Slider is in milliseconds. Fires only for user interaction (programmatic
    // updates are blocked in setProgressMs).
    currentTimeLabel->setText(formatTime(value / 1000));
    if (!ffPlayer)
        return;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (isSeeking) {
        // Live scrub while dragging the handle: throttle so we don't flood the
        // op queue; the exact target is sent on sliderReleased.
        if (now - lastSeekTime >= 200) {
            lastSeekTime = now;
            seekTargetMs = value;
            ffPlayer->seek(value / 1000.0);
        }
    } else {
        // Groove click or keyboard step (no handle drag): seek immediately.
        seekTargetMs = value;
        lastSeekTime = now;
        ffPlayer->seek(value / 1000.0);
    }
}

void player::onVolumeChanged(int value)
{
    if (ffPlayer) {
        ffPlayer->setVolume(value);
    }
}

void player::onFullscreenClicked()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void player::onPlayerProgress(int state, double position, double duration)
{
    // Update duration if needed
    if (duration > 0 && totalSeconds != (int)duration) {
        totalSeconds = (int)duration;
        progressSlider->blockSignals(true);
        progressSlider->setRange(0, totalSeconds * 1000);
        progressSlider->blockSignals(false);
        totalTimeLabel->setText(formatTime(totalSeconds));
    }

    // Update position. After a seek, keep the handle pinned at the release
    // position until the decoder reports a position near the target (or a 2 s
    // timeout), so the handle never snaps back to the pre-seek position.
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    int posMs = (int)(position * 1000);
    bool settled = (seekTargetMs < 0) ||
                   qAbs(posMs - seekTargetMs) < 1000 ||
                   (now - lastSeekTime > 2000);
    if (!isSeeking && settled) {
        seekTargetMs = -1;
        setProgressMs(posMs);
    }

    // Update state
    switch (state) {
    case FFPLAYER_STATE_PLAYING:
        if (!isPlaying) {
            isPlaying = true;
            playBtn->setIcon(QIcon(":/icons/pause.png"));
            playBtn->setToolTip("Pause");
        }
        break;
    case FFPLAYER_STATE_PAUSED:
        if (isPlaying) {
            isPlaying = false;
            playBtn->setIcon(QIcon(":/icons/play.png"));
            playBtn->setToolTip("Play");
        }
        break;
    case FFPLAYER_STATE_FINISHED:
    case FFPLAYER_STATE_STOPPED:
        if (isPlaying) {
            isPlaying = false;
            playBtn->setIcon(QIcon(":/icons/play.png"));
            playBtn->setToolTip("Play");
        }
        if (state == FFPLAYER_STATE_FINISHED) {
            // Auto-stop on finish
            if (ffPlayer) {
                ffPlayer->stop();
                delete ffPlayer;
                ffPlayer = nullptr;
            }
            currentSeconds = 0;
            setProgressMs(0);
        }
        break;
    }
}
