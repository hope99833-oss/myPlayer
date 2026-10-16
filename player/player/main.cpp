#include "player.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    player window;
    window.show();
    return app.exec();
}
