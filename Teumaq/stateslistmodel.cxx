#include "stateslistmodel.h"
#include "machine.h"
#include <QBrush>

StatesListModel::StatesListModel(QObject *parent, const Machine *m) :
    QAbstractListModel(parent), _m(m)
{
}

int StatesListModel::rowCount(const QModelIndex &) const
{
    if (_m != NULL)
        return _m->states().size();
    else
        return 0;
}

QVariant StatesListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && _m != NULL &&
        index.row() >= 0 && index.row() < _m->states().size())
        switch (role)
        {
        case Qt::DisplayRole:
            return _m->states().at(index.row());
        case Qt::BackgroundRole:
            if (index.row() == Machine::STATE_INITIAL)
                return QBrush(Qt::green);
            else if (index.row() == Machine::STATE_FINAL)
                return QBrush(Qt::yellow);
            else if (index.row() == Machine::STATE_ILLEGAL)
                return QBrush(Qt::red);
            break;
        }

    return QVariant();
}
