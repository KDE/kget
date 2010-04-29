/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "dlgchecksumsearch.h"

#include "kget_export.h"

#include "core/verifier.h"

#include "checksumsearch.h"
#include "checksumsearchsettings.h"

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>
#include <QtGui/QStringListModel>

KGET_EXPORT_PLUGIN_CONFIG(DlgChecksumSettingsWidget)

const KUrl ChecksumSearchAddDlg::URL = KUrl("http://www.example.com/file.zip");

ChecksumSearchAddDlg::ChecksumSearchAddDlg(QStringListModel *modesModel, QStringListModel *typesModel, QWidget *parent, Qt::WFlags flags)
  : KDialog(parent, flags),
    m_modesModel(modesModel),
    m_typesModel(typesModel)
{
    setCaption(i18n("Add item"));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    if (m_modesModel)
    {
        ui.mode->setModel(m_modesModel);
    }
    if (m_typesModel)
    {
        ui.type->setModel(m_typesModel);
    }

    slotUpdate();

    connect(ui.change, SIGNAL(textChanged(QString)), this, SLOT(slotUpdate()));
    connect(ui.mode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdate()));
    connect(this, SIGNAL(accepted()), this, SLOT(slotAccpeted()));
}

void ChecksumSearchAddDlg::slotUpdate()
{
    enableButtonOk(!ui.change->text().isEmpty());

    const ChecksumSearch::UrlChangeMode mode = static_cast<ChecksumSearch::UrlChangeMode>(ui.mode->currentIndex());
    const KUrl modifiedUrl = ChecksumSearch::createUrl(URL, ui.change->text(), mode);
    const QString text = i18n("%1 would become %2", URL.prettyUrl(), modifiedUrl.prettyUrl());
    ui.label->setText(text);
}

void ChecksumSearchAddDlg::slotAccpeted()
{
    emit addItem(ui.change->text(), ui.mode->currentIndex(), ui.type->currentText());
}

ChecksumDelegate::ChecksumDelegate(QObject *parent)
  : QStyledItemDelegate(parent),
    m_modesModel(0),
    m_typesModel(0)
{
}

ChecksumDelegate::ChecksumDelegate(QStringListModel *modesModel, QStringListModel *typesModel, QObject *parent)
  : QStyledItemDelegate(parent),
    m_modesModel(modesModel),
    m_typesModel(typesModel)
{
}

QWidget *ChecksumDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (index.isValid())
    {
        if (index.column() == 0l)
        {
            KLineEdit *line = new KLineEdit(parent);

            return line;
        }
        else if (index.column() == 1)
        {
            if (m_modesModel)
            {
                KComboBox *modesBox = new KComboBox(parent);
                modesBox->setModel(m_modesModel);

                return modesBox;
            }
        }
        else if (index.column() == 2)
        {
            if (m_typesModel)
            {
                KComboBox *typesBox = new KComboBox(parent);
                typesBox->setModel(m_typesModel);

                return typesBox;
            }
        }
    }

    return 0;
}

void ChecksumDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.isValid() && editor) {
        if (index.column() == 0) {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            line->setText(index.data(Qt::EditRole).toString());
        } else if (index.column() == 1) {
            KComboBox *modesBox = static_cast<KComboBox*>(editor);
            const QString mode = index.data(Qt::EditRole).toString();
            modesBox->setCurrentIndex(modesBox->findText(mode));
        } else if (index.column() == 2) {
            KComboBox *typesBox = static_cast<KComboBox*>(editor);
            const QString type = index.data(Qt::EditRole).toString();
            typesBox->setCurrentIndex(typesBox->findText(type));
        }
    }
}

void ChecksumDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.isValid() && editor && model)
    {
        if (index.column() == 0)
        {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            if (!line->text().isEmpty())
            {
                model->setData(index, line->text());
            }
        }
        else if (index.column() == 1)
        {
            KComboBox *modesBox = static_cast<KComboBox*>(editor);
            model->setData(index, modesBox->currentText());
            model->setData(index, modesBox->currentIndex(), Qt::UserRole);
        }
        else if (index.column() == 2)
        {
            KComboBox *typesBox = static_cast<KComboBox*>(editor);
            model->setData(index, typesBox->currentText());
        }
    }
}

void ChecksumDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

DlgChecksumSettingsWidget::DlgChecksumSettingsWidget(QWidget *parent, const QVariantList &args)
  : KCModule(KGetFactory::componentData(), parent, args)
{
    ui.setupUi(this);

    m_modes = ChecksumSearch::urlChangeModes();
    m_modesModel = new QStringListModel(m_modes, this);
    QStringList types = Verifier::supportedVerficationTypes();
    types.insert(0, QString());
    m_typesModel = new QStringListModel(types, this);

    m_model = new QStandardItemModel(0, 3, this);
    m_model->setHeaderData(0, Qt::Horizontal, i18nc("the string that is used to modify an url", "Change string"));
    m_model->setHeaderData(1, Qt::Horizontal, i18nc("the mode defines how the url should be changed", "Change mode"));
    m_model->setHeaderData(2, Qt::Horizontal, i18nc("the type of the checksum e.g. md5", "Checksum type"));

    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    ui.treeView->setModel(m_proxy);
    ChecksumDelegate *delegate = new ChecksumDelegate(m_modesModel, m_typesModel, this);
    ui.treeView->setItemDelegate(delegate);
    ui.treeView->sortByColumn(2, Qt::AscendingOrder);
    ui.add->setGuiItem(KStandardGuiItem::add());
    ui.remove->setGuiItem(KStandardGuiItem::remove());
    slotUpdate();

    connect(ui.add, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(ui.remove, SIGNAL(clicked()), this, SLOT(slotRemove()));
    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUpdate()));
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(changed()));
    connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(changed()));
    connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(changed()));
}

DlgChecksumSettingsWidget::~DlgChecksumSettingsWidget()
{
}

void DlgChecksumSettingsWidget::slotAdd()
{
    ChecksumSearchAddDlg *dialog = new ChecksumSearchAddDlg(m_modesModel, m_typesModel, this);
    connect(dialog, SIGNAL(addItem(QString,int,QString)), this, SLOT(slotAddItem(QString,int,QString)));

    dialog->show();
}

void DlgChecksumSettingsWidget::slotRemove()
{
    while (ui.treeView->selectionModel()->hasSelection()) {
        const QModelIndex index = ui.treeView->selectionModel()->selectedRows().first();
        m_model->removeRow(m_proxy->mapToSource(index).row());
    }
}

void DlgChecksumSettingsWidget::slotAddItem(const QString &change, int mode, const QString &type)
{
    QStandardItem *item = new QStandardItem(m_modes.value(mode));
    item->setData(QVariant(mode), Qt::UserRole);

    QList<QStandardItem*> items;
    items << new QStandardItem(change);
    items << item;
    items << new QStandardItem(type);
    m_model->insertRow(m_model->rowCount(), items);
}

void DlgChecksumSettingsWidget::slotUpdate()
{
    ui.remove->setEnabled(ui.treeView->selectionModel()->hasSelection());
}

void DlgChecksumSettingsWidget::load()
{
    QStringList changes = ChecksumSearchSettings::self()->searchStrings();
    QList<int> modes = ChecksumSearchSettings::self()->urlChangeModeList();
    QStringList types = ChecksumSearchSettings::self()->checksumTypeList();

    for(int i = 0; i < changes.size(); ++i)
    {
        slotAddItem(changes.at(i), modes.at(i), types.at(i));
    }
}

void DlgChecksumSettingsWidget::save()
{
    kDebug(5001);
    QStringList changes;
    QList<int> modes;
    QStringList types;

    for (int row = 0; row < m_model->rowCount(); ++row)
    {
        changes.append(m_model->data(m_model->index(row, 0)).toString());
        modes.append(m_model->data(m_model->index(row, 1), Qt::UserRole).toInt());
        types.append(m_model->data(m_model->index(row, 2)).toString());
    }

    ChecksumSearchSettings::self()->setSearchStrings(changes);
    ChecksumSearchSettings::self()->setUrlChangeModeList(modes);
    ChecksumSearchSettings::self()->setChecksumTypeList(types);

    ChecksumSearchSettings::self()->writeConfig();
}

#include "dlgchecksumsearch.moc"
