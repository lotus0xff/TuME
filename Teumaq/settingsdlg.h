#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include "ui_settingsdlg.h"

class SettingsDlg : public QDialog, private Ui::SettingsDlg
{
    Q_OBJECT

public:
    explicit SettingsDlg(QWidget *parent = 0);
};

#endif // SETTINGSDLG_H
