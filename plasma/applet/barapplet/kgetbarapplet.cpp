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

#include "barapplet/kgetbarapplet.h"
#include "barapplet/kgetbarapplet_p.h"
#include "common/kgetappletutils.h"

#include <QVBoxLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsLayoutItem>
#include <QGraphicsWidget>
#include <QPainter>
#include <QHBoxLayout>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

#include <KDebug>
#include <KIcon>

#include <plasma/svg.h>
#include <plasma/applet.h>
#include <plasma/theme.h>
#include <plasma/dataengine.h>

#define TOP_MARGIN 60
#define SPACING 4
#define MARGIN 20
#define MAX_DOWNLOADS_PER_PAGE 5

KGetBarApplet::Private::Private(QGraphicsWidget *parent) : QGraphicsProxyWidget(parent),
    m_actualPage(0)
{
    // Pager layout and next, previous buttons
    QHBoxLayout *pager_layout = new QHBoxLayout();

    m_pageLabel = new QLabel();

    m_previousPageButton = new QPushButton(KIcon("go-previous"), "");
    m_nextPageButton = new QPushButton(KIcon("go-next"), "");
    m_previousPageButton->setEnabled(false);
    m_nextPageButton->setEnabled(false);

    pager_layout->addWidget(m_previousPageButton);
    pager_layout->addWidget(m_nextPageButton);

    // Total size
    m_totalSizeLabel = new QLabel();
    m_totalSizeLabel->setAlignment(Qt::AlignRight);

    m_barsLayout = new QVBoxLayout();

    m_verticalLayout = new QVBoxLayout();

    m_verticalLayout->addLayout(m_barsLayout);
    m_verticalLayout->addLayout(pager_layout);
    m_verticalLayout->addWidget(m_totalSizeLabel);
    m_verticalLayout->addWidget(m_pageLabel);

    // main widget
    QWidget *widget = new QWidget();
    widget->setStyleSheet("background-color: transparent; color: white");
    widget->setLayout(m_verticalLayout);

    setWidget(widget);

    // connect the clicked signal of the next and previous buttons
    QObject::connect(m_previousPageButton, SIGNAL(clicked()), SLOT(previousPage()));
    QObject::connect(m_nextPageButton, SIGNAL(clicked()), SLOT(nextPage()));
}

KGetBarApplet::Private::~Private()
{
}

void KGetBarApplet::Private::setTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    /*QStringList urls;
    foreach (OrgKdeKgetTransferInterface* transfer, transfers)
        urls << transfer->source().value();
    QStringList oldUrls;
    foreach (OrgKdeKgetTransferInterface* transfer, m_progressBars.keys())
        oldUrls << transfer->source().value();
    if (urls != oldUrls) { //FIXME
        clear();
    }*/

    populate(transfers);
}

void KGetBarApplet::Private::nextPage()
{
    if (m_progressBars.size() >= ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE)) {
        m_actualPage ++;

        clear();
        populate(m_progressBars.keys());
    }
}

void KGetBarApplet::Private::previousPage()
{
    if (m_actualPage > 0) {
        m_actualPage --;

        clear();
        populate(m_progressBars.keys());
    }
}

void KGetBarApplet::Private::populate(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    int totalSize = 0;
    int limit = transfers.size() < ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE) ? 
            transfers.size() :
            ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE);

    for(int i = (m_actualPage * MAX_DOWNLOADS_PER_PAGE); i < limit; i++) {
        OrgKdeKgetTransferInterface* transfer = transfers.at(i);
        QString fileName = KUrl(transfer->source().value()).fileName();
        kDebug() << fileName;

        if (!m_progressBars.keys().contains(transfer)) {
            QProgressBar *bar = new QProgressBar(0);
            bar->setFormat(fileName + " %v%");

            m_progressBars[transfer] = bar;
            m_barsLayout->insertWidget(0, bar);
        }

        m_progressBars[transfer]->setValue(transfer->percent().value());
        totalSize += transfer->totalSize().value();
    }

    m_pageLabel->setText(i18n("Showing %1-%2 of %3 transfers",
        m_actualPage * MAX_DOWNLOADS_PER_PAGE, limit, transfers.size()));

    // remove the progressbars for the deleted transfers
    QMap<OrgKdeKgetTransferInterface*, QProgressBar*>::iterator it;
    QMap<OrgKdeKgetTransferInterface*, QProgressBar*>::iterator itEnd = m_progressBars.end();
    for (it = m_progressBars.begin(); it != itEnd; ) {
        if (transfers.contains(it.key())) {
            ++it;
        } else {
            QProgressBar *bar = *it;
            it = m_progressBars.erase(it);
            m_barsLayout->removeWidget(bar);
            delete bar;
        }
    }

    // activate or deactivate the navigation buttons
    if(transfers.size() > ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE))
        m_nextPageButton->setEnabled(true);
    else
        m_nextPageButton->setEnabled(false);

    if(m_actualPage > 0)
        m_previousPageButton->setEnabled(true);
    else
        m_previousPageButton->setEnabled(false);
}

void KGetBarApplet::Private::clear()
{
    QMap<OrgKdeKgetTransferInterface*, QProgressBar*>::iterator it;
    QMap<OrgKdeKgetTransferInterface*, QProgressBar*>::iterator itEnd = m_progressBars.end();
    for (it = m_progressBars.begin(); it != itEnd; ) {
        QProgressBar *bar = *it;
        it = m_progressBars.erase(it);
        m_barsLayout->removeWidget(bar);
        delete bar;
    }
}



KGetBarApplet::KGetBarApplet(QObject *parent, const QVariantList &args) 
        : KGetApplet(parent, args),
        m_errorWidget(0),
        d(new KGetBarApplet::Private(0))
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(Applet::DefaultBackground);

    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/kget");
}

KGetBarApplet::~KGetBarApplet()
{
    delete d;
    delete m_errorWidget;
}

void KGetBarApplet::init()
{
    KGlobal::locale()->insertCatalog("plasma_applet_kget");
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setSpacing(SPACING);
    m_layout->setContentsMargins(MARGIN, TOP_MARGIN, MARGIN, MARGIN);
    m_layout->setOrientation(Qt::Vertical);

    m_layout->addItem(d);

    setLayout(m_layout);

    resize(QSize(300, 360));

    KGetApplet::init();
}

void KGetBarApplet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);
    if(formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
        KGetAppletUtils::paintTitle(p, m_theme, contentsRect);
    }
}

void KGetBarApplet::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)
    kDebug();

    if(data["error"].toBool()) {
        if (!m_errorWidget) {
            m_layout->removeAt(0);
            delete d;
            d = 0;

            m_errorWidget = KGetAppletUtils::createErrorWidget(data["errorMessage"].toString(), this);
            m_layout->addItem(m_errorWidget);
        }
    } else if(!data["error"].toBool()) {
        if (!d) {
            m_layout->removeAt(0);
            delete m_errorWidget;
            m_errorWidget = 0;

            d = new KGetBarApplet::Private(this);

            m_layout->addItem(d);
        }

        setTransfers(data["transfers"].toMap());
        d->setTransfers(m_transfers);
    }
}

#include "kgetbarapplet.moc"
#include "kgetbarapplet_p.moc"
