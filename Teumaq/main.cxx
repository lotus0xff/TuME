#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QtGui>
#include "machine.h"
#include "parser.h"
#include "centralwindow.h"
#include "tapeview.h"


const char *say(MachineIO::ErrorCode errId)
{
    switch (errId)
    {
    case MachineIO::PROD_L_STATE:
        return "Bad left state";
    case MachineIO::PROD_L_SYMBOL:
        return "Bad left symbol";
    case MachineIO::PROD_OP:
        return "Missed production symbol";
    case MachineIO::PROD_R_STATE:
        return "bad right state";
    case MachineIO::PROD_R_SYMBOL:
        return "bad right symbol";
    case MachineIO::PROD_R_DIR:
        return "bad direction";
    default:
        return QString::number(errId).toStdString().c_str();
    }
}

template <typename T>
QString linkedList2Text(const QList<T> &list)
{
    QString str("[");
    typename QList<T>::ConstIterator itr = list.begin();
    if (itr != list.end())
    {
        str.append(*itr++);
        while (itr != list.end())
            str.append(", ").append(*itr++);
    }
    str.append("]");
    return str;
}

void testMachine()
{
    Machine m;
    MachineIO p(m);

    // Check load
    QFile file("m2o.tml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File opening problems";
        return;
    }
    QTextStream strm(&file);
    QStringList txtTape;
    if (p.load(strm))
    {
        qDebug() << m.symbols();
        qDebug() << m.states();
        qDebug() << m.program();
        p.tape2text(txtTape);
        qDebug() << linkedList2Text(txtTape);
        file.close();
    }
    else
    {
        QString msg = "Line "+QString::number(p.message().line)+
                      say(p.message().error);
        qDebug() << msg;
    }



    Machine::RunResult res = m.exec();
    res = m.exec();
    res = m.exec();
    switch (res.reason)
    {
    case Machine::XR_END:
        qDebug() << "Operation succeeded.";
        break;
    case Machine::XR_LIMIT:
        qDebug() << "Time limit reached... Machine stopped";
        break;
    case Machine::XR_NOPROD:
        qDebug() << "Halt due to incorrect program.";
        break;
    case Machine::XR_BADSTATE:
        qDebug() << "Attemtped to run machine from illegal state";
    }
    txtTape.clear();
    p.tape2text(txtTape);
    qDebug() << linkedList2Text(txtTape);

    // Check save
    // file.setFileName("m2o.tml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "File creating problems";
        return;
    }
    QTextStream strm2(&file);
    if (!p.save(strm2, true))
    {
        QString msg = "Line "+QString::number(p.message().line)+
                      say(p.message().error);
        qDebug() << msg;
    }
    else
        strm2.flush();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    CentralWindow *cw = new CentralWindow;
    cw->show();
    /*
    QWidget *win = new QWidget;
    win->setLayout(new QVBoxLayout());
    TapeView *tv = new TapeView(m);
    QPixmap curimg(":/curimg");
    tv->move(50, 50);
    tv->resize(200, 100);
    tv->setVisibleCells(5);
    tv->setCursor(curimg);
    win->layout()->addWidget(tv);
    QPushButton *bfwd = new QPushButton("Forward", win),
                *bbwd = new QPushButton("Backward", win);
    win->layout()->addWidget(bfwd);
    win->layout()->addWidget(bbwd);
    QObject::connect(bfwd, SIGNAL(clicked()), tv, SLOT(shiftRight()));
    QObject::connect(bfwd, SIGNAL(clicked()), tv, SLOT(repaint()));
    QObject::connect(bbwd, SIGNAL(clicked()), tv, SLOT(shiftLeft()));
    QObject::connect(bbwd, SIGNAL(clicked()), tv, SLOT(repaint()));
    win->show();
    */

    return app.exec();
}
