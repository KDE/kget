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

#include "mirrorsettings.h"
#include "mirrormodel.h"
#include "core/transferhandler.h"

#include <QSortFilterProxyModel>

#include <KLocalizedString>
#include <KStandardGuiItem>

MirrorAddDlg::MirrorAddDlg(MirrorModel *model, QWidget *parent, Qt::WFlags flags)
  : QDialog(parent, flags),
    m_model(model),
    m_countryModel(nullptr)
{
    init();
}

MirrorAddDlg::MirrorAddDlg(MirrorModel *model, QSortFilterProxyModel *countryModel, QWidget *parent, Qt::WFlags flags)
  : QDialog(parent, flags),
    m_model(model),
    m_countryModel(countryModel)
{
    init();
}

QSize MirrorAddDlg::sizeHint() const
{
    QSize sh = QDialog::sizeHint();
    sh.setHeight(minimumSize().height());
    sh.setWidth(sh.width() * 1.5);
    return sh;
}

void MirrorAddDlg::init()
{
    setWindowTitle(i18n("Add mirror"));
    ui.setupUi(this);

    if (m_countryModel)
    {
        ui.location->setModel(m_countryModel);
        ui.location->setCurrentIndex(-1);
    }

    KGuiItem::assign(ui.buttonBox->button(QDialogButtonBox::Yes), KStandardGuiItem::add());

    updateButton();

    connect(ui.url, SIGNAL(textChanged(QString)), this, SLOT(updateButton(QString)));
    connect(this, &QDialog::accepted, this, &MirrorAddDlg::addMirror);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void MirrorAddDlg::showItem(MirrorItem::DataType type, bool show)
{
    switch (type)
    {
        case MirrorItem::Connections:
            ui.labelConnections->setVisible(show);
            ui.numConnections->setVisible(show);
            break;

        case MirrorItem::Priority:
            ui.labelPriority->setVisible(show);
            ui.priority->setVisible(show);
            break;

        case MirrorItem::Country:
            ui.labelLocation->setVisible(show);
            ui.location->setVisible(show);
            break;

        default:
            break;
    }
    update();
}

void MirrorAddDlg::updateButton(const QString &text)
{
    bool enabled = false;
    QUrl url(text);
    if (url.isValid() && !url.scheme().isEmpty() && !url.path().isEmpty())
    {
        enabled = true;
    }
    ui.buttonBox->button(QDialogButtonBox::Yes)->setEnabled(enabled);
}

void MirrorAddDlg::addMirror()
{
    const int numConnections = ui.numConnections->isVisible() ? ui.numConnections->value() : 0;
    const int priority = ui.priority->isVisible() ? ui.priority->value() : 0;
    const QString countryCode = ui.location->itemData(ui.location->currentIndex()).toString();
    m_model->addMirror(QUrl(ui.url->text()), numConnections, priority, countryCode);
    if (m_countryModel)
    {
        ui.location->setCurrentIndex(-1);
    }
}

MirrorSettings::MirrorSettings(QWidget *parent, TransferHandler *handler, const QUrl &file)
  : KGetSaveSizeDialog("MirrorSettings", parent),
    m_transfer(handler),
    m_file(file)
{
    m_model = new MirrorModel(this);
    m_model->setMirrors(m_transfer->availableMirrors(m_file));
    m_proxy = new MirrorProxyModel(this);
    m_proxy->setSourceModel(m_model);

    ui.setupUi(this);
    KGuiItem::assign(ui.add, KStandardGuiItem::add());
    KGuiItem::assign(ui.remove, KStandardGuiItem::remove());
    KGuiItem::assign(ui.closeButton, KStandardGuiItem::close());
    ui.treeView->setModel(m_proxy);
    ui.treeView->header()->setResizeMode(QHeaderView::ResizeToContents);
    ui.treeView->hideColumn(MirrorItem::Priority);
    ui.treeView->hideColumn(MirrorItem::Country);
    ui.treeView->setItemDelegate(new MirrorDelegate(this));

    updateButton();

    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButton()));
    connect(ui.add, &QPushButton::clicked, this, &MirrorSettings::addClicked);
    connect(ui.remove, &QPushButton::clicked, this, &MirrorSettings::removeMirror);
    connect(this, &MirrorSettings::finished, this, &MirrorSettings::save);

    setWindowTitle(i18n("Modify the used mirrors"));
    
    connect(ui.closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

QSize MirrorSettings::sizeHint() const
{
    return QSize(700, 400);
}

void MirrorSettings::updateButton()
{
    ui.remove->setEnabled(ui.treeView->selectionModel()->hasSelection());
}

void MirrorSettings::addClicked()
{
    MirrorAddDlg *dialog = new MirrorAddDlg(m_model, this);
    dialog->showItem(MirrorItem::Priority, false);
    dialog->showItem(MirrorItem::Country, false);
    dialog->show();
}

void MirrorSettings::removeMirror()
{
    while (ui.treeView->selectionModel()->hasSelection()) {
        const QModelIndex index = ui.treeView->selectionModel()->selectedRows().first();
        m_model->removeRow(m_proxy->mapToSource(index).row());
    }
}

void MirrorSettings::save()
{
    m_transfer->setAvailableMirrors(m_file, m_model->availableMirrors());
}


