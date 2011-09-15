#include "tapelistmodel.h"
#include "machine.h"

TapeListModel::TapeListModel(QObject *owner, const Machine *m):
    QAbstractListModel(owner), _m(m)
{
}

int TapeListModel::rowCount(const QModelIndex &parent) const
{
    if (_m)
        return _m->tape().size();
    return 0;
}

QVariant TapeListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        Machine::Cursor_RO itr;
        _m->setToTapeFirst(itr);
        itr += index.row();
        if (_m->validCursor(itr))
        {
            switch (role)
            {
            case Qt::DisplayRole:
                return _m->symbolName(*itr);
            }
        }
    }

    return QVariant();
}
