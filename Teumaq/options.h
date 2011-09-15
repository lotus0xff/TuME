#ifndef OPTIONS_H
#define OPTIONS_H

#include <QSettings>
#include "settingsdlg.h"

#define ORGANIZATION "White Lotus"
#define APPLICATION "TuME"

/// Proxy class for keepin' settings and allowing user to edit them
class Options
{
    const QObject * const _owner;
    QSettings _settings;
    SettingsDlg *_window;

public:
    Options(QObject * const owner):
        _owner(owner),
        _settings(QObject::tr(ORGANIZATION), QObject::tr(APPLICATION), owner),
        _window(NULL)
    { }
};

#endif // OPTIONS_H
