/***************************************************************************
*                                bwdatetime.h
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

/*
 KPostit - postit notes for the KDE Project
 Copyright (C) 1997 Bernd Johannes Wuebben
 
 wuebben@math.cornell.edu 
 wuebben@kde.org
 
 This class is a modified version of a class found in:
 
 qtremind - an X windows appoint reminder program.
 Copyright (C) 1997  Tom Daley
 */


#ifndef BW_DATE_TIME_H_
#define BW_DATE_TIME_H_

#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qdatetime.h>
#include "common.h"
// #include "spin.h"

class QSpinBox;

class BWDateTime:public QWidget {

  Q_OBJECT public:

     BWDateTime(QDateTime qdt, QWidget * parent = 0, const char *name = 0);

    void setDateTime(QDateTime qdtime);
    QDateTime getDateTime();
    bool checkDateTime();
    void setEnabled(bool);

    bool time_notvalid;
    bool date_notvalid;

  private:
     bool use12Clock;

    QSpinBox *hour;
    QSpinBox *minute;
    QSpinBox *month;
    QSpinBox *year;
    QSpinBox *day;

    QRadioButton *am, *pm;
    QButtonGroup *ampm;
    QLabel *daylabel;
    QLabel *monthlabel;
    QLabel *yearlabel;
    QLabel *timelabel;
    QDateTime mydatetime;

    private slots:		// Private slots
	/** No descriptions */
    void slotValueChanged(int);
     signals:			// Signals
	/** No descriptions */
    void signalDateChanged(const QDateTime &);
};

#endif
