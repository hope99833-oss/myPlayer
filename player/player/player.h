#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>
#include <QTimer>

class FFPlayer;

// The QSS-styled QSlider in this Qt build renders groove/sub-page/handle from
// stale cached geometry (full-width fill, ghost segments after any layout
// change), so the bar is painted straight from the current value instead.
class SeekSlider : public QSlider
{
public:
    using QSlider::QSlider;

    struct Look {
        int grooveH = 6;
        int handleW = 14;
        QColor groove{0x3a, 0x3a, 0x3e};
        QColor fill{0x00, 0x78, 0xd4};
        QColor handle{0xff, 0xff, 0xff};
        QColor handleBorder{0x00, 0x78, 0xd4};
        int borderW = 2;
    };
    void setLook(const Look& l) { look = l; update(); }
    int handleWidth() const { return look.handleW; }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const double span = maximum() - minimum();
        const double f = span > 0 ? qBound(0.0, (value() - minimum()) / span, 1.0) : 0.0;
        const qreal hw = look.handleW;
        const QRectF groove(hw / 2, (height() - look.grooveH) / 2.0,
                            width() - hw, look.grooveH);
        p.setPen(Qt::NoPen);
        p.setBrush(look.groove);
        p.drawRoundedRect(groove, look.grooveH / 2.0, look.grooveH / 2.0);
        p.setBrush(look.fill);
        p.drawRoundedRect(QRectF(groove.left(), groove.top(),
                                 groove.width() * f, groove.height()),
                          look.grooveH / 2.0, look.grooveH / 2.0);
        const qreal cx = groove.left() + groove.width() * f;
        p.setBrush(look.handle);
        if (look.borderW > 0)
            p.setPen(QPen(look.handleBorder, look.borderW));
        p.drawEllipse(QRectF(cx - hw / 2, (height() - hw) / 2.0, hw, hw)
                          .adjusted(look.borderW / 2.0, look.borderW / 2.0,
                                    -look.borderW / 2.0, -look.borderW / 2.0));
    }

private:
    Look look;
};

class player : public QWidget
{
    Q_OBJECT

public:
    player(QWidget *parent = nullptr);
    ~player();
    void onPlayerProgress(int state, double position, double duration);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

public slots:
    void onPlayClicked();
    void onStopClicked();
    void onOpenFileClicked();
    void onProgressChanged(int value);
    void onVolumeChanged(int value);
    void onFullscreenClicked();

private:
    void setupUI();
    void setupConnections();
    void startFile(const QString& path);
    QPushButton* createIconButton(const QString& iconPath, const QString& tooltip, int size = 36);
    QString formatTime(int seconds);
    void setProgressMs(int ms);
    int sliderValueFromPos(const QPoint& pos) const;
    void scrubTo(int valueMs, bool force);
    void updateMaximizeIcon();
    void updateMuteIcon();
    void syncVideoWindow();
    void applyVideoHostRect(int x, int y, int w, int h);
    void destroyVideoHostWindow();
    void toggleMaximize();

    QWidget* videoContainer;
    QWidget* videoArea;
    void* videoHostWnd = nullptr;   // owned top-level window hosting the SDL surface
    int lastVideoW = -1;
    int lastVideoH = -1;
    QPoint lastVideoTl;
    QSize lastWindowSize;           // last client size, to know which band a resize exposed
    void paintExposedStrip(int winW, int winH);
    QWidget* titleBar;
    QWidget* controlBar;
    QLabel* titleLabel;
    QPushButton* minBtn;
    QPushButton* maxBtn;
    QPushButton* closeBtn;
    QPushButton* playBtn;
    QPushButton* stopBtn;
    QPushButton* openBtn;
    QPushButton* fullscreenBtn;
    QPushButton* volumeBtn;
    SeekSlider* progressSlider;
    SeekSlider* volumeSlider;
    QLabel* currentTimeLabel;
    QLabel* totalTimeLabel;

    FFPlayer* ffPlayer = nullptr;
    QString currentFile;
    bool isPlaying = false;
    int totalSeconds = 300;
    bool isSeeking = false;
    qint64 lastSeekTime = 0;
    int seekTargetMs = -1;
    QPoint dragStartPos;
    bool dragging = false;
    QTimer* hoverTimer = nullptr;   // fullscreen control-bar auto-hide poller
};
