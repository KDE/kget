/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
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

#include "panelbar/kgetpanelbar.h"
#include "panelbar/kgetpanelbar_p.h"
#include "common/kgetappletutils.h"

#include <QGridLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QPainter>
#include <QProgressBar>

#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KLocale>
#include <KTitleWidget>
#include <KWindowSystem>
#include <kio/global.h>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/dialog.h>
#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/widgets/iconwidget.h>

const static int TOP_MARGIN = 60;
const static int SPACING = 4;
const static int MARGIN = 20;
const static int MAX_DOWNLOADS_PER_PAGE = 5;

KGetPanelBar::Private::Private(QWidget *parent) : Plasma::Dialog(parent),
    m_transfers()
{
    m_dialogLayout = new QGridLayout(this);
    m_dialogLayout->setColumnStretch(0, 0);
    m_dialogLayout->setColumnStretch(1, 0);
    m_dialogLayout->setColumnStretch(2, 1);
    setLayout(m_dialogLayout);

    KTitleWidget *title = new KTitleWidget(this);
    title->setText(i18n("KGet transfers"));
    title->setPixmap(KIcon("kget").pixmap(22, 22), KTitleWidget::ImageRight);
    m_dialogLayout->addWidget(title, 0, 0, 1, 3);
}

KGetPanelBar::Private::~Private()
{
}

void KGetPanelBar::Private::transfersAdded(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    foreach (OrgKdeKgetTransferInterface* transfer, transfers) {
        QLabel *icon = new QLabel(this);
        QLabel *name = new QLabel(this);
        QProgressBar *progressBar = new QProgressBar(this);

        QString fileName = KUrl(transfer->source().value()).fileName();
        icon->setPixmap(KIO::pixmapForUrl(fileName, 0, KIconLoader::Desktop, 16));
        name->setText(fileName);
        name->setStyleSheet(QString("color: %1;").
                            arg(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name()));
        progressBar->setValue(transfer->percent().value());
        
        int row = m_dialogLayout->rowCount();

        m_dialogLayout->addWidget(icon, row, 0);
        m_dialogLayout->addWidget(name, row, 1);
        m_dialogLayout->addWidget(progressBar, row, 2);

        m_transfers.insert(transfer, progressBar);
    }
}

void KGetPanelBar::Private::transfersRemoved(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    foreach (OrgKdeKgetTransferInterface* transfer, transfers) {
        int index = m_dialogLayout->indexOf(m_transfers[transfer]);
        int row, column, rowSpan, columnSpan;
        m_dialogLayout->getItemPosition(index, &row, &column, &rowSpan, &columnSpan);
        kDebug() << row;

        QList<QLayoutItem*> items;
        for (int i = 0; i != 3; i++) {
            QLayoutItem * item = m_dialogLayout->itemAtPosition(row, i);
            items << item;
            m_dialogLayout->removeItem(item);
            delete item->widget();
            delete item;
        }

        m_transfers.remove(transfer);
    }
}

void KGetPanelBar::Private::update()
{
    int totalSize = 0;
    int completedSize = 0;
    QMap<OrgKdeKgetTransferInterface*, QProgressBar*>::const_iterator it;
    QMap<OrgKdeKgetTransferInterface*, QProgressBar*>::const_iterator itEnd = m_transfers.constEnd();
    for (it = m_transfers.constBegin(); it != itEnd; ++it) {
        OrgKdeKgetTransferInterface *transfer = it.key();
        (*it)->setValue(transfer->percent().value());
        totalSize += transfer->totalSize().value();
        completedSize += transfer->downloadedSize().value();
    }
    if (totalSize > 0) {
        emit progressBarChanged((int) ((completedSize * 100) / totalSize));
    }
    else {
        emit progressBarChanged(0);
    }
}

void KGetPanelBar::Private::clear()
{
    // TODO : Find another way to remove all layout widgets except the title one
    for (int row = 1; row < m_dialogLayout->rowCount(); ++row) {
        delete m_dialogLayout->itemAtPosition(row, 0);
        delete m_dialogLayout->itemAtPosition(row, 1);
        delete m_dialogLayout->itemAtPosition(row, 2);

        m_dialogLayout->removeItem(m_dialogLayout->itemAtPosition(row, 0));
        m_dialogLayout->removeItem(m_dialogLayout->itemAtPosition(row, 1));
        m_dialogLayout->removeItem(m_dialogLayout->itemAtPosition(row, 2));
    }
    m_transfers.clear();
}


KGetPanelBar::KGetPanelBar(QObject *parent, const QVariantList &args)
        : KGetApplet(parent, args),
        m_icon(0),
        d(new KGetPanelBar::Private())
{
    d->setFocusPolicy(Qt::NoFocus);
    connect(this, SIGNAL(transfersAdded(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(transfersAdded(QList<OrgKdeKgetTransferInterface*>)));
    connect(this, SIGNAL(transfersRemoved(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(transfersRemoved(QList<OrgKdeKgetTransferInterface*>)));
    setBackgroundHints(Applet::NoBackground);
}

KGetPanelBar::~KGetPanelBar()
{
    delete d;
    delete m_bar;
}

void KGetPanelBar::init()
{
    KGlobal::locale()->insertCatalog("plasma_applet_kget");

    m_icon = new Plasma::IconWidget(KIcon("go-down"), QString(), this);

    m_layout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    m_layout->addItem(m_icon);
    
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(this);

    m_bar = new QProgressBar();
    
    connect(d, SIGNAL(progressBarChanged(int)), m_bar, SLOT(setValue(int)));
    
    proxy->setWidget(m_bar);

    m_bar->setValue(0);
    m_bar->setStyleSheet("background-color: transparent");

    m_layout->addItem(proxy);

    setLayout(m_layout);

    connect(m_icon, SIGNAL(clicked()), SLOT(showDialog()));

    KGetApplet::init();
}

void KGetPanelBar::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(p)
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)
}

void KGetPanelBar::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    if(data["error"].toBool()) {
        kDebug() << "Error : " << data["errorMessage"].toString();
        d->clear();
    }
    else if(!data["error"].toBool()) {
        setTransfers(data["transfers"].toMap());
        d->update();
    }
}

void KGetPanelBar::showDialog()
{
    if (d->isVisible()) {
        d->hide();
    }
    else {
        d->show();
        KWindowSystem::setState(d->winId(), NET::SkipTaskbar);
        d->move(popupPosition(d->sizeHint()));
    }
}

#include "kgetpanelbar.moc"
#include "kgetpanelbar_p.moc"
