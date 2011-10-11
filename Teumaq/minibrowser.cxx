#include "minibrowser.h"
#include <QTextBrowser>
#include <QAction>
#include <QMenuBar>
#include <QVBoxLayout>

MiniBrowser::MiniBrowser(const QString &url, QWidget *parent) :
    QDialog(parent)
{
    // Build UI
    m_browser = new QTextBrowser(this);
    QMenuBar *mbar = new QMenuBar(this);
    mbar->setNativeMenuBar(true);
    setLayout(new QVBoxLayout(this));
    layout()->setMenuBar(mbar);
    layout()->addWidget(m_browser);

    // Set up actions
    m_toc = new QAction("Contents", this);
    connect(m_toc, SIGNAL(triggered()), m_browser, SLOT(home()));
    m_back = new QAction("Back", this);
    connect(m_back, SIGNAL(triggered()), m_browser, SLOT(backward()));
    m_forward = new QAction("Forward", this);
    connect(m_forward, SIGNAL(triggered()), m_browser, SLOT(forward()));
    m_quit = new QAction("Quit", this);
    connect(m_quit, SIGNAL(triggered()), SLOT(close()));

    QMenu *menu = mbar->addMenu("Navigation");
    menu->addAction(m_toc);
    menu->addAction(m_back);
    menu->addAction(m_forward);
    menu->addSeparator();
    menu->addAction(m_quit);

    m_browser->setSource(QUrl(url));
}

