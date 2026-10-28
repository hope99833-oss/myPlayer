#include "player.h"
#include "ffplayer.h"
#include <QDebug>
#include <QMetaObject>
#include <QDateTime>
#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QEvent>
#include <QResizeEvent>
#include <QFileDialog>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static constexpr int kVideoSideInset = 16;

#ifdef Q_OS_WIN
// Map a global point to a Win32 resize hit-code if it lies on a window edge,
// else 0. grip is the grabbable border thickness in pixels.
static int hitCodeForRect(const QRect& r, const QPoint& g, int grip)
{
    const bool left   = g.x() <  r.left()  + grip;
    const bool right  = g.x() >= r.right() - grip;
    const bool top    = g.y() <  r.top()   + grip;
    const bool bottom = g.y() >= r.bottom()- grip;
    if (top && left)      return HTTOPLEFT;
    if (top && right)     return HTTOPRIGHT;
    if (bottom && left)   return HTBOTTOMLEFT;
    if (bottom && right)  return HTBOTTOMRIGHT;
    if (left)             return HTLEFT;
    if (right)            return HTRIGHT;
    if (top)              return HTTOP;
    if (bottom)           return HTBOTTOM;
    return 0;
}

static Qt::CursorShape cursorForHit(int code)
{
    switch (code) {
    case HTLEFT:
    case HTRIGHT:        return Qt::SizeHorCursor;
    case HTTOP:
    case HTBOTTOM:       return Qt::SizeVerCursor;
    case HTTOPLEFT:
    case HTBOTTOMRIGHT:  return Qt::SizeFDiagCursor;
    case HTTOPRIGHT:
    case HTBOTTOMLEFT:   return Qt::SizeBDiagCursor;
    default:             return Qt::ArrowCursor;
    }
}

// The SDL surface runs in its own top-level window owned by this window, instead
// of a native child inside the client area. A native child made Qt and Windows
// fight over the same pixel region during a live resize, which is what flashed
// the title/control bars; a separate owned window is composited on its own and
// leaves this window free to repaint itself. Ownership keeps it stacked above
// this window and hides it automatically while this window is minimized.
static HWND createVideoHostWindow(HWND owner)
{
    // Own window class with a black background brush: the area SDL has not drawn
    // into yet (and the strip Windows exposes while the window grows) erases to
    // black instead of the light-grey STATIC default, so nothing flashes.
    static const wchar_t* kClass = L"MyPlayerVideoHost";
    static ATOM cls = 0;
    if (!cls) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
        wc.lpszClassName = kClass;
        cls = RegisterClassExW(&wc);
    }
    if (!cls)
        return nullptr;
    // WS_THICKFRAME is not for the user: SDL only follows the host window's
    // size (WM_SIZE -> renderer swapchain) when the window looks resizable, and
    // it decides that from this style bit at SDL_CreateWindowFrom time. FFPlayer
    // drops the bit again right after, so the client area still equals the
    // window rect instead of being inset by the resize frame.
    return CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
                           kClass, L"",
                           WS_POPUP | WS_THICKFRAME | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                           0, 0, 16, 16, owner, nullptr,
                           GetModuleHandleW(nullptr), nullptr);
}

// Windows leaves the strip a live resize is about to expose unpainted until Qt
// runs its next paint pass, and WM_ERASEBKGND is suppressed here, so that strip
// shows the desktop: a transparent, scrollbar-looking sliver along the edge
// being dragged. Pre-fill it with the same flat colours Qt paints (title bar /
// video area / control bar), so the sliver is already correct when the frame
// grows.
static void fillChromeBand(HDC dc, const RECT& rc, int winH, int chromeTop, int chromeBottom)
{
    const int left = (int)rc.left;    const int right = (int)rc.right;
    const int top = qMax((int)rc.top, 0);
    const int bottom = qMin((int)rc.bottom, winH);
    if (left >= right || top >= bottom)
        return;
    static HBRUSH titleBrush = nullptr;
    static HBRUSH controlBrush = nullptr;
    if (!titleBrush)
        titleBrush = CreateSolidBrush(RGB(0x1b, 0x1f, 0x25));
    if (!controlBrush)
        controlBrush = CreateSolidBrush(RGB(0x12, 0x14, 0x18));
    RECT band = { left, top, right, qMin(bottom, chromeTop) };
    if (band.bottom > band.top)
        FillRect(dc, &band, titleBrush);
    band = { left, qMax(top, chromeTop), right, qMin(bottom, winH - chromeBottom) };
    if (band.bottom > band.top)
        FillRect(dc, &band, (HBRUSH)GetStockObject(BLACK_BRUSH));
    band = { left, qMax(top, winH - chromeBottom), right, bottom };
    if (band.bottom > band.top)
        FillRect(dc, &band, controlBrush);
}
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
    // Custom chrome. Use FramelessWindowHint so Qt and Windows agree the frame
    // margins are zero (this is what WM_NCCALCSIZE below reports), then re-add
    // WS_THICKFRAME/WS_CAPTION at the Win32 level. THICKFRAME gives native edge
    // resize + smooth single-step maximize; CAPTION keeps the DWM shadow and lets
    // NCCALCSIZE strip a real frame. Mixing Qt::Window (Qt thinks there's a frame)
    // with NCCALCSIZE=0 desyncs the geometry and kills the resize grips.
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowTitle("Player");
    // Solid palette fill on the frame instead of a full-window gradient repaint
    // (which tears/flickers on every resize step).
    {
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(0x1a, 0x1a, 0x1e));
        setPalette(pal);
    }
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAutoFillBackground(true);
    setMinimumSize(900, 600);
    resize(960, 640);
    setupUI();
    setupConnections();
    // Fullscreen auto-hide controls: poll the cursor (the native video window
    // swallows mouse-move events, so a timer is the reliable way) and reveal the
    // control bar only while the pointer is near the bottom edge.
    hoverTimer = new QTimer(this);
    hoverTimer->setInterval(120);
    connect(hoverTimer, &QTimer::timeout, this, [this]() {
        if (!isFullScreen())
            return;
        // The native video host window swallows mouse-move events, so poll the
        // global cursor and map it into this window to decide whether the pointer
        // is over the bottom strip where the control bar should appear.
        const int y = mapFromGlobal(QCursor::pos()).y();
        const bool nearBottom = y >= 0 && y >= height() - 100;
        if (nearBottom != controlBar->isVisible()) {
            controlBar->setVisible(nearBottom);
            QTimer::singleShot(0, this, [this]() { syncVideoWindow(); });
        }
    });
    // App-level filter so edge hover/press is seen even where child widgets
    // (including the native video HWND) would otherwise swallow the mouse.
    qApp->installEventFilter(this);
}

player::~player()
{
    if (ffPlayer) {
        ffPlayer->stop();
        delete ffPlayer;
        ffPlayer = nullptr;
    }
    destroyVideoHostWindow();
}

void player::setupUI()
{
    setStyleSheet(
        "QWidget#videoContainer { background-color: #000; }"
        "QWidget#videoArea { background-color: #000; }"
        "QWidget#controlBar { background-color: #121418; border-top: 1px solid #262b33; }"
        "QWidget#titleBar { background-color: #1b1f25; border-bottom: 1px solid #262b33; }"

        "QPushButton {"
        "  border: 1px solid transparent;"
        "  border-radius: 8px;"
        "  padding: 4px;"
        "  background: rgba(255,255,255,0.08);"
        "}"
        "QPushButton#navBtn:hover, QPushButton#utilBtn:hover, QPushButton#playBtn:hover {"
        "  background: rgba(255,255,255,0.14);"
        "  border-color: rgba(255,255,255,0.10);"
        "}"
        "QPushButton#navBtn:pressed, QPushButton#utilBtn:pressed, QPushButton#playBtn:pressed {"
        "  background: rgba(255,255,255,0.20);"
        "}"

        // Sliders are self-painted (SeekSlider::paintEvent); opaque label
        // backgrounds so stale text pixels are cleared on every repaint.
        "QLabel#timeLabel {"
        "  color: #d7dbe3;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "  background-color: #0d1015;"
        "  border: 1px solid #262b33;"
        "  border-radius: 8px;"
        "  padding: 0 8px;"
        "  font-family: 'Cascadia Mono', 'Consolas', 'Segoe UI Mono';"
        "}"
        "QLabel#titleLabel { color: #eef1f6; font-size: 13px; font-weight: bold; background-color: #1b1f25; font-family: 'Segoe UI', sans-serif; }"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Title bar ──
    titleBar = new QWidget(this);
    titleBar->setAutoFillBackground(true);
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(36);
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(kVideoSideInset, 0, kVideoSideInset, 0);
    titleLayout->setSpacing(0);

    titleLabel = new QLabel("MyPlayer", this);
    titleLabel->setObjectName("titleLabel");
    // Let presses fall through to the title bar so the whole bar is draggable.
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    titleLayout->addWidget(titleLabel, 1);

    const QString capBtnStyle =
        "QPushButton { border: none; background: transparent; border-radius: 6px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.10); }"
        "QPushButton:pressed { background: rgba(255,255,255,0.18); }";

    minBtn = new QPushButton(this);
    minBtn->setObjectName("capBtn");
    minBtn->setFixedSize(36, 36);
    minBtn->setIcon(QIcon(":/icons/minimize.png"));
    minBtn->setIconSize(QSize(14, 14));
    minBtn->setToolTip("Minimize");
    minBtn->setStyleSheet(capBtnStyle);
    titleLayout->addWidget(minBtn);

    maxBtn = new QPushButton(this);
    maxBtn->setObjectName("capBtn");
    maxBtn->setFixedSize(36, 36);
    maxBtn->setIconSize(QSize(14, 14));
    maxBtn->setToolTip("Maximize");
    maxBtn->setStyleSheet(capBtnStyle);
    titleLayout->addWidget(maxBtn);

    closeBtn = new QPushButton(this);
    closeBtn->setObjectName("capBtn");
    closeBtn->setFixedSize(36, 36);
    closeBtn->setIcon(QIcon(":/icons/close.png"));
    closeBtn->setIconSize(QSize(14, 14));
    closeBtn->setToolTip("Close");
    closeBtn->setStyleSheet(
        "QPushButton { border: none; background: transparent; border-radius: 6px; }"
        "QPushButton:hover { background: #e81123; }"
        "QPushButton:pressed { background: #c50f1f; }");
    titleLayout->addWidget(closeBtn);

    // Drag-to-move and double-click-to-maximize are handled in eventFilter.
    titleBar->installEventFilter(this);

    mainLayout->addWidget(titleBar, 0);

    // ── Video area ──
    videoContainer = new QWidget(this);
    videoContainer->setObjectName("videoContainer");
    videoContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    videoContainer->setAttribute(Qt::WA_OpaquePaintEvent, true);
    videoContainer->setAttribute(Qt::WA_StyledBackground, true);
    videoContainer->setAutoFillBackground(true);
    {
        QPalette pal = videoContainer->palette();
        pal.setColor(QPalette::Window, Qt::black);
        videoContainer->setPalette(pal);
    }
    QHBoxLayout* videoLayout = new QHBoxLayout(videoContainer);
    videoLayout->setContentsMargins(kVideoSideInset, 0, kVideoSideInset, 0);
    videoLayout->setSpacing(0);

    videoArea = new QWidget(videoContainer);
    videoArea->setObjectName("videoArea");
    videoArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    videoArea->setAttribute(Qt::WA_OpaquePaintEvent, true);
    videoArea->setAttribute(Qt::WA_StyledBackground, true);
    videoArea->setAutoFillBackground(true);
    {
        QPalette pal = videoArea->palette();
        pal.setColor(QPalette::Window, Qt::black);
        videoArea->setPalette(pal);
    }
    // Stays an alien widget: the video is hosted by a separate owned window
    // (see createVideoHostWindow) that is positioned over this area, so nothing
    // native ever lives inside the Qt client area.
    videoLayout->addWidget(videoArea);
    mainLayout->addWidget(videoContainer, 1);

    // ─ Control bar ──
    controlBar = new QWidget(this);
    controlBar->setObjectName("controlBar");
    controlBar->setFixedHeight(80);
    controlBar->setAutoFillBackground(true);

    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(kVideoSideInset, 12, kVideoSideInset, 12);
    controlLayout->setSpacing(8);

    playBtn = createIconButton(":/icons/play.png", "Play", 44);
    playBtn->setObjectName("playBtn");
    stopBtn = createIconButton(":/icons/stop.png", "Stop", 36);
    stopBtn->setObjectName("navBtn");

    controlLayout->addWidget(playBtn);
    controlLayout->addWidget(stopBtn);

    controlLayout->addSpacing(16);

    // Progress section
    currentTimeLabel = new QLabel("00:00", this);
    currentTimeLabel->setObjectName("timeLabel");
    currentTimeLabel->setFixedWidth(56);
    currentTimeLabel->setFixedHeight(32);
    currentTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    controlLayout->addWidget(currentTimeLabel);

    progressSlider = new SeekSlider(Qt::Horizontal, this);
    progressSlider->setObjectName("progressSlider");
    progressSlider->setLook({4, 14, QColor(0x2b, 0x31, 0x3b), QColor(0x0a, 0x84, 0xff),
                             QColor(0xff, 0xff, 0xff), QColor(0x0a, 0x84, 0xff), 2});
    progressSlider->setRange(0, totalSeconds * 1000);
    progressSlider->setValue(0);
    progressSlider->setMinimumHeight(24);
    progressSlider->setCursor(Qt::PointingHandCursor);
    // Default QSlider groove clicks only nudge by pageStep (10 ms here), so a
    // click far from the handle looks dead. Intercept mouse input to map the
    // click/drag x-position straight to a timeline value, like commercial players.
    progressSlider->installEventFilter(this);
    controlLayout->addWidget(progressSlider, 1);

    totalTimeLabel = new QLabel(formatTime(totalSeconds), this);
    totalTimeLabel->setObjectName("timeLabel");
    totalTimeLabel->setFixedWidth(56);
    totalTimeLabel->setFixedHeight(32);
    totalTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    controlLayout->addWidget(totalTimeLabel);

    controlLayout->addSpacing(12);

    // Volume section
    volumeBtn = createIconButton(":/icons/volume.png", "Volume", 32);
    volumeBtn->setObjectName("utilBtn");
    controlLayout->addWidget(volumeBtn);

    volumeSlider = new SeekSlider(Qt::Horizontal, this);
    volumeSlider->setObjectName("volumeSlider");
    volumeSlider->setLook({4, 12, QColor(0x2b, 0x31, 0x3b), QColor(0x98, 0xa2, 0xb3),
                           QColor(0xf4, 0xf6, 0xf8), QColor(0xf4, 0xf6, 0xf8), 0});
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(80);
    volumeSlider->setFixedWidth(104);
    volumeSlider->setMinimumHeight(20);
    volumeSlider->setCursor(Qt::PointingHandCursor);
    controlLayout->addWidget(volumeSlider);

    controlLayout->addSpacing(8);

    fullscreenBtn = createIconButton(":/icons/fullscreen.png", "Fullscreen", 32);
    fullscreenBtn->setObjectName("utilBtn");
    controlLayout->addWidget(fullscreenBtn);

    controlLayout->addSpacing(8);

    // Open-file picker, at the outer right edge like other players.
    openBtn = createIconButton(":/icons/open.png", "Open File", 32);
    openBtn->setObjectName("utilBtn");
    controlLayout->addWidget(openBtn);

    mainLayout->addWidget(controlBar, 0);

    // Window button connections
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(maxBtn, &QPushButton::clicked, this, [this]() {
        toggleMaximize();
    });
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    updateMaximizeIcon();
}

void player::setupConnections()
{
    connect(playBtn, &QPushButton::clicked, this, &player::onPlayClicked);
    connect(stopBtn, &QPushButton::clicked, this, &player::onStopClicked);
    connect(openBtn, &QPushButton::clicked, this, &player::onOpenFileClicked);
    // valueChanged now only reaches here for keyboard steps (arrow keys) and any
    // non-mouse change; mouse press/drag/release is handled in eventFilter, which
    // blocks signals and seeks explicitly.
    connect(progressSlider, &QSlider::valueChanged, this, &player::onProgressChanged);
    connect(volumeSlider, &QSlider::valueChanged, this, &player::onVolumeChanged);
    connect(volumeBtn, &QPushButton::clicked, this, [this]() {
        if (ffPlayer)
            ffPlayer->toggleMute();
        updateMuteIcon();
    });
    connect(fullscreenBtn, &QPushButton::clicked, this, &player::onFullscreenClicked);
}

bool player::eventFilter(QObject* watched, QEvent* event)
{
#ifdef Q_OS_WIN
    // Edge resize + resize cursor. Runs app-wide (installed on qApp) so it sees
    // the mouse even over child widgets and the native video HWND, which would
    // otherwise swallow WM_NCHITTEST and leave the cursor as a plain arrow.
    if (!isMaximized() && !isFullScreen()) {
        const QEvent::Type t = event->type();
        if (t == QEvent::MouseButtonPress || t == QEvent::MouseMove ||
            t == QEvent::HoverMove) {
            QPoint g;
            if (t == QEvent::HoverMove)
                g = QCursor::pos();
            else
                g = static_cast<const QMouseEvent*>(event)->globalPos();
            if (!g.isNull()) {
                const int grip = 8;
                const int code = hitCodeForRect(frameGeometry(), g, grip);
                if (t == QEvent::MouseButtonPress && code) {
                    ReleaseCapture();
                    SendMessage((HWND)winId(), WM_NCLBUTTONDOWN, (WPARAM)code,
                                (LPARAM)MAKELPARAM(g.x(), g.y()));
                    return true;   // consume: don't let titleBar start a drag
                }
                if (t != QEvent::MouseButtonPress)
                    setCursor(cursorForHit(code));
            }
        }
    }
#endif
    if (watched == progressSlider) {
        const QEvent::Type t = event->type();
        if (t == QEvent::MouseButtonPress || t == QEvent::MouseMove ||
            t == QEvent::MouseButtonRelease) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            const bool leftHeld = (me->buttons() & Qt::LeftButton) ||
                                  (me->button() == Qt::LeftButton);
            if (leftHeld) {
                int val = sliderValueFromPos(me->pos());
                if (t == QEvent::MouseButtonPress) {
                    isSeeking = true;
                    progressSlider->setSliderDown(true);
                    scrubTo(val, true);   // click jumps straight here
                } else if (t == QEvent::MouseMove) {
                    scrubTo(val, false);  // throttled live scrub while dragging
                } else {                  // release
                    scrubTo(val, true);   // final precise seek
                    progressSlider->setSliderDown(false);
                    isSeeking = false;
                }
                return true;              // swallow QSlider's page-step handling
            }
        }
    }
    if (watched == titleBar) {
        const QEvent::Type t = event->type();
        if (t == QEvent::MouseButtonDblClick) {
            toggleMaximize();
            return true;
        }
        if (t == QEvent::MouseButtonPress || t == QEvent::MouseMove ||
            t == QEvent::MouseButtonRelease) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton || (me->buttons() & Qt::LeftButton)) {
                if (t == QEvent::MouseButtonPress) {
                    dragging = true;
                    dragStartPos = me->globalPos() - frameGeometry().topLeft();
                } else if (t == QEvent::MouseMove) {
                    if (dragging && !isMaximized())
                        move(me->globalPos() - dragStartPos);
                } else {
                    dragging = false;
                }
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

bool player::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        // Strip the standard non-client frame (caption + borders) so the client
        // area fills the whole window, while keeping WS_THICKFRAME for native
        // resize/maximize. Returning 0 for wParam==TRUE means "client == window".
        if (msg->message == WM_NCCALCSIZE && msg->wParam == TRUE) {
            *result = 0;
            return true;
        }
        // Suppress the default background erase: Qt repaints the styled background
        // itself, and letting Windows clear to the class brush first is what makes
        // maximize/resize flash.
        if (msg->message == WM_ERASEBKGND) {
            *result = 1;
            return true;
        }
        // Keep the owned video window on top of videoArea while the frame moves
        // or resizes (it is a separate top-level window, not a child).
        if (msg->message == WM_WINDOWPOSCHANGING || msg->message == WM_WINDOWPOSCHANGED) {
            // Only once the video host exists: this runs while the window is
            // still being created otherwise, and the native calls below are not
            // safe against that (it crashed inside Qt's window creation).
            if (videoHostWnd) {
                RECT wr = {};
                if (GetWindowRect((HWND)winId(), &wr)) {
                    WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(msg->lParam);
                    const int winX = (wp && !(wp->flags & SWP_NOMOVE)) ? wp->x : wr.left;
                    const int winY = (wp && !(wp->flags & SWP_NOMOVE)) ? wp->y : wr.top;
                    const int winW = (wp && !(wp->flags & SWP_NOSIZE)) ? wp->cx : (wr.right - wr.left);
                    const int winH = (wp && !(wp->flags & SWP_NOSIZE)) ? wp->cy : (wr.bottom - wr.top);
                    const int chromeTop = (!isFullScreen() && titleBar && titleBar->isVisible())
                                              ? titleBar->height() : 0;
                    const int chromeBottom = (controlBar && controlBar->isVisible())
                                                 ? controlBar->height() : 0;
                    const int sideInset = isFullScreen() ? 0 : kVideoSideInset;
                    applyVideoHostRect(winX + sideInset, winY + chromeTop,
                                       winW - sideInset * 2,
                                       winH - chromeTop - chromeBottom);
                }
            }
        } else if (msg->message == WM_SIZE) {
            // Paint the band this resize exposed before returning from the
            // message (see paintExposedStrip), then re-seat the video host.
            paintExposedStrip((int)LOWORD(msg->lParam), (int)HIWORD(msg->lParam));
            syncVideoWindow();
        } else if (msg->message == WM_MOVE) {
            syncVideoWindow();
        }
        // Clamp the maximized rect to the work area (excludes the taskbar).
        if (msg->message == WM_GETMINMAXINFO) {
            // Reporting the work area verbatim (relative to its monitor) makes
            // Windows place the maximized window exactly on it. Qt's
            // availableGeometry() used to be 8px larger on every side here,
            // which pushed the window past the work area: those strips are part
            // of the client area (WM_NCCALCSIZE reports no non-client area) and
            // nothing paints them, so they showed the desktop through.
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
            MONITORINFO mi = { sizeof(MONITORINFO) };
            if (GetMonitorInfoW(MonitorFromWindow((HWND)winId(), MONITOR_DEFAULTTONEAREST), &mi)) {
                mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
                mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
                mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
                mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
            }
            *result = 0;
            return true;
        }
    }
#else
    Q_UNUSED(eventType); Q_UNUSED(message); Q_UNUSED(result);
#endif
    return QWidget::nativeEvent(eventType, message, result);
}

void player::updateMaximizeIcon()
{
    maxBtn->setIcon(QIcon(isMaximized() ? ":/icons/restore.png" : ":/icons/maximize.png"));
    maxBtn->setToolTip(isMaximized() ? "Restore" : "Maximize");
}

void player::toggleMaximize()
{
#ifdef Q_OS_WIN
    // Use the native command so Windows/DWM performs a single-step transition.
    // Qt's showMaximized()/showNormal() runs its own geometry pass, which is the
    // "small window then big window" two-step flicker.
    SendMessage((HWND)winId(), WM_SYSCOMMAND,
                isMaximized() ? SC_RESTORE : SC_MAXIMIZE, 0);
#else
    isMaximized() ? showNormal() : showMaximized();
#endif
    updateMaximizeIcon();
}

void player::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        updateMaximizeIcon();
        // Fullscreen shows the video edge-to-edge: hide the chrome so the
        // video area (and the host window over it) covers the whole screen.
        const bool fs = isFullScreen();
        titleBar->setVisible(!fs);
        controlBar->setVisible(!fs);
        if (fs)
            hoverTimer->start();
        else
            hoverTimer->stop();
        // Re-sync after the layout drops the hidden bars and videoArea grows.
        QTimer::singleShot(0, this, [this]() { syncVideoWindow(); });
    }
    QWidget::changeEvent(event);
}

void player::updateMuteIcon()
{
    bool muted = ffPlayer && ffPlayer->isMuted();
    volumeBtn->setIcon(QIcon(muted ? ":/icons/volume_mute.png" : ":/icons/volume.png"));
    volumeBtn->setToolTip(muted ? "Unmute" : "Mute");
}

void player::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    syncVideoWindow();
    update();
}

void player::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
#ifdef Q_OS_WIN
    // In fullscreen the window must stay exactly screen-sized with no native
    // caption; re-adding WS_CAPTION/WS_THICKFRAME here would fight Qt's
    // fullscreen styles, so leave them alone while fullscreen.
    if (isFullScreen())
        return;
    // Qt re-applies window styles from windowFlags() on show; FramelessWindowHint
    // makes it set WS_POPUP, which drops WS_THICKFRAME. Re-add the thick frame here
    // (after Qt's own setup) so the HT* codes returned by WM_NCHITTEST actually
    // enable native edge/corner resize and single-step maximize.
    HWND hwnd = (HWND)winId();
    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    if (!(style & WS_THICKFRAME)) {
        style |= WS_THICKFRAME | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
        SetWindowLongPtrW(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
#endif
}

void player::syncVideoWindow()
{
#ifdef Q_OS_WIN
    // Keep the owned video window exactly over the video strip. It is a separate
    // top-level window, not a child, so it has to be repositioned explicitly on
    // every resize/move of this window.
    if (!videoHostWnd)
        return;
    // Take the client area straight from Win32 rather than from the widget tree.
    // Qt dispatches its own geometry update after the native WM_SIZE/WM_MOVE has
    // been handled, so videoArea is one message behind while the frame is being
    // dragged. Using the native client rect keeps the host in lock-step with the
    // actual painted content area and avoids the 1px-class side slivers that can
    // show up if we position against the outer window rect instead.
    HWND hwnd = (HWND)winId();
    RECT cr = {};
    if (!GetClientRect(hwnd, &cr))
        return;
    POINT origin = { cr.left, cr.top };
    if (!ClientToScreen(hwnd, &origin))
        return;
    const int chromeTop = (!isFullScreen() && titleBar && titleBar->isVisible())
                              ? titleBar->height() : 0;
    const int chromeBottom = (controlBar && controlBar->isVisible())
                                 ? controlBar->height() : 0;
    const int sideInset = isFullScreen() ? 0 : kVideoSideInset;
    const int x = origin.x + sideInset;
    const int y = origin.y + chromeTop;
    const int w = (cr.right - cr.left) - sideInset * 2;
    const int h = (cr.bottom - cr.top) - chromeTop - chromeBottom;
    applyVideoHostRect(x, y, w, h);
#endif
}

void player::paintExposedStrip(int winW, int winH)
{
#ifdef Q_OS_WIN
    // Windows only *posts* WM_PAINT for the band a resize exposes, and Qt
    // services that paint a whole event-loop turn later; WM_ERASEBKGND is
    // suppressed here as well, so until then those pixels are untouched and the
    // compositor lets the window underneath show through: a transparent,
    // scrollbar-shaped sliver along the edge being dragged. Filling the band on
    // the window DC while the WM_SIZE is still being handled leaves it opaque
    // in the frame the compositor picks up.
    if (!videoHostWnd || winW <= 0 || winH <= 0)
        return;
    const QSize prev = lastWindowSize;
    lastWindowSize = QSize(winW, winH);
    if (!prev.isValid() || prev == lastWindowSize)
        return;
    const int chromeTop = (!isFullScreen() && titleBar && titleBar->isVisible())
                              ? titleBar->height() : 0;
    const int chromeBottom = (controlBar && controlBar->isVisible())
                                 ? controlBar->height() : 0;
    HWND hwnd = (HWND)winId();
    HDC dc = GetDC(hwnd);
    if (!dc)
        return;
    // Only growth exposes pixels; the DC clips anything outside the new client
    // rect, so band coordinates may safely overshoot on shrink.
    if (winW > prev.width()) {
        const RECT band = { prev.width(), 0, winW, winH };
        fillChromeBand(dc, band, winH, chromeTop, chromeBottom);
    }
    if (winH > prev.height()) {
        const RECT band = { 0, prev.height(), winW, winH };
        fillChromeBand(dc, band, winH, chromeTop, chromeBottom);
    }
    ReleaseDC(hwnd, dc);
#else
    Q_UNUSED(winW); Q_UNUSED(winH);
#endif
}

void player::applyVideoHostRect(int x, int y, int w, int h)
{
#ifdef Q_OS_WIN
    if (w <= 0 || h <= 0)
        return;
    const QPoint tl(x, y);
    // Idempotent guard: SetWindowPos on the owned window re-enters this window
    // through WM_WINDOWPOSCHANGING, which would otherwise resize the host again
    // and grow/shrink the frame in a feedback loop. Skip when already in place.
    if (tl == lastVideoTl && w == lastVideoW && h == lastVideoH)
        return;
    lastVideoTl = tl;
    lastVideoW = w;
    lastVideoH = h;
    SetWindowPos((HWND)videoHostWnd, nullptr, x, y, w, h,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    RedrawWindow((HWND)videoHostWnd, nullptr, nullptr,
                 RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
#endif
}

void player::destroyVideoHostWindow()
{
#ifdef Q_OS_WIN
    if (videoHostWnd) {
        DestroyWindow((HWND)videoHostWnd);
        videoHostWnd = nullptr;
    }
    lastVideoW = lastVideoH = -1;
    lastVideoTl = QPoint();
    lastWindowSize = QSize();
#endif
}

int player::sliderValueFromPos(const QPoint& pos) const
{
    SeekSlider* s = progressSlider;
    int handle = s->handleWidth();
    int span = s->width() - handle;
    if (span <= 0)
        return s->value();
    int x = qBound(0, pos.x() - handle / 2, span);
    return s->minimum() + (int)((qint64)x * (s->maximum() - s->minimum()) / span);
}

void player::scrubTo(int valueMs, bool force)
{
    // Move the handle without re-entering onProgressChanged (which is for
    // keyboard/programmatic changes); we drive the seek from here.
    progressSlider->blockSignals(true);
    progressSlider->setValue(valueMs);
    progressSlider->blockSignals(false);
    currentTimeLabel->setText(formatTime(valueMs / 1000));

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (!ffPlayer)
        return;
    if (force || now - lastSeekTime >= 200) {
        lastSeekTime = now;
        seekTargetMs = valueMs;
        ffPlayer->seek(valueMs / 1000.0);
    }
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
        // Nothing loaded yet: ask for a file instead of starting playback.
        if (currentFile.isEmpty()) {
            onOpenFileClicked();
            return;
        }
        // Create FFPlayer instance with the chosen file and embed in videoArea
        const QByteArray filename = currentFile.toUtf8();
#ifdef Q_OS_WIN
        // Hand SDL an owned window placed over videoArea instead of a native Qt
        // child: place it, show it (SDL treats a foreign HWND as already shown),
        // then let SDL_CreateWindowFrom bind its renderer to it.
        if (!videoHostWnd)
            videoHostWnd = createVideoHostWindow((HWND)winId());
        syncVideoWindow();
        if (videoHostWnd)
            ShowWindow((HWND)videoHostWnd, SW_SHOWNOACTIVATE);
        void* hwnd = videoHostWnd;
#else
        void* hwnd = nullptr;
#endif
        int w = videoArea->width();
        int h = videoArea->height();

        ffPlayer = new FFPlayer(filename.constData(), hwnd, w, h);
        ffPlayer->setCallback(playerCallback, this);
        int ret = ffPlayer->play();
        if (ret != 0) {
            qWarning() << "Failed to start playback";
            delete ffPlayer;
            ffPlayer = nullptr;
            return;
        }
        updateMuteIcon();
        syncVideoWindow();
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
    destroyVideoHostWindow();
    videoArea->update();
    isPlaying = false;
    isSeeking = false;
    seekTargetMs = -1;
    playBtn->setIcon(QIcon(":/icons/play.png"));
    playBtn->setToolTip("Play");
    updateMuteIcon();
    setProgressMs(0);
}

void player::onOpenFileClicked()
{
    const QString filter =
        "Video Files (*.mp4 *.mkv *.avi *.mov *.flv *.wmv *.webm *.ts *.m4v *.mpg *.mpeg *.rmvb *.3gp);;"
        "All Files (*.*)";
    const QString dir = currentFile.isEmpty() ? QString() : QFileInfo(currentFile).absolutePath();
    const QString path = QFileDialog::getOpenFileName(this, "Open Media File", dir, filter);
    if (path.isEmpty())
        return;
    startFile(path);
}

void player::startFile(const QString& path)
{
    // Drop whatever is loaded before switching sources, mirroring onStopClicked.
    if (ffPlayer) {
        ffPlayer->stop();
        delete ffPlayer;
        ffPlayer = nullptr;
    }
    destroyVideoHostWindow();
    videoArea->update();
    isPlaying = false;
    isSeeking = false;
    seekTargetMs = -1;
    updateMuteIcon();
    setProgressMs(0);

    currentFile = path;
    titleLabel->setText(QFileInfo(path).fileName());
    onPlayClicked();
}

void player::onProgressChanged(int value)
{
    // Only reached for keyboard steps (arrow keys) now: mouse press/drag/release
    // is intercepted in eventFilter, which blocks signals and seeks via scrubTo().
    // Slider is in milliseconds.
    currentTimeLabel->setText(formatTime(value / 1000));
    if (!ffPlayer || isSeeking)
        return;
    seekTargetMs = value;
    lastSeekTime = QDateTime::currentMSecsSinceEpoch();
    ffPlayer->seek(value / 1000.0);
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
            destroyVideoHostWindow();
            videoArea->update();
            setProgressMs(0);
        }
        break;
    }
}
