#include "centralwindow.h"
#include <QApplication>
#include <QStringListModel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include "tapeview.h"
#include "widgetgroup.h"
#include "tracetablemodel.h"
#include "parser.h"
#include "messages.h"

#include "ui_about.h"

const QString CentralWindow::FILE_FILTER = QObject::tr("TML sources (*.tml)");
const QString CentralWindow::TITLE_BASE = QObject::tr("TuME v1.0");

CentralWindow::CentralWindow(QWidget *parent) :
    QMainWindow(parent),
    _asyncRunner(_machine, Machine::MAX_TIME, true, this),
    _mdlStates(NULL), _mdlSymbols(NULL), _mdlTapeList(NULL),
    _mdlProgram(NULL), _mdlHMD(NULL), _delgStates(NULL),
    _delgSymbols(NULL), _delgHMD(NULL),
    _opts(this)
{
    _machine.setLog(&_machLog);

    // Set up UI
    setupUi(this);
    setupStatusBar();
    setupConnections();

    // Complete setup
    postSetupUI();

    setWindowTitle(TITLE_BASE);
}

void CentralWindow::setupConnections()
{
    // Emulation
    connect(_uiNavFwd, SIGNAL(clicked()), _uiTape, SLOT(shiftRight()));
    connect(_uiNavEnd, SIGNAL(clicked()), _uiTape, SLOT(rewindToRight()));
    connect(_uiNavBack, SIGNAL(clicked()), _uiTape, SLOT(shiftLeft()));
    connect(_uiNavBeg, SIGNAL(clicked()), _uiTape, SLOT(rewindToLeft()));
    connect(_uiRewind, SIGNAL(clicked()), _uiTape, SLOT(rewindToCursor()));
    connect(_actStart, SIGNAL(triggered()), this, SLOT(startExec()));
    connect(_actPause, SIGNAL(triggered()), this, SLOT(pauseExec()));
    connect(_actStepFwd, SIGNAL(triggered()), this, SLOT(stepExec()));
    connect(_actReset, SIGNAL(triggered()), this, SLOT(resetMachine()));
    connect(_actOpen, SIGNAL(triggered()), this, SLOT(openText()));
    connect(_actSave, SIGNAL(triggered()), this, SLOT(saveText()));
    connect(_actSave_as, SIGNAL(triggered()), this, SLOT(saveTextAs()));
    connect(_actNew, SIGNAL(triggered()), this, SLOT(createNew()));
    connect(&_asyncRunner, SIGNAL(finished()), this, SLOT(execFinished()));
    connect(&_asyncRunner, SIGNAL(terminated()), this, SLOT(execFinished()));
    connect(_uiCurState, SIGNAL(activated(int)), this, SLOT(chgCurState(int)));
    connect(_uiCurSym, SIGNAL(activated(int)), this, SLOT(chgCurSymbol(int)));

    // Edition
    connect(_uiAbcAdd, SIGNAL(clicked()), this, SLOT(symbolAdd()));
    connect(_uiAbcRem, SIGNAL(clicked()), this, SLOT(symbolDel()));
    connect(_uiAbcSet, SIGNAL(clicked()), this, SLOT(symbolSet()));
    connect(_uiStateAdd, SIGNAL(clicked()), this, SLOT(stateAdd()));
    connect(_uiStateRem, SIGNAL(clicked()), this, SLOT(stateDel()));
    connect(_uiStateSet, SIGNAL(clicked()), this, SLOT(stateSet()));
}

void CentralWindow::setupStatusBar()
{
    _uiSBTime = new QLabel(statusBar());
    _uiSBTime->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _uiSBTime->setAlignment(Qt::AlignCenter);
    _uiSBTime->setToolTip(tr("Current time"));
    _uiSBExec = new QLabel(statusBar());
    _uiSBExec->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _uiSBExec->setToolTip(tr("Machine execution status"));
    _uiSBStatus = new QLabel(statusBar());
    _uiSBStatus->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _uiSBStatus->setToolTip(tr("Current status"));

    statusBar()->addPermanentWidget(_uiSBTime, 1);
    statusBar()->addPermanentWidget(_uiSBExec, 4);
    statusBar()->addPermanentWidget(_uiSBStatus, 4);

    _uiSBTime->setText(tr("0"));
    _uiSBStatus->setText(tr("Ready"));
}

void CentralWindow::postSetupUI()
{
    // Set up validators
    _uiStateText->setValidator(new QRegExpValidator(QRegExp("\\w+"), _uiStateText));
    _uiAbcText->setValidator(new QRegExpValidator(QRegExp("(?:\\w+\\s*)+"), _uiAbcText));

    // Set up models
    _mdlStates = new StatesListModel(this, &_machine);
    _mdlSymbols = new SymbolsListModel(this, &_machine);
    _mdlTapeList = new TapeListModel(this, &_machine);
    _mdlTrace = new TraceTableModel(this, &_machine);
    _mdlProgram = new PrdEdTblModel(this, &_machine);
    _mdlHMD = new HMDListModel(this);

    _uiTrace->setModel(_mdlTrace);
    _uiCurState->setModel(_mdlStates);
    _uiCurSym->setModel(_mdlSymbols);
    _uiSetStates->setModel(_mdlStates);
    _uiSetSymbols->setModel(_mdlSymbols);
    _uiTSymSel->setModel(_mdlSymbols);
    _uiTapeList->setModel(_mdlTapeList);

    // Set up delegate
    _delgStates = new CMCBDelegate(_mdlStates, this);
    _delgSymbols = new CMCBDelegate(_mdlSymbols, this);
    _delgHMD = new CMCBDelegate(_mdlHMD, this);

    _uiProgram->setModel(_mdlProgram);
    _uiProgram->setItemDelegateForColumn(PrdEdTblModel::COL_L_STA, _delgStates);
    _uiProgram->setItemDelegateForColumn(PrdEdTblModel::COL_R_STA, _delgStates);
    _uiProgram->setItemDelegateForColumn(PrdEdTblModel::COL_L_SYM, _delgSymbols);
    _uiProgram->setItemDelegateForColumn(PrdEdTblModel::COL_R_SYM, _delgSymbols);
    _uiProgram->setItemDelegateForColumn(PrdEdTblModel::COL_MD, _delgHMD);

    // Set up groups
    _grpAStill.clear();
    _grpAStill << _actNew << _actOpen << _actSave
               << _actSave_as << _actStart << _actStepFwd;
    _grpAActive.clear();
    _grpAActive << _actPause;
    _grpWStill.clear();
    _grpWStill << _uiRun << _uiStepFwd << _uiStepCount
               << _uiNavBeg << _uiNavBack << _uiNavFwd << _uiNavEnd
               << _uiCurState << _uiCurSym << _uiCurCmd
               << _uiTape << _uiTrace << _uiSctProgram;
    _grpWActive.clear();
    _grpWActive << _uiPause;

    _uiTape->setMachine(&_machine);
    _uiTape->cursor().load(":/machine/cursor");
    _uiTape->setVisibleCells(7);

    updateEmulationTab();
}

CentralWindow::~CentralWindow()
{
}

void CentralWindow::updateEmulationTab()
{
    _mdlStates->refresh();
    _mdlSymbols->refresh();
    _uiCurState->setCurrentIndex(_machine.currentState());
    _uiCurSym->setCurrentIndex(*_machine.cursor());
    _mdlTrace->refresh();
    _uiTape->update();
    _uiSBTime->setText(QString::number(_machine.currentTime()));

    // TEMP
    _mdlTapeList->refresh();

    updateCurrentProduction();
}

void CentralWindow::updateCurrentProduction()
{
    Production prd;
    QString str;
    if (_machine.currentProduction(prd))
        MachineIO::productionToString(_machine, prd, str);
    if (str.isEmpty())
        str = tr("Not available");
    _uiCurCmd->setText(str);
}

void CentralWindow::updateProgramTab()
{
    _uiCurOffset->setValue(static_cast<int>(_machine.offset()));
}


/// Completes application initialization. Must be
/// called after each machine change.
void CentralWindow::reload()
{
    _mcopy = _machine;
    _uiTape->rewindToCursor();
    updateEmulationTab();
    _mdlProgram->load();
    updateProgramTab();
}

void CentralWindow::about()
{
    QDialog *dlg = new QDialog(this);
    Ui::About decorator;
    decorator.setupUi(dlg);
    dlg->show();
}


void CentralWindow::close()
{
    int res = QMessageBox::question(this, tr("Quit?"),
                                          tr("Are you sure?"),
                                          QMessageBox::Yes,
                                          QMessageBox::No);
    if (res == QMessageBox::Yes)
        QMainWindow::close();
}

void CentralWindow::startExec()
{
    _uiSBStatus->setText(tr("Running..."));
    _uiSBExec->setText(QString());

    _asyncRunner.start();

    _grpWStill.setEnabled(false);
    _grpAStill.setEnabled(false);
    _grpWActive.setEnabled(true);
    _grpAActive.setEnabled(true);
}

void CentralWindow::stepExec()
{
    int steps = _uiStepCount->value();
    Machine::RunResult rr = _machine.exec(steps);
    if (rr.reason == Machine::XR_LIMIT)
        _uiSBStatus->setText(tr("%1 commands executed").arg(QString::number(steps)));
    QString buf;
    if (Messages::machineResult(buf, rr.reason))
        _uiSBExec->setText(buf);
    updateEmulationTab();
}

void CentralWindow::pauseExec()
{
    if (_asyncRunner.isRunning())
    {
        _asyncRunner.stop();
        _asyncRunner.wait();
    }
}

void CentralWindow::execFinished()
{
    _uiSBStatus->setText(tr("Execution finished"));

    _grpWStill.setEnabled(true);
    _grpAStill.setEnabled(true);
    _grpWActive.setEnabled(false);
    _grpAActive.setEnabled(false);

    QString buf;
    if (Messages::machineResult(buf, _asyncRunner.result().reason))
        _uiSBExec->setText(buf);

    updateEmulationTab();
}

void CentralWindow::resetMachine()
{
    int r = QMessageBox::question(this,
                                  tr("Warning!"),
                                  tr("Resetting will revert all changes to the last save point.\n"\
                                     "Are you sure?"),
                                  QMessageBox::Yes, QMessageBox::No);
    if (r == QMessageBox::No)
        return;

    _uiSBStatus->setText(tr("Stopping and cleaning"));
    if (_asyncRunner.isRunning())
    {
        _asyncRunner.stop();
        _asyncRunner.wait();
    }
    _machine = _mcopy;
    _machLog.clear();
    _mdlTrace->clearIntervPoints();
    _uiTape->rewindToCursor();
    updateEmulationTab();
    _mdlProgram->load();
    _uiSBStatus->setText(tr("Reset done"));
}

void CentralWindow::chgCurSymbol(int ndx)
{
    if (_machine.changeCurrentSymbol(ndx))
    {
        _mdlTrace->setIntervPoint(_machine.currentTime());
        _mdlTrace->refresh();
        _uiTape->update();
        updateCurrentProduction();
    }
}

void CentralWindow::chgCurState(int ndx)
{
    if (_machine.changeCurrentState(ndx))
    {
        _mdlTrace->setIntervPoint(_machine.currentTime());
        _mdlTrace->refresh();
        updateCurrentProduction();
    }
}

/// Load program from text file
bool CentralWindow::openText()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Open program source"),
                                                QString(),
                                                FILE_FILTER);
    if (!file.isNull())
    {
        QFile fin(file);
        if (fin.open(QIODevice::ReadOnly))
        {
            QTextStream strm(&fin);
            MachineIO mio(_machine);
            if (mio.load(strm))
            {
                reload();
                _fileName = file;
                _uiSBStatus->setText(tr("Source loaded"));
                setWindowTitle(TITLE_BASE + " - " + _fileName);
                return true;
            }
            else
            {
                QString message = tr("Line %1: %2\nLoad process interrupted."), buf;
                Messages::parserError(buf, mio.message().error);
                message = message.arg(QString::number(mio.message().line))
                                 .arg(buf);
                QMessageBox::warning(this,
                                     tr("Parser error"),
                                     message);
            }
            fin.close();
        }
        else
            QMessageBox::critical(this,
                                  tr("Read error"),
                                  tr("Cannot read data from\n%1").arg(fin.fileName()));
    }

    return false;
}

/// Save program to a text file
bool CentralWindow::saveText()
{
    if (_fileName.isEmpty())
    {
        QString file = QFileDialog::getSaveFileName(this,
                                                    tr("Select source file to save"),
                                                    QString(),
                                                    FILE_FILTER);
        if (!file.isNull())
            _fileName = file;
        else
            return false;
    }

    // assume that _fileName contains something reasonable
    QFile fout(_fileName);
    if (fout.open(QIODevice::WriteOnly))
    {
        QTextStream strm(&fout);
        MachineIO mio(_machine);
        if (mio.save(strm, true))
        {
            _mcopy = _machine;
            fout.close();
            _uiSBStatus->setText(tr("Save in text format successful"));
            setWindowTitle(TITLE_BASE + " - " + _fileName);
            return true;
        }
        else
        {
            fout.close();
            // A rare case, but...
            QString message = tr("%2\nData is not saved."), buf;
            Messages::parserError(buf, mio.message().error);
            QMessageBox::warning(this,
                                 tr("Parser error"),
                                 message.arg(buf));
        }
    }
    else
        QMessageBox::critical(this,
                              tr("Write error"),
                              tr("Cannot write data to\n%1").arg(fout.fileName()));

    return false;
}

/// Save as...
bool CentralWindow::saveTextAs()
{
    QString tmpName = _fileName;
    _fileName.clear();
    if (saveText())
        return true;
    else
        _fileName = tmpName;
    return false;
}

void CentralWindow::createNew()
{
    _machine.reset(Machine::RM_ERASE_DEF);
    reload();
    _fileName.clear();
    setWindowTitle(TITLE_BASE);
    _uiSBExec->clear();
    _uiSBStatus->setText(tr("New machine created"));
}

void CentralWindow::symbolAdd()
{
    QString strSym = _uiAbcText->text();
    if (!strSym.isEmpty() &&
        _machine.addSymbol(strSym) != Machine::SYMBOL_ILLEGAL)
    {
        _mdlSymbols->refresh();
        _uiAbcText->clear();
    }
}

void CentralWindow::symbolDel()
{
    symbol_t sym = _machine.lastSymbol();
    if (_machine.validSymbol(sym))
    {
        if (!_machine.symbolInUse(sym))
        {
            _machine.delLastSymbol();
            _mdlSymbols->refresh();
            _uiAbcText->clear();
        }
        else
            QMessageBox::information(this,
                                     tr("Not permitted"),
                                     tr("Symbol '%1' is currently in use. "\
                                        "Remove it from the tape and/or commands "\
                                        "and try again.").arg(_machine.symbolName(sym)));
    }
}

void CentralWindow::symbolSet()
{
    QModelIndexList sel = _uiSetSymbols->selectionModel()->selectedIndexes();
    if (!sel.isEmpty() &&
        sel.first().isValid())
    {
        QString strSym = _uiAbcText->text();
        symbol_t sym = static_cast<symbol_t>(sel.first().row());
        if (_machine.validSymbol(sym) && !strSym.isEmpty())
        {
            if (_machine.setSymbol(sym, strSym))
            {
                _mdlSymbols->refresh();
                _uiAbcText->clear();
            }
            else
                QMessageBox::warning(this,
                                     tr("Illegal symbol literal"),
                                     tr("'%1' is not eligible symbol literal.\n"\
                                        "You can use letters, digits, '_' and space.")
                                     .arg(strSym));
        }
    }
}

void CentralWindow::stateAdd()
{
    QString strStt = _uiStateText->text();
    if (!strStt.isEmpty() &&
        _machine.addState(strStt) != Machine::STATE_ILLEGAL)
    {
        _mdlStates->refresh();
        _uiStateText->clear();
    }
}

void CentralWindow::stateDel()
{
    state_t sta = _machine.lastState();
    if (_machine.validState(sta))
    {
        if (!_machine.stateInUse(sta))
        {
            _machine.delLastState();
            _mdlStates->refresh();
            _uiStateText->clear();
        }
        else
            QMessageBox::information(this,
                                     tr("Not permitted"),
                                     tr("State '%1' is currently in use. "\
                                        "Remove in trom the commands "\
                                        "and try again").arg(_machine.stateName(sta)));
    }
}

void CentralWindow::stateSet()
{
    QModelIndexList sel = _uiSetStates->selectionModel()->selectedIndexes();
    if (!sel.isEmpty() &&
        sel.first().isValid())
    {
        QString strStt = _uiStateText->text();
        state_t stt = static_cast<state_t>(sel.first().row());
        if (_machine.validState(stt) && !strStt.isEmpty())
        {
            if (_machine.setState(stt, strStt))
            {
                _mdlStates->refresh();
                _uiStateText->clear();
            }
            else
                QMessageBox::warning(this,
                                     tr("Illegal state literal"),
                                     tr("'%1' is not eligible state literal.\n"\
                                        "You can use letters, digits and '_', no spaces.")
                                     .arg(strStt));
        }
    }
}

void CentralWindow::on__uiUseOffset_clicked()
{
    QModelIndexList lstTapeSel = _uiTapeList->selectionModel()->selectedIndexes();
    if (!lstTapeSel.isEmpty() && lstTapeSel.first().isValid())
    {
        int row = lstTapeSel.first().row();
        if (row != _uiCurOffset->value())
            _uiCurOffset->setValue(row);
        else
        {
            _machine.moveStaticCursor(row);
            _uiTape->rewindToCursor();
        }
    }
}

void CentralWindow::on__uiTSymAdd_clicked()
{
    symbol_t sym = static_cast<symbol_t>(_uiTSymSel->currentIndex());
    if (_machine.validSymbol(sym))
    {
        _machine.appendToTape(sym);
        _mdlTapeList->refresh();
    }
}

void CentralWindow::on__uiTSymSet_clicked()
{
    QModelIndexList lstTapeSel = _uiTapeList->selectionModel()->selectedIndexes();
    symbol_t sym = static_cast<symbol_t>(_uiTSymSel->currentIndex());
    if (!lstTapeSel.isEmpty() &&
        lstTapeSel.first().isValid() &&
        _machine.validSymbol(sym))
    {
        size_t off = static_cast<size_t>(lstTapeSel.first().row());
        _machine.setTapeSymbol(off, sym);
        _mdlTapeList->refresh();
    }
}

void CentralWindow::on__uiTSymRem_clicked()
{
    QModelIndexList lstTapeSel = _uiTapeList->selectionModel()->selectedIndexes();
    symbol_t sym = static_cast<symbol_t>(_uiTSymSel->currentIndex());
    if (!lstTapeSel.isEmpty() &&
        lstTapeSel.first().isValid() &&
        _machine.validSymbol(sym))
    {
        int maxIndex = _mdlTapeList->rowCount(QModelIndex()) - 1;
        if (maxIndex == 0)
            return;
        _machine.delTapeSymbol(lstTapeSel.first().row());
        if (_uiCurOffset->value() > maxIndex)
            _uiCurOffset->setValue(maxIndex);
        else
            _uiTape->rewindToCursor();
        _mdlTapeList->refresh();
    }
}

void CentralWindow::on__uiCurOffset_valueChanged(int off)
{
    _machine.moveStaticCursor(off);
    _uiTape->rewindToCursor();
}

void CentralWindow::on__uiCmdClear_clicked()
{
    int res = QMessageBox::question(this,
                                    tr("Confirm revert"),
                                    tr("Are you sure want to revert program "\
                                       "to its initial state?"),
                                    QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes)
        _mdlProgram->load();
}

void CentralWindow::on__uiCmdApply_clicked()
{
    int res = QMessageBox::question(this,
                                    tr("Confirm apply"),
                                    tr("Are you sure want to apply changes "\
                                       "you have made to program?"),
                                    QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
        _mdlProgram->commit();
        // _mcopy.copyProgram(_machine.program());
        updateEmulationTab();
    }
}


void CentralWindow::on__uiCmdAdd_clicked()
{
    QModelIndexList ilist = _uiProgram->selectionModel()->selectedIndexes();
    int ndx = !ilist.isEmpty() ?
              ilist.first().row() :
              _uiProgram->model()->rowCount();
    _uiProgram->model()->insertRow(ndx);
}

void CentralWindow::on__uiCmdRem_clicked()
{
    QModelIndexList ilist = _uiProgram->selectionModel()->selectedIndexes();
    if (!ilist.isEmpty())
        _uiProgram->model()->removeRow(ilist.first().row());
}

void CentralWindow::on__actAbout_Qt_triggered()
{
    qobject_cast<QApplication*>(QApplication::instance())->aboutQt();
}

void CentralWindow::on__actDisLog_toggled(bool logOff)
{
    if (!logOff)
    {
        _machine.setLog(&_machLog);
    }
    else
    {
        _machLog.clear();
        _machine.setLog(NULL);
    }
    _uiTrace->setEnabled(!logOff);
    _uiTrace->setModel(logOff ? NULL : _mdlTrace);
}
