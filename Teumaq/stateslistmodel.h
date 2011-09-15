#ifndef STATESLISTMODEL_H
#define STATESLISTMODEL_H

#include <QAbstractListModel>

class Machine;

/// Lists machine states in read-only model
class StatesListModel : public QAbstractListModel
{
public:
    explicit StatesListModel(QObject *parent = 0, const Machine *m = NULL);
    void refresh()
    {
        emit layoutChanged();
    }
    void setMachine(const Machine *m)
    {
        _m = m;
        refresh();
    }
    const Machine *machine() const
    {
        return _m;
    }
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private:
   const Machine *_m;
};

#endif // STATESLISTMODEL_H
