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

#include "urlwidget.h"
#include "../mirror/mirrormodel.h"
#include "../mirror/mirrorsettings.h"
#include "metalinker.h"
#include <KGuiItem>
#include <KStandardGuiItem>
#include <QSortFilterProxyModel>

UrlWidget::UrlWidget(QObject *parent)
    : QObject(parent)
    , m_resources(nullptr)
    , m_countrySort(nullptr)
    , m_widget(nullptr)
{
    m_widget = new QWidget; // TODO inherit from qWidget and set this widget as mainwidget?
    ui.setupUi(m_widget);

    m_mirrorModel = new MirrorModel(this);
    m_proxy = new MirrorProxyModel(this);
    m_proxy->setSourceModel(m_mirrorModel);
    ui.used_mirrors->setModel(m_proxy);
    ui.used_mirrors->sortByColumn(MirrorItem::Priority, Qt::DescendingOrder);
    ui.used_mirrors->resizeColumnToContents(MirrorItem::Priority);
    ui.used_mirrors->hideColumn(MirrorItem::Used);
    ui.used_mirrors->hideColumn(MirrorItem::Connections);

    KGuiItem::assign(ui.add_mirror, KStandardGuiItem::add());
    KGuiItem::assign(ui.remove_mirror, KStandardGuiItem::remove());
    ui.remove_mirror->setEnabled(false);
    connect(ui.used_mirrors->selectionModel(), &QItemSelectionModel::selectionChanged, this, &UrlWidget::slotUrlClicked);
    connect(ui.add_mirror, &QPushButton::clicked, this, &UrlWidget::slotAddMirror);
    connect(ui.remove_mirror, &QPushButton::clicked, this, &UrlWidget::slotRemoveMirror);
    connect(m_mirrorModel, &MirrorModel::dataChanged, this, &UrlWidget::urlsChanged);
    connect(m_mirrorModel, &MirrorModel::rowsInserted, this, &UrlWidget::urlsChanged);
    connect(m_mirrorModel, &MirrorModel::rowsRemoved, this, &UrlWidget::urlsChanged);
}

UrlWidget::~UrlWidget()
{
    delete m_widget;
}

QWidget *UrlWidget::widget()
{
    return m_widget;
}

void UrlWidget::init(KGetMetalink::Resources *resources, QSortFilterProxyModel *countrySort)
{
    if (m_resources) {
        m_mirrorModel->removeRows(0, m_mirrorModel->rowCount());
    }

    m_resources = resources;
    m_countrySort = countrySort;

    foreach (const KGetMetalink::Url &url, m_resources->urls) {
        m_mirrorModel->addMirror(url.url, 0, url.priority, url.location);
    }

    auto *delegate = new MirrorDelegate(m_countrySort, this);
    ui.used_mirrors->setItemDelegate(delegate);
}

bool UrlWidget::hasUrls() const
{
    return m_mirrorModel->rowCount();
}

void UrlWidget::slotUrlClicked()
{
    const QModelIndexList selected = ui.used_mirrors->selectionModel()->selectedRows();
    ui.remove_mirror->setEnabled(!selected.isEmpty());
}

void UrlWidget::slotAddMirror()
{
    auto *dialog = new MirrorAddDlg(m_mirrorModel, m_countrySort, m_widget);
    dialog->showItem(MirrorItem::Connections, false);
    dialog->show();
}

void UrlWidget::slotRemoveMirror()
{
    while (ui.used_mirrors->selectionModel()->hasSelection()) {
        const QModelIndex index = ui.used_mirrors->selectionModel()->selectedRows().first();
        m_mirrorModel->removeRow(m_proxy->mapToSource(index).row());
    }
}

void UrlWidget::save()
{
    if (m_resources) {
        for (int i = 0; i < m_mirrorModel->rowCount(); ++i) {
            KGetMetalink::Url url;
            url.url = QUrl(m_mirrorModel->index(i, MirrorItem::Url).data(Qt::UserRole).toUrl());
            url.priority = m_mirrorModel->index(i, MirrorItem::Priority).data(Qt::UserRole).toInt();
            url.location = m_mirrorModel->index(i, MirrorItem::Country).data(Qt::UserRole).toString();
            m_resources->urls.append(url);
        }
    }
}

#include "moc_urlwidget.cpp"
