/***************************************************************************
*                                droptarget.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/

#include <kapplication.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kiconeffect.h>
#include <kmessagebox.h>
#include <stdlib.h>
#include <math.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qapplication.h>

#include "conf/settings.h"
#include "kget.h"
#include "droptarget.h"

#define TARGET_WIDTH   68
#define TARGET_HEIGHT  67
#define TARGET_OFFSETX -11
#define TARGET_OFFSETY -6
#define TARGET_ANI_MS  30


DropTarget::DropTarget(KMainWidget * mw)
    : QWidget(0, "drop", WType_TopLevel | WStyle_StaysOnTop |
    WStyle_Customize | WStyle_NoBorder | WStyle_Tool), ViewInterface(),
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

    // setup mask
    QBitmap mask(TARGET_WIDTH, TARGET_HEIGHT);
    mask.fill(color0);
    QPainter p2;
    p2.begin(&mask);
    p2.setBrush(color1);
    p2.drawChord(0, 0, TARGET_WIDTH, TARGET_HEIGHT, 5760, 5760);
    p2.end();
    setMask( mask );

    // setup pixmaps
    QPixmap bgnd = QPixmap(TARGET_WIDTH, TARGET_HEIGHT);
    bgnd.fill( Qt::white );
    QPixmap tmp = UserIcon( "target" );
    bitBlt(&bgnd, TARGET_OFFSETX, TARGET_OFFSETY, &tmp );

    /* The following code was adapted from kdebase/kicker/ui/k_mnu.cpp
     * It paints a tint over the kget arrow taking the tint color from
     * active or inactive window title colors
     */
    KConfig *config = KGlobal::config();
    QColor color = palette().active().highlight();

    config->setGroup("WM");
    QColor activeTitle = config->readColorEntry("activeBackground", &color);
    QColor inactiveTitle = config->readColorEntry("inactiveBackground", &color);

    // figure out which color is most suitable for recoloring to
    int h1, s1, v1, h2, s2, v2, h3, s3, v3;
    activeTitle.hsv(&h1, &s1, &v1);
    inactiveTitle.hsv(&h2, &s2, &v2);
    palette().active().background().hsv(&h3, &s3, &v3);

    if ( (abs(h1-h3)+abs(s1-s3)+abs(v1-v3) < abs(h2-h3)+abs(s2-s3)+abs(v2-v3)) &&
         ((abs(h1-h3)+abs(s1-s3)+abs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
        color = inactiveTitle;
    else
        color = activeTitle;

    // limit max/min brightness
    int h, s, v;
    color.getHsv( &h, &s, &v );
    if (v > 180)
        color.setHsv( h, s, 180 );
    else if (v < 76 )
        color.setHsv( h, s, 76 );

    QImage image = bgnd.convertToImage();
    KIconEffect::colorize( image, color, 1.0 );
    bgnd.convertFromImage( image );
    setErasePixmap( bgnd );

    // popup menu for right mouse button
    popupMenu = new KPopupMenu();
    popupMenu->insertTitle(mw->caption());
    popupMenu->setCheckable(true);

    KToggleAction * downloadAction = (KToggleAction *)mw->actionCollection()->action("download");
    downloadAction->plug(popupMenu);
    connect( downloadAction, SIGNAL( toggled(bool) ), this, SLOT( slotStartStopToggled(bool) ) );
    popupMenu->insertSeparator();
    pop_show = popupMenu->insertItem("", this, SLOT(toggleMinimizeRestore()));
    popupMenu->insertItem(i18n("Hide me"), this, SLOT(slotClose()));
    pop_sticky = popupMenu->insertItem(i18n("Sticky"), this, SLOT(toggleSticky()));
    popupMenu->setItemChecked(pop_sticky, Settings::dropSticky());
    popupMenu->insertSeparator();
    mw->actionCollection()->action("preferences")->plug(popupMenu);
    mw->actionCollection()->action("quit")->plug(popupMenu);

    isdragging = false;

    // Enable dropping
    setAcceptDrops(true);
}


DropTarget::~DropTarget()
{
    Settings::setDropPosition( pos() );
    Settings::setShowDropTarget( isShown() );
//    unsigned long state = KWin::windowInfo(kdrop->winId()).state();
//    // state will be 0L if droptarget is hidden. Sigh.
//    config->writeEntry("State", state ? state : DEFAULT_DOCK_STATE ); 
    delete popupMenu;
    delete animTimer;
}


void DropTarget::slotClose()
{
    setShown( false );
    KMessageBox::information(parentWidget,
        i18n("Drop target has been hidden. If you want to show it "
             "again, go to Settings->Configure KGet->Look & Feel."),
        i18n("Hiding drop target"),
        "CloseDroptarget");
}


void DropTarget::slotStartStopToggled( bool started )
{
    if ( started && Settings::animateDropTarget() )
        playAnimationSync();
}


/** widget events */

void DropTarget::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == LeftButton)
    {
        // toggleMinimizeRestore ();
//        oldX = 0;
//        oldY = 0;
        isdragging = true;
        dx = QCursor::pos().x() - pos().x();
        dy = QCursor::pos().y() - pos().y();
    }
    else if (e->button() == RightButton)
    {
        popupMenu->changeItem(pop_show, parentWidget->isHidden() ?
                              i18n("Show main window") :
                              i18n("Hide main window") );
        popupMenu->popup(QCursor::pos());
    }
    else if (e->button() == MidButton)
        schedRequestOperation( OpPasteTransfer );
}


void DropTarget::dragEnterEvent(QDragEnterEvent * event)
{
    event->accept(KURLDrag::canDecode(event)
                  || QTextDrag::canDecode(event));
}


void DropTarget::dropEvent(QDropEvent * event)
{
    KURL::List list;
    QString str;

    if (KURLDrag::decode(event, list))
	schedNewURLs( list, QString::null );
    else if (QTextDrag::decode(event, str))
	schedNewURLs( KURL::fromPathOrURL(str), QString::null );
    else return;
    if ( Settings::animateDropTarget() )
        playAnimationSync();
}


void DropTarget::closeEvent( QCloseEvent * e )
{
    if( kapp->sessionSaving() )
        e->ignore();
    else
        setShown( false );
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
    parentWidget->setShown( nextState );
}

void DropTarget::mouseMoveEvent(QMouseEvent * e)
{
/*
    if (oldX == 0)
    {
        oldX = e->x();
        oldY = e->y();
        return;
    }
*/
    if (isdragging)
        move( QCursor::pos().x() - dx, QCursor::pos().y() - dy );

//    QWidget::move(x() + (e->x() - oldX), y() + (e->y() - oldY));
//    move(x() + (e->x() - oldX), y() + (e->y() - oldY));  // <<--
}

void DropTarget::mouseReleaseEvent(QMouseEvent *)
{
    isdragging = false;
}

void DropTarget::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (e->button() == LeftButton)
        toggleMinimizeRestore();
}

void DropTarget::setShown( bool shown, bool internal )
{
    if (shown == isShown())
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
    QWidget *d = QApplication::desktop();
    static float dT = TARGET_ANI_MS / 1000.0;
    
    ani_vy -= ani_y * 30 * dT;
    ani_vy *= 0.95;
    ani_y += ani_vy * dT;
    
    move( x(), (int)(d->height()/3 * (1 + ani_y)) );
//    move( x(), (int)(Settings::dropPosition().y() * (1 + ani_y)) );

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
