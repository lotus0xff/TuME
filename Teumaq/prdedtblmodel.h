#ifndef PRDEDTBLMODEL_H
#define PRDEDTBLMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "machine.h"

class PrdEdTblModel : public QAbstractTableModel
{
public:
    enum
    {
        COL_L_STA = 0, COL_L_SYM, COL_MD,
        COL_R_STA, COL_R_SYM, COL_RES
    };

    PrdEdTblModel(QObject *owner = NULL, Machine *m = NULL):
        QAbstractTableModel(owner), _m(m)
    {
        if (_m)
            load();
    }
    void load(Machine *m = NULL)
    {
        if (!m)
            m = _m;
        _program.clear();
        
        // Following code removed for compatibility with
        // elder versions:
        // _program.reserve(m->program().size()); 
        
        foreach (const Production &cmd, m->program())
            _program.append(cmd);
        emit layoutChanged();
    }
    void commit(Machine *m = NULL)
    {
        if (!m)
            m = _m;
        m->eraseProgram();
        foreach (const Production &cmd, _program)
            m->addProduction(cmd);
    }
    void refresh()
    {
        emit layoutChanged();
    }
    void setMachine(Machine *m)
    {
        _m = m;
        load();
    }
    const Machine *machine() const
    {
        return _m;
    }
    virtual bool insertRows(int row, int count, const QModelIndex &parent);
    virtual bool removeRows(int row, int count, const QModelIndex &parent);
    virtual int rowCount(const QModelIndex &) const
    {
        return _program.size();
    }
    virtual int columnCount(const QModelIndex &) const
    {
        return 6;
    }
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags flgs = QAbstractTableModel::flags(index);
        if (index.isValid() && index.column() != COL_RES)
            flgs |= Qt::ItemIsEditable;
        return flgs;
    }
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
    Machine *_m;
    QList<Production> _program;
};

#endif // PRDEDTBLMODEL_H
