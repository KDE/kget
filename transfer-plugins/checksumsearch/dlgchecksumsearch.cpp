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

#include "core/verifier.h"

#include "checksumsearch.h"
#include "checksumsearchsettings.h"
#include "kget_debug.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>

#include <KStandardGuiItem>
#include <KPluginFactory>


const QUrl ChecksumSearchAddDlg::URL = QUrl("http://www.example.com/file.zip");

K_PLUGIN_FACTORY( KGetFactory, registerPlugin<DlgChecksumSettingsWidget>(); )

ChecksumSearchAddDlg::ChecksumSearchAddDlg(QStringListModel *modesModel, QStringListModel *typesModel, QWidget *parent, Qt::WindowFlags flags)
  : QDialog(parent, flags),
    m_modesModel(modesModel),
    m_typesModel(typesModel)
{
    setWindowTitle(i18n("Add item"));
    
    ui.setupUi(this);

    if (m_modesModel)
    {
        ui.mode->setModel(m_modesModel);
    }
    if (m_typesModel)
    {
        ui.type->setModel(m_typesModel);
    }

    slotUpdate();

    connect(ui.change, &QLineEdit::textChanged, this, &ChecksumSearchAddDlg::slotUpdate);
    connect(ui.mode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdate()));
    connect(this, &QDialog::accepted, this, &ChecksumSearchAddDlg::slotAccpeted);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ChecksumSearchAddDlg::slotUpdate()
{
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!ui.change->text().isEmpty());

    const auto mode = static_cast<ChecksumSearch::UrlChangeMode>(ui.mode->currentIndex());
    const QUrl modifiedUrl = ChecksumSearch::createUrl(URL, ui.change->text(), mode);
    const QString text = i18n("%1 would become %2", URL.toDisplayString(), modifiedUrl.toDisplayString());
    ui.label->setText(text);
}

void ChecksumSearchAddDlg::slotAccpeted()
{
    Q_EMIT addItem(ui.change->text(), ui.mode->currentIndex(), ui.type->currentText());
}

ChecksumDelegate::ChecksumDelegate(QObject *parent)
  : QStyledItemDelegate(parent),
    m_modesModel(nullptr),
    m_typesModel(nullptr)
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
            auto *line = new KLineEdit(parent);

            return line;
        }
        else if (index.column() == 1)
        {
            if (m_modesModel)
            {
                auto *modesBox = new KComboBox(parent);
                modesBox->setModel(m_modesModel);

                return modesBox;
            }
        }
        else if (index.column() == 2)
        {
            if (m_typesModel)
            {
                auto *typesBox = new KComboBox(parent);
                typesBox->setModel(m_typesModel);

                return typesBox;
            }
        }
    }

    return nullptr;
}

void ChecksumDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.isValid() && editor) {
        if (index.column() == 0) {
            auto *line = static_cast<KLineEdit*>(editor);
            line->setText(index.data(Qt::EditRole).toString());
        } else if (index.column() == 1) {
            auto *modesBox = static_cast<KComboBox*>(editor);
            const QString mode = index.data(Qt::EditRole).toString();
            modesBox->setCurrentIndex(modesBox->findText(mode));
        } else if (index.column() == 2) {
            auto *typesBox = static_cast<KComboBox*>(editor);
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
            auto *line = static_cast<KLineEdit*>(editor);
            if (!line->text().isEmpty())
            {
                model->setData(index, line->text());
            }
        }
        else if (index.column() == 1)
        {
            auto *modesBox = static_cast<KComboBox*>(editor);
            model->setData(index, modesBox->currentText());
            model->setData(index, modesBox->currentIndex(), Qt::UserRole);
        }
        else if (index.column() == 2)
        {
            auto *typesBox = static_cast<KComboBox*>(editor);
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
  : KCModule(/*KGetFactory::componentDaa(), */parent, args)
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
    auto *delegate = new ChecksumDelegate(m_modesModel, m_typesModel, this);
    ui.treeView->setItemDelegate(delegate);
    ui.treeView->sortByColumn(2, Qt::AscendingOrder);
    KGuiItem::assign(ui.add, KStandardGuiItem::add());
    KGuiItem::assign(ui.remove, KStandardGuiItem::remove());
    slotUpdate();

    connect(ui.add, &QAbstractButton::clicked, this, &DlgChecksumSettingsWidget::slotAdd);
    connect(ui.remove, &QAbstractButton::clicked, this, &DlgChecksumSettingsWidget::slotRemove);
    connect(ui.treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DlgChecksumSettingsWidget::slotUpdate);
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(changed()));
    connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(changed()));
    connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(changed()));
}

DlgChecksumSettingsWidget::~DlgChecksumSettingsWidget()
{
}

void DlgChecksumSettingsWidget::slotAdd()
{
    auto *dialog = new ChecksumSearchAddDlg(m_modesModel, m_typesModel, this);
    connect(dialog, &ChecksumSearchAddDlg::addItem, this, &DlgChecksumSettingsWidget::slotAddItem);

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
    auto *item = new QStandardItem(m_modes.value(mode));
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
    qCDebug(KGET_DEBUG);
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

    ChecksumSearchSettings::self()->save();
}

#include "dlgchecksumsearch.moc"

