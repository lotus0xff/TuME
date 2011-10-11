#include <QApplication>
#include "centralwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    CentralWindow win;
    win.showMaximized();

    return app.exec();
}
