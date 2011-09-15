#ifndef CMCBDELEGATE_H
#define CMCBDELEGATE_H

#include <QItemDelegate>
#include <QAbstractListModel>
#include <QScopedPointer>

/// Custom Model ComboBox Delegate
class CMCBDelegate : public QItemDelegate
{
    QAbstractListModel *_listModel;

public:
    CMCBDelegate(QAbstractListModel *mdl, QObject *parent = NULL):
        QItemDelegate(parent), _listModel(mdl)
    {
    }
    void setModel(QAbstractListModel *mdl)
    {
        _listModel = mdl;
    }
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                              const QModelIndex &index) const;
};

#endif // CMCBDELEGATE_H
