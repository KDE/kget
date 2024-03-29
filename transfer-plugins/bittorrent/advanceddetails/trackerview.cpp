/***************************************************************************
 *   Copyright (C) 2006-2007 by Joris Guisson, Ivan Vasic                  *
 *   joris.guisson@gmail.com                                               *
 *	 ivasic@gmail.com                                                  *
 *									   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "trackerview.h"

#include <QClipboard>
#include <QHeaderView>
#include <QInputDialog>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include "trackermodel.h"
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerinterface.h>
#include <interfaces/trackerslist.h>
#include <torrent/globals.h>
#include <util/log.h>

using namespace bt;

namespace kt
{

TrackerView::TrackerView(QWidget *parent)
    : QWidget(parent)
    , tc(nullptr)
{
    setupUi(this);
    model = new TrackerModel(this);
    proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSortRole(Qt::UserRole);
    proxy_model->setSourceModel(model);
    m_tracker_list->setModel(proxy_model);
    m_tracker_list->setAllColumnsShowFocus(true);
    m_tracker_list->setRootIsDecorated(false);
    m_tracker_list->setAlternatingRowColors(true);
    m_tracker_list->setSortingEnabled(true);
    connect(m_add_tracker, &QAbstractButton::clicked, this, &TrackerView::addClicked);
    connect(m_remove_tracker, &QAbstractButton::clicked, this, &TrackerView::removeClicked);
    connect(m_change_tracker, &QAbstractButton::clicked, this, &TrackerView::changeClicked);
    connect(m_restore_defaults, &QAbstractButton::clicked, this, &TrackerView::restoreClicked);
    connect(m_tracker_list->selectionModel(), &QItemSelectionModel::currentChanged, this, &TrackerView::currentChanged);
    connect(m_scrape, &QAbstractButton::clicked, this, &TrackerView::scrapeClicked);

    m_add_tracker->setIcon(QIcon::fromTheme("list-add"));
    m_remove_tracker->setIcon(QIcon::fromTheme("list-remove"));
    m_restore_defaults->setIcon(QIcon::fromTheme("kt-restore-defaults"));
    m_change_tracker->setIcon(QIcon::fromTheme("kt-change-tracker"));

    setEnabled(false);
    torrentChanged(nullptr);
}

TrackerView::~TrackerView()
{
}

void TrackerView::addClicked()
{
    if (!tc)
        return;

    bool ok = false;
    QClipboard *clipboard = QApplication::clipboard();
    QString text = QInputDialog::getText(this, i18n("Add Tracker"), i18n("Enter the URL of the tracker:"), QLineEdit::Normal, clipboard->text(), &ok);

    if (!ok)
        return;

    QUrl url(text);
    if (!url.isValid()) {
        KMessageBox::error(nullptr, i18n("Malformed URL."));
        return;
    }

    // check for dupes
    if (!tc->getTrackersList()->addTracker(url, true)) {
        KMessageBox::error(nullptr, i18n("There already is a tracker named <b>%1</b>.", text));
    } else {
        model->insertRow(model->rowCount(QModelIndex()));
    }
}

void TrackerView::removeClicked()
{
    QModelIndex current = proxy_model->mapToSource(m_tracker_list->selectionModel()->currentIndex());
    if (!current.isValid())
        return;

    model->removeRow(current.row());
}

void TrackerView::changeClicked()
{
    QModelIndex current = m_tracker_list->selectionModel()->currentIndex();
    if (!current.isValid())
        return;

    bt::TrackersList *tlist = tc->getTrackersList();
    bt::TrackerInterface *trk = model->tracker(proxy_model->mapToSource(current));
    if (trk && trk->isEnabled())
        tlist->setCurrentTracker(trk);
}

void TrackerView::restoreClicked()
{
    tc->getTrackersList()->restoreDefault();
    tc->updateTracker();
    model->changeTC(tc); // trigger reset
}

void TrackerView::updateClicked()
{
    if (!tc)
        return;

    tc->updateTracker();
}

void TrackerView::scrapeClicked()
{
    if (!tc)
        return;

    tc->scrapeTracker();
}

void TrackerView::changeTC(TorrentInterface *ti)
{
    if (tc == ti)
        return;

    setEnabled(ti != nullptr);
    torrentChanged(ti);
    update();
}

void TrackerView::update()
{
    if (tc)
        model->update();
}

void TrackerView::torrentChanged(TorrentInterface *ti)
{
    tc = ti;
    if (!tc) {
        m_add_tracker->setEnabled(false);
        m_remove_tracker->setEnabled(false);
        m_restore_defaults->setEnabled(false);
        m_change_tracker->setEnabled(false);
        m_scrape->setEnabled(false);
        model->changeTC(nullptr);
    } else {
        m_add_tracker->setEnabled(true);
        m_remove_tracker->setEnabled(true);
        m_restore_defaults->setEnabled(true);
        m_scrape->setEnabled(true);
        model->changeTC(tc);
        currentChanged(m_tracker_list->selectionModel()->currentIndex(), QModelIndex());
    }
}

void TrackerView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (!tc) {
        m_change_tracker->setEnabled(false);
        m_remove_tracker->setEnabled(false);
        return;
    }

    const TorrentStats &s = tc->getStats();

    bt::TrackerInterface *trk = model->tracker(proxy_model->mapToSource(current));
    bool enabled = trk ? trk->isEnabled() : false;
    m_change_tracker->setEnabled(s.running && model->rowCount(QModelIndex()) > 1 && enabled);
    m_remove_tracker->setEnabled(trk && tc->getTrackersList()->canRemoveTracker(trk));
}

void TrackerView::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("TrackerView");
    QByteArray s = m_tracker_list->header()->saveState();
    g.writeEntry("state", s.toBase64());
}

void TrackerView::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("TrackerView");
    QByteArray s = QByteArray::fromBase64(g.readEntry("state", QByteArray()));
    if (!s.isNull()) {
        QHeaderView *v = m_tracker_list->header();
        v->restoreState(s);
    }
}
}

#include "moc_trackerview.cpp"
