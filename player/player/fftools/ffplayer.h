#ifndef FFPLAYER_H
#define FFPLAYER_H

#ifdef FFPLAY_BUILD_DLL
#define FFPLAYER_API __declspec(dllexport)
#else
#define FFPLAYER_API __declspec(dllimport)
#endif

class FFPLAYER_API FFPlayer {
public:
    FFPlayer(const char* filename, void* hwnd, int width, int height);
    ~FFPlayer();

    int  play();
    void stop();
    void pause();
    void resume();
    void seek(double seconds);
    void setVolume(int volume);
    void toggleMute();
    void toggleFullScreen();

    bool isRunning() const;
    bool isPaused() const;
    double getDuration() const;
    double getPosition() const;

private:
    struct Impl;
    Impl* m;
    friend int player_run(Impl* impl);
    friend int play_thread_fn(void* arg);
};

#endif /* FFPLAYER_H */
