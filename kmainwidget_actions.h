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
#include <qptrvector.h>
class KMainWidget;
class QComboBox;
class QSize;
class QPixmap;
class QPaintEvent;
class QPopupMenu;

/* kmainwidget_actions.h
 *
 * This file contains multiple classes mainly used in the KMAinWidget as
 * actions pluggable to the toolbar. Thus most classes are actions and some
 * are widgets 'embedded' inside the actions.
 *
 */

/** Widgets */
class BMW_TTimer : public QTimer
{
  Q_OBJECT
  public:
    BMW_TTimer()
    {
        connect( this, SIGNAL( timeout() ), this, SLOT( ito() ) );
        start( 100 );
    }
  
  private slots:
    void ito()
    {
        static float i = 1.0;
        static bool toggle = false;
        i *= 1.01;
        newSample( (toggle = !toggle) ? i : 1.5-i );
    }

  signals:
    void newSample( float );

};

class BandMonWidget : public QWidget
{
  Q_OBJECT
  public:
    enum Direction { Horizontal, Vertical };
    enum ScaleMode { ManualMax, AutoPeak, AutoMean };
    
    BandMonWidget( enum Direction, QWidget * parent = 0, const char * name = 0 );
    ~BandMonWidget();

    void setLength( int length );
    
    QSize minimumSizeHint() const;
    QSize sizeHint() const { return minimumSizeHint(); }
    
  public slots:
    // data plot related slots
    void addSample( float speed_kbps );
    void clear();

    // display mode related slots
    void setDuration( float time ) { timeGap = time; }
    void setScaleMode( enum ScaleMode s ) { scale = s; }
    void enableCompression( bool enabled ) { bCompression = enabled; }
    void enableText( bool enabled ) { bText = enabled; }

  protected:
    void draw();
    void paintEvent( QPaintEvent * );
    double timeStamp();

  private:
    int len;
    enum Direction dir;
    enum ScaleMode scale;
    bool bCompression;
    bool bText;
    float timeGap;
    
    struct SpeedSample { float speed_kbps, time_stamp; };
    QPtrVector<SpeedSample> samples;
    int samples_current;
    
    QPixmap * pm;
    QPopupMenu * popup;
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
