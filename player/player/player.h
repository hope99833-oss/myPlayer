#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

class FFPlayer;

class player : public QWidget
{
    Q_OBJECT

public:
    player(QWidget *parent = nullptr);
    ~player();
    void onPlayerProgress(int state, double position, double duration);

public slots:
    void onPlayClicked();
    void onStopClicked();
    void onProgressChanged(int value);
    void onVolumeChanged(int value);
    void onFullscreenClicked();

private:
    void setupUI();
    void setupConnections();
    QPushButton* createIconButton(const QString& iconPath, const QString& tooltip, int size = 36);
    QString formatTime(int seconds);
    void setProgressMs(int ms);

    QWidget* videoArea;
    QPushButton* playBtn;
    QPushButton* stopBtn;
    QPushButton* prevBtn;
    QPushButton* nextBtn;
    QPushButton* fullscreenBtn;
    QPushButton* volumeBtn;
    QSlider* progressSlider;
    QSlider* volumeSlider;
    QLabel* currentTimeLabel;
    QLabel* totalTimeLabel;

    FFPlayer* ffPlayer = nullptr;
    bool isPlaying = false;
    int currentSeconds = 0;
    int totalSeconds = 300;
    bool isSeeking = false;
    qint64 lastSeekTime = 0;
    int seekTargetMs = -1;
};
