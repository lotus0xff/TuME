#ifndef SYMBOLSLISTMODEL_H
#define SYMBOLSLISTMODEL_H

#include <QAbstractListModel>

class Machine;

class SymbolsListModel : public QAbstractListModel
{
public:
    explicit SymbolsListModel(QObject *parent = 0, const Machine *m = NULL);
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

#endif // SYMBOLSLISTMODEL_H
