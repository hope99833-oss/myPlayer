#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>

class player : public QWidget
{
    Q_OBJECT

public:
    player(QWidget *parent = nullptr);
    ~player();

private slots:
    void onPlayClicked();
    void onStopClicked();
    void onProgressChanged(int value);
    void onVolumeChanged(int value);
    void onFullscreenClicked();
    void updateTimeLabel();

private:
    void setupUI();
    void setupConnections();
    QPushButton* createIconButton(const QString& iconPath, const QString& tooltip, int size = 36);
    QString formatTime(int seconds);

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
    QTimer* timer;

    bool isPlaying = false;
    int currentSeconds = 0;
    int totalSeconds = 300;
};
