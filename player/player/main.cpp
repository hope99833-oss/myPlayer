#include "player.h"
#include <QtWidgets/QApplication>
#include "ffplayer.h"
#include "ffmpeg_dll.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    player window;
    window.show();
    return app.exec();
}
