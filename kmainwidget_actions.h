/* This file is part of the KDE project
   Copyright (C) 2004 KGet2x developers < >
                      Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _KMAINWIDGET_ACTION_H
#define _KMAINWIDGET_ACTION_H

#include <kaction.h>
#include <qwidget.h>
#include <qtimer.h>
#include <qvaluevector.h>
#include <stdlib.h>
class KMainWidget;
class QComboBox;
class QSize;
class QPixmap;
class QTimer;
class QPaintEvent;
class QPopupMenu;

/* kmainwidget_actions.h
 *
 * This file contains multiple classes mainly used in the KMAinWidget as
 * actions pluggable to the toolbar. Thus most classes are actions and some
 * are widgets 'embedded' inside the actions.
 *
 */

static const int ToolBar_HEIGHT = 35;

/** Widgets */
class BMW_TTimer : public QTimer
{
  Q_OBJECT
  public:
    BMW_TTimer()
    {
        connect( this, SIGNAL( timeout() ), this, SLOT( ito() ) );
        start( 50 );
    }
  
  private slots:
    void ito() { newSample( drand48() * 3.0 + drand48() * 3.0 ); }

  signals:
    void newSample( float );
};

class BandMonWidget : public QWidget
{
  Q_OBJECT
  public:
    BandMonWidget( bool isVertical = false, QWidget * parent = 0, const char * name = 0 );
    ~BandMonWidget();

    void setLength( int length );
    void setFrequency( float samples_per_second );
    void setMaximum( float max_kbps = -1 );

    QSize minimumSizeHint() const;
    QSize sizeHint() const { return minimumSizeHint(); }
    
  public slots:
    void addSample( float speed_kbps );
    void clear();

  private slots:
    void slotScroll();

  private:
    void drawPixmap();
    void paintEvent( QPaintEvent * );

    int pixelLength;
    bool isVertical;
    float samplesPerSecond;
    float maximum;
    QValueVector<float> samples;
    int samples_current;

    QPixmap * pm;
    QTimer * timer;
};


/** Actions */

// View shape selector
class ComboAction : public KAction
{
  Q_OBJECT
  public:
    ComboAction( const QString& name, const KShortcut&, KActionCollection*,
        KMainWidget*, const char* name );
    virtual int plug( QWidget* w, int index = -1 );

  private slots:
    void slotComboActivated(int);
    void slotViewModeChanged(int);

  private:
    QComboBox * widget;
    KMainWidget * parent;
};

// Label saying "View as:"
class ViewAsAction : public KAction
{
  public:
    ViewAsAction( const QString& name, const KShortcut&, KActionCollection*,
        const char* name );
    virtual int plug( QWidget* w, int index = -1 );
};

// Displays a band graph widget
class BandAction : public KAction
{
  public:
    BandAction( const QString& name, const KShortcut&, KActionCollection*,
        const char* name );
    virtual int plug( QWidget* w, int index = -1 );
};

// Resizeable spacer (to right align actions on toolbars)
class SpacerAction : public KAction
{
  public:
    SpacerAction( const QString& name, const KShortcut&, KActionCollection*,
        const char* name );
    virtual int plug( QWidget* w, int index = -1 );
};

/*
class XyzAction : public KAction
{
public:
    XyzAction( const QString& name, const KShortcut&, KActionCollection* );
    virtual int plug( QWidget* w, int index = -1 );
};
*/
//END 

#endif
