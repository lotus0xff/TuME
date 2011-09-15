#include "cmcbdelegate.h"
#include <QComboBox>
#include <QPainter>

QWidget *CMCBDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
    const
{
    if (_listModel)
    {
        QComboBox *combo = new QComboBox(parent);
        combo->setModel(_listModel);
        return combo;
    }

    return QItemDelegate::createEditor(parent, option, index);
}

void CMCBDelegate::setEditorData(QWidget *editor, const QModelIndex &index)
    const
{
    if (_listModel)
    {
        int ndx = index.model()->data(index, Qt::EditRole).toInt();
        QComboBox *combo = qobject_cast<QComboBox*>(editor);
        combo->setCurrentIndex(ndx);
    }
    else
        QItemDelegate::setEditorData(editor, index);
}

void CMCBDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index)
    const
{
    if (_listModel)
    {
        QComboBox *combo = qobject_cast<QComboBox*>(editor);
        model->setData(index, combo->currentIndex(), Qt::EditRole);
    }
    else
        QItemDelegate::setModelData(editor, model, index);
}
