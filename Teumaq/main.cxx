#include <QApplication>
#include "centralwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    CentralWindow *cw = new CentralWindow;
    cw->show();

    return app.exec();
}
