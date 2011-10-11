#ifndef MINIBROWSER_H
#define MINIBROWSER_H

#include <QDialog>

class QTextBrowser;
class QAction;

class MiniBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit MiniBrowser(const QString &url, QWidget *parent = 0);

private:
    QTextBrowser *m_browser;
    QAction *m_quit,
            *m_back,
            *m_forward,
            *m_toc;
};

#endif // MINIBROWSER_H
