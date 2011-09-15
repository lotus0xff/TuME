#include "symbolslistmodel.h"
#include "machine.h"
#include <QBrush>

SymbolsListModel::SymbolsListModel(QObject *parent, const Machine *m) :
    QAbstractListModel(parent), _m(m)
{
}

int SymbolsListModel::rowCount(const QModelIndex &parent) const
{
    if (_m != NULL)
        return _m->symbols().size();
    else
        return 0;
}

QVariant SymbolsListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && _m != NULL &&
        index.row() >= 0 && index.row() < _m->symbols().size())
        switch (role)
        {
        case Qt::DisplayRole:
            return _m->symbols().at(index.row());
        case Qt::BackgroundRole:
            if (index.row() == Machine::SYMBOL_EMPTY)
                return QBrush(Qt::lightGray);
            else if (index.row() == Machine::SYMBOL_ILLEGAL)
                return QBrush(Qt::red);
            break;
        }

        return QVariant();
}
