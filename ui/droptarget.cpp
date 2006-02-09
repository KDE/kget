/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <QBitmap>
#include <QClipboard>
#include <QTimer>

#include <kapplication.h>
#include <kiconloader.h>
#include <kwin.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <math.h>

#include "core/model.h"
#include "conf/settings.h"
#include "ui/droptarget.h"
#include "kget.h"

#define TARGET_WIDTH   80
#define TARGET_HEIGHT  80
#define TARGET_ANI_MS  20

DropTarget::DropTarget(KGet * mw)
    : QWidget(0, Qt::WType_TopLevel | Qt::WStyle_StaysOnTop |
    Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool),
    parentWidget((QWidget *)mw), animTimer(0)
{
    QRect desk = KGlobalSettings::desktopGeometry(this);
    desk.setRight( desk.right() - TARGET_WIDTH );
    desk.setBottom( desk.bottom() - TARGET_HEIGHT );

    if ( desk.contains(Settings::dropPosition()) )
        move(Settings::dropPosition());
    else
        move(desk.x()+200, desk.y()+200);
    resize(TARGET_WIDTH, TARGET_HEIGHT);

    unsigned long state = NET::SkipTaskbar | NET::StaysOnTop;
    KWin::setState(winId(), Settings::dropSticky() ? (state | NET::Sticky) : state );

    // set background pixmap
    QPixmap bgnd = UserIcon( "target" );
    if (!bgnd.mask())
        kError(5001) << "Drop target pixmap has no mask!\n";
    else
        bgnd.setMask(bgnd.mask());

    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(bgnd));
    setPalette(palette);

    // popup menu for right mouse button
    popupMenu = new KMenu();
    popupMenu->addTitle(mw->windowTitle());

    KToggleAction * downloadAction = (KToggleAction *)mw->actionCollection()->action("download");
    downloadAction->plug(popupMenu);
    connect( downloadAction, SIGNAL( toggled(bool) ), this, SLOT( slotStartStopToggled(bool) ) );
    popupMenu->addSeparator();
    pop_show = popupMenu->insertItem("", this, SLOT(toggleMinimizeRestore()));
    popupMenu->insertItem(i18n("Hide me"), this, SLOT(slotClose()));
    pop_sticky = popupMenu->insertItem(i18n("Sticky"), this, SLOT(toggleSticky()));
    popupMenu->setItemChecked(pop_sticky, Settings::dropSticky());
    popupMenu->addSeparator();
    mw->actionCollection()->action("preferences")->plug(popupMenu);
    mw->actionCollection()->action("quit")->plug(popupMenu);

    isdragging = false;

    // Enable dropping
    setAcceptDrops(true);
}


DropTarget::~DropTarget()
{
    Settings::setDropPosition( pos() );
    Settings::setShowDropTarget( !isHidden() );
//    unsigned long state = KWin::windowInfo(kdrop->winId()).state();
//    // state will be 0L if droptarget is hidden. Sigh.
//    config->writeEntry("State", state ? state : DEFAULT_DOCK_STATE ); 
    delete popupMenu;
    delete animTimer;
}


void DropTarget::slotClose()
{
    setVisible( false );
    if (!Settings::expertMode())
    {
        KMessageBox::information(parentWidget,
            i18n("Drop target has been hidden. If you want to show it "
                 "again, go to Settings->Configure KGet->Look & Feel."),
            i18n("Hiding drop target"),
            "CloseDroptarget");
    }
}


void DropTarget::slotStartStopToggled( bool started )
{
    if ( started && Settings::animateDropTarget() )
        playAnimationSync();
}


/** widget events */

void DropTarget::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        isdragging = true;
        dx = e->globalPos().x() - pos().x();
        dy = e->globalPos().y() - pos().y();
    }
    else if (e->button() == Qt::RightButton)
    {
        popupMenu->changeItem(pop_show, parentWidget->isHidden() ?
                              i18n("Show main window") :
                              i18n("Hide main window") );
        popupMenu->popup(e->globalPos());
    }
    else if (e->button() == Qt::MidButton)
    {
        //Here we paste the transfer
        QString newtransfer = QApplication::clipboard()->text();
        newtransfer = newtransfer.trimmed();

        if(!newtransfer.isEmpty())
            Model::addTransfer(KUrl::fromPathOrURL(newtransfer),"");
    }
}


void DropTarget::dragEnterEvent(QDragEnterEvent * event)
{
    event->setAccepted(KUrl::List::canDecode(event->mimeData())
                  || event->mimeData()->hasText());
}


void DropTarget::dropEvent(QDropEvent * event)
{
    KUrl::List list = KUrl::List::fromMimeData(event->mimeData());
    QString str;

    if (!list.isEmpty())
    {
        KUrl::List::Iterator it = list.begin();
        KUrl::List::Iterator itEnd = list.end();

        for( ; it!=itEnd ; ++it )
            Model::addTransfer(*it);
    }
    else
    {
        str = event->mimeData()->text();
        Model::addTransfer(KUrl::fromPathOrURL(str));
    }

    if ( Settings::animateDropTarget() )
        playAnimationSync();
}


void DropTarget::closeEvent( QCloseEvent * e )
{
    if( kapp->sessionSaving() )
        e->ignore();
    else
    {
        setVisible( false );
        e->accept();
    }
}


void DropTarget::toggleSticky()
{
    Settings::setDropSticky( !Settings::dropSticky() );
    popupMenu->setItemChecked(pop_sticky, Settings::dropSticky());
    updateStickyState();
}

void DropTarget::updateStickyState()
{
    if ( Settings::dropSticky() )
        KWin::setState(winId(), NET::SkipTaskbar | NET::StaysOnTop | NET::Sticky);
    else
        KWin::clearState(winId(), NET::Sticky);
}

void DropTarget::toggleMinimizeRestore()
{
    bool nextState = parentWidget->isHidden();
    Settings::setShowMain( nextState );
    parentWidget->setVisible( nextState );
}

void DropTarget::mouseMoveEvent(QMouseEvent * e)
{
    Q_UNUSED(e);
    if ( isdragging && !Settings::dropSticky() )
    {
        move( QCursor::pos().x() - dx, QCursor::pos().y() - dy );
        e->accept();
    }
}

void DropTarget::mouseReleaseEvent(QMouseEvent *)
{
    isdragging = false;
}

void DropTarget::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
        toggleMinimizeRestore();
}

void DropTarget::setVisible( bool shown, bool internal )
{
    if (shown == !isHidden())
        return;

    if ( internal )
        Settings::setShowDropTarget( shown );

    if (!shown)
    {
        Settings::setDropPosition( pos() );
        if ( Settings::animateDropTarget() )
            playAnimationHide();
        else
            hide();
    }
    else
    {
        show();
        if ( Settings::animateDropTarget() )
            playAnimation();
    }
}

/** widget animations */

void DropTarget::playAnimation()
{
    if ( animTimer )
        delete animTimer;
    animTimer = new QTimer;
    connect( animTimer, SIGNAL( timeout() ),
        this, SLOT( slotAnimate() ));
    move( Settings::dropPosition().x(), -TARGET_HEIGHT );
    ani_y = -1;
    ani_vy = 0;
    animTimer->start(TARGET_ANI_MS);
}

void DropTarget::playAnimationHide()
{
    if ( animTimer )
    {
        if ( animTimer->isActive() )
            move( x(), (int)(ani_y) );
        delete animTimer;
    }
    animTimer = new QTimer;
    connect( animTimer, SIGNAL( timeout() ),
        this, SLOT( slotAnimateHide() ));
    ani_y = (float)y();
    ani_vy = 0;
    animTimer->start(TARGET_ANI_MS);
}

void DropTarget::playAnimationSync()
{
    if ( animTimer )
    {
        if ( animTimer->isActive() )
            move( x(), (int)(ani_y) );
        delete animTimer;
    }
    animTimer = new QTimer;
    connect( animTimer, SIGNAL( timeout() ),
        this, SLOT( slotAnimateSync() ));
    ani_y = (float)y();
    ani_vy = -1;
    animTimer->start(TARGET_ANI_MS);
}

void DropTarget::slotAnimate()
{
//     QWidget *d = QApplication::desktop()->screen();
    static float dT = TARGET_ANI_MS / 1000.0;

    ani_vy -= ani_y * 30 * dT;
    ani_vy *= 0.95;
    ani_y += ani_vy * dT;

//     move( x(), (int)(d->height()/3 * (1 + ani_y)) );
    move( x(), (int)(Settings::dropPosition().y() * (1 + ani_y)) );

    if ( fabsf(ani_y) < 0.01 && fabsf(ani_vy) < 0.01 && animTimer )
    {
        animTimer->stop();
        delete animTimer;
        animTimer = 0;
    }
}

void DropTarget::slotAnimateHide()
{
    static float dT = TARGET_ANI_MS / 1000.0;

    ani_vy += -2000 * dT;
    float new_y = y() + ani_vy * dT;

    if ( new_y < -height() )
    {
        animTimer->stop();
        delete animTimer;
        animTimer = 0;
        move( x(), (int)(ani_y) );
        hide();
    } else
        move( x(), (int)(new_y) );
}

void DropTarget::slotAnimateSync()
{
    static float dT = TARGET_ANI_MS / 1000.0;

    ani_vy += 4 * dT;               // from -1 to 1 in 0.5 seconds
    float i = 2 * M_PI * ani_vy;    // from -2PI to 2PI
    float j = (i == 0.0) ? 1 : (sin( i ) / i) * (1 + fabs(ani_vy));

    if ( ani_vy >= 1 )
    {
        animTimer->stop();
        delete animTimer;
        animTimer = 0;
        move( x(), (int)(ani_y) );
    } else
        move( x(), (int)(ani_y + 6*j) );
}

#include "droptarget.moc"
