#ifndef SELECT_DIRECTORY_ITEM_DELEGATE_H
#define SELECT_DIRECTORY_ITEM_DELEGATE_H

#include <QItemDelegate>

class SelectDirectoryItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SelectDirectoryItemDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, 
        const QStyleOptionViewItem &option, const QModelIndex & index) const;
    
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;

};

#endif
