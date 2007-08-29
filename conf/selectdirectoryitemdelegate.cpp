#include "selectdirectoryitemdelegate.h"

#include <QHBoxLayout>
#include <QLabel>

#include <KUrlRequester>

SelectDirectoryItemDelegate::SelectDirectoryItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *SelectDirectoryItemDelegate::createEditor(QWidget *parent, 
            const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    KUrlRequester *widget = new KUrlRequester(parent);
    widget->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    widget->setPath(index.model()->data(index, Qt::DisplayRole).toString());

    return widget;
}

void SelectDirectoryItemDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

void SelectDirectoryItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    KUrlRequester *fileWidget = static_cast<KUrlRequester*>(editor);
    QString path = fileWidget->url().pathOrUrl();

    if(QString::compare(path, "") != 0) {
        model->setData(index, path);
    }
}
