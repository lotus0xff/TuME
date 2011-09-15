#include "prdedtblmodel.h"
#include "parser.h"
#include <QBrush>

bool PrdEdTblModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row+count-1);
    Production p = {
        Machine::STATE_INITIAL,
        Machine::SYMBOL_EMPTY,
        Machine::SYMBOL_EMPTY,
        HMD_STILL,
        Machine::STATE_INITIAL
    };
    int i;
    if (row < _program.size())
        for (i = row; i < row + count; i++)
            _program.insert(i, p);
    else
        for (i = 0; i < count; i++)
            _program.append(p);
    endInsertRows();

    return true;
}

bool PrdEdTblModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row + count > _program.size())
        return false;

    beginRemoveRows(parent, row, row+count-1);
    for (int i = row; i < row + count; i++)
        _program.removeAt(i);
    endRemoveRows();

    return true;
}

QVariant PrdEdTblModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() < _program.size())
    {
        const Production &prd = _program.at(index.row());
        if (role == Qt::BackgroundRole &&
            prd.inState == prd.outState &&
            prd.inState == Machine::STATE_INITIAL &&
            prd.inSymbol == prd.outSymbol &&
            prd.inSymbol== Machine::SYMBOL_EMPTY &&
            prd.moveDir == HMD_STILL)
            return QBrush(Qt::lightGray);
        else if (role == Qt::DisplayRole ||
                 role == Qt::EditRole)
        {
            switch (index.column())
            {
            case COL_L_STA:
                return role == Qt::DisplayRole ?
                       QVariant(_m->stateName(prd.inState)) :
                       QVariant(prd.inState);
            case COL_L_SYM:
                return role == Qt::DisplayRole ?
                       QVariant(_m->symbolName(prd.inSymbol)) :
                       QVariant(prd.inSymbol);
            case COL_MD:
                return role == Qt::DisplayRole ?
                       QVariant(MachineIO::interpret(prd.moveDir)) :
                       QVariant(prd.moveDir);
            case COL_R_STA:
                return role == Qt::DisplayRole ?
                       QVariant(_m->stateName(prd.outState)) :
                       QVariant(prd.outState);
            case COL_R_SYM:
                return role == Qt::DisplayRole ?
                       QVariant(_m->symbolName(prd.outSymbol)) :
                       QVariant(prd.outSymbol);
            case COL_RES:
                QString str;
                if (MachineIO::productionToString(*_m, prd, str) == MachineIO::OK)
                    return str;
            }
        }
    }

    return QVariant();
}

QVariant PrdEdTblModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        switch (section)
        {
        case COL_L_STA:
            return tr("Current state");
        case COL_L_SYM:
            return tr("Observed symbol");
        case COL_MD:
            return tr("Movement direction");
        case COL_R_STA:
            return tr("State to switch to");
        case COL_R_SYM:
            return tr("Symbol to write");
        case COL_RES:
            return tr("TML code");
        }

    return QVariant();
}

bool PrdEdTblModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    if (index.isValid() && role == Qt::EditRole)
    {
        bool ok;
        int idx = index.row(),
            val = value.toInt(&ok);
        if (ok && idx < _program.size())
        {
            switch (index.column())
            {
            case COL_L_STA:
                _program[idx].inState = val;
                break;
            case COL_L_SYM:
                _program[idx].inSymbol = val;
                break;
            case COL_MD:
                _program[idx].moveDir = static_cast<HeadMoveDir>(val);
                break;
            case COL_R_STA:
                _program[idx].outState = val;
                break;
            case COL_R_SYM:
                _program[idx].outSymbol = val;
                break;
            default:
                return false;
            }
            emit dataChanged(index, index);
            return true;
        }
    }

    return false;
}
