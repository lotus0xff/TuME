#ifndef TRACETABLEMODEL_H
#define TRACETABLEMODEL_H

#include <QAbstractTableModel>
#include <QSet>

class Machine;

class TraceTableModel : public QAbstractTableModel
{
public:
    explicit TraceTableModel(QObject *parent = 0, const Machine *m = NULL);
    void refresh()
    {
        emit layoutChanged();
    }
    void setIntervPoint(quint64 time)
    {
        _irvSet.insert(time);
    }
    void clearIntervPoint(quint64 time)
    {
        _irvSet.remove(time);
    }
    void clearIntervPoints()
    {
        _irvSet.clear();
    }
    void setMachine(const Machine *m)
    {
        _machine = m;
        refresh();
    }
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const
    {
        return 4;
    }
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    /// Column indices
    enum
    {
        COL_TIME = 0, COL_STATE, COL_SYM, COL_PROD
    };

    const Machine *_machine;
    QSet<quint64> _irvSet;
};

#endif // TRACETABLEMODEL_H
