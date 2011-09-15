#include "tracetablemodel.h"
#include "machine.h"
#include "parser.h"
#include <QBrush>

TraceTableModel::TraceTableModel(QObject *parent, const Machine *m) :
    QAbstractTableModel(parent), _machine(m)
{
}

int TraceTableModel::rowCount(const QModelIndex &parent)
    const
{
    if (_machine != NULL && _machine->log() != NULL)
        return _machine->log()->size();
    else
        return 0;
}

QVariant TraceTableModel::data(const QModelIndex &index, int role)
    const
{
    if (index.isValid() && _machine != NULL &&
        _machine->log() != NULL)
    {
        Machine::LogItem item = _machine->log()->at(index.row());
        switch (role)
        {
        case Qt::DisplayRole:
            if (Machine::isSpecial(item.cmd))
                return "...";
            switch (index.column())
            {
            case COL_TIME:
                return QVariant(item.time);
            case COL_STATE:
                return QVariant(_machine->stateName(item.cmd.inState));
            case COL_SYM:
                return QVariant(_machine->symbolName(item.cmd.inSymbol));
            case COL_PROD:
                QString str;
                if (MachineIO::productionToString(*_machine, item.cmd, str) != MachineIO::OK)
                    str = tr("Bad command");
                return QVariant(str);
            }
            break;
        case Qt::BackgroundRole:
            if (_irvSet.contains(item.time))
                return QBrush(Qt::red);
            break;
        }
    }

    return QVariant();
}

QVariant TraceTableModel::headerData(int section, Qt::Orientation orientation, int role)
    const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        QString str;
        switch (section)
        {
        case COL_TIME:
            str = tr("Time");
            break;
        case COL_STATE:
            str = tr("State");
            break;
        case COL_SYM:
            str = tr("Symbol");
            break;
        case COL_PROD:
            str = tr("Chosen command");
        }
        return QVariant(str);
    }

    return QVariant();
}
