#ifndef TAPELISTMODEL_H
#define TAPELISTMODEL_H

#include <QAbstractListModel>
#include <QList>

class Machine;

class TapeListModel : public QAbstractListModel
{
public:
    explicit TapeListModel(QObject *owner = NULL, const Machine *m = NULL);
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

#endif // TAPELISTMODEL_H
