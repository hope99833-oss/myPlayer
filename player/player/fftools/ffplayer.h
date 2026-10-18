#ifndef FFPLAYER_H
#define FFPLAYER_H

#ifdef FFPLAY_BUILD_DLL
#define FFPLAYER_API __declspec(dllexport)
#else
#define FFPLAYER_API __declspec(dllimport)
#endif

enum FFPlayerState {
    FFPLAYER_STATE_STOPPED  = 0,   /* stopped by user */
    FFPLAYER_STATE_PLAYING  = 1,
    FFPLAYER_STATE_PAUSED   = 2,
    FFPLAYER_STATE_FINISHED = 3    /* reached end of stream */
};

/* Called from the playback thread (~10 Hz during playback, once on
 * pause/resume transitions and once with the final state on stop/EOF).
 * position/duration are in seconds; duration is 0 until known.
 * NOTE: runs on a non-GUI thread — marshal to the UI thread if needed. */
typedef void (*FFPlayerCallback)(void* userdata, int state, double position, double duration);

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
    void setCallback(FFPlayerCallback callback, void* userdata);

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
