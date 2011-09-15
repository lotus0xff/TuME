#ifndef HMDLISTMODEL_H
#define HMDLISTMODEL_H

#include <QAbstractListModel>
#include "machine.h"
#include "parser.h"

class HMDListModel : public QAbstractListModel
{
public:
    HMDListModel(QObject *parent = NULL):
        QAbstractListModel(parent)
    { }
    virtual int rowCount(const QModelIndex &parent) const
    {
        return 3;
    }
    virtual QVariant data(const QModelIndex &index, int role) const
    {
        if (index.isValid() && role == Qt::DisplayRole)
        {
            QString md = MachineIO::interpret(static_cast<HeadMoveDir>(index.row()));
            if (!md.isNull())
                return md;
        }
        return QVariant();
    }
};

#endif // HMDLISTMODEL_H
