#ifndef CENTRALWINDOW_H
#define CENTRALWINDOW_H

#include "ui_centralwindow.h"
#include "machine.h"
#include "stateslistmodel.h"
#include "symbolslistmodel.h"
#include "hmdlistmodel.h"
#include "tapelistmodel.h"
#include "cmcbdelegate.h"
#include "prdedtblmodel.h"
#include "options.h"
#include "widgetgroup.h"

class TapeView;
class TraceTableModel;

class CentralWindow : public QMainWindow, private Ui::CentralWindow
{
    Q_OBJECT

public:
    explicit CentralWindow(QWidget *parent = 0);
    virtual ~CentralWindow();

public slots:
    // Emulation
    void about();
    void startExec();
    void stepExec();
    void pauseExec();
    void execFinished();
    void resetMachine();

    bool openText();
    bool saveText();
    bool saveTextAs();
    void createNew();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void on__uiCurOffset_valueChanged(int );
    void on__uiTSymRem_clicked();
    void on__uiTSymSet_clicked();
    void on__uiTSymAdd_clicked();
    void on__uiUseOffset_clicked();
    void chgCurSymbol(int ndx);
    void chgCurState(int ndx);

    // Edition
    void symbolAdd();
    void symbolDel();
    void symbolSet();
    void stateAdd();
    void stateDel();
    void stateSet();
    void on__uiCmdClear_clicked();
    void on__uiCmdApply_clicked();

    void on__uiCmdAdd_clicked();

    void on__uiCmdRem_clicked();

    void on__actAbout_Qt_triggered();

    void on__actDisLog_toggled(bool arg1);

private:
    void setupConnections();
    void setupStatusBar();
    void postSetupUI();
    void updateEmulationTab();
    void updateProgramTab();
    void updateCurrentProduction();
    void reload();

    static const QString FILE_FILTER,
                         TITLE_BASE;

    Machine _machine;
    QList<Machine::LogItem> _machLog;
    Machine::AsyncRunner _asyncRunner;

    /// Status bar indicators
    QLabel *_uiSBTime, *_uiSBExec,
           *_uiSBStatus;

    /// Common machine-related data models
    StatesListModel *_mdlStates;
    SymbolsListModel *_mdlSymbols;
    TapeListModel *_mdlTapeList;
    TraceTableModel *_mdlTrace;
    PrdEdTblModel *_mdlProgram;
    HMDListModel *_mdlHMD;

    /// Spec. delegate
    CMCBDelegate *_delgStates,
                 *_delgSymbols,
                 *_delgHMD;

    /// Widget & action groups
    WidgetGroup<QWidget, 11> _grpWStill;
    WidgetGroup<QAction, 6> _grpAStill;
    WidgetGroup<QAction, 1> _grpAActive;

    /// Global options
    Options _opts;

    /// Current file name
    QString _fileName;
};

#endif // CENTRALWINDOW_H
