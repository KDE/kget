/***************************************************************************
                                dlgLimits.cpp
                             -------------------
    Revision 				: $Id$
    begin						: Tue Jan 29 2002
    copyright				: (C) 2002 by Patrick Charbonnier
									: Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
    email						: pch@freeshell.og
 ***************************************************************************/

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


#include <qlayout.h>

#include <klocale.h>
#include <kdialog.h>

#include "settings.h"
#include "kmainwidget.h"
#include "dlgLimits.h"


DlgLimits::DlgLimits(QWidget * parent):QGroupBox(parent)
{

        setTitle(i18n("Limits Options"));

        QGridLayout *limitsLayout =
                new QGridLayout(this, 4, 2, 20, KDialog::spacingHint());

        limitsLayout->addRowSpacing(1, 25);

        limitsLayout->setRowStretch(0, 5);
        limitsLayout->setRowStretch(1, 0);
        limitsLayout->setRowStretch(2, 5);
        limitsLayout->setRowStretch(3, 5);

        limitsLayout->setColStretch(0, 5);
        limitsLayout->setColStretch(1, 5);

        // opened connection
        lb_maxnum =
                new QLabel(i18n("Maximum number of opened connections :"), this);
        limitsLayout->addWidget(lb_maxnum, 0, 0);

        le_maxnum = new KIntNumInput(0, this, 10);
        le_maxnum->setRange(1, 3600, 1, false);
        limitsLayout->addWidget(le_maxnum, 0, 1);

        // minimum bandwidth
        lb_minband = new QLabel(i18n("Minimum network bandwidth :"), this);
        limitsLayout->addWidget(lb_minband, 2, 0);

        le_minband = new KIntNumInput(0, this, 10);
        le_minband->setRange(1, 100000, 100, false);
        le_minband->setSuffix(i18n("b / sec"));
        limitsLayout->addWidget(le_minband, 2, 1);

        // maximum bandwidth
        lb_maxband = new QLabel(i18n("Maximum network bandwidth :"), this);
        limitsLayout->addWidget(lb_maxband, 3, 0);

        le_maxband = new KIntNumInput(0, this, 10);
        le_maxband->setRange(1, 100000, 100, false);
        le_maxband->setSuffix(i18n("b / sec"));
        limitsLayout->addWidget(le_maxband, 3, 1);

        // TODO: these are not supported yet, so disable them
        le_maxband->setEnabled(false);
        le_minband->setEnabled(false);
}


void DlgLimits::setData()
{
        le_maxnum->setValue(ksettings.maxSimultaneousConnections);
        le_minband->setValue(ksettings.minimumBandwidth);
        le_maxband->setValue(ksettings.maximumBandwidth);
}


void DlgLimits::applyData()
{
        ksettings.maxSimultaneousConnections = le_maxnum->value();
        ksettings.minimumBandwidth = le_minband->value();
        ksettings.maximumBandwidth = le_maxband->value();
        kmain->checkQueue();
}

#include "dlgLimits.moc"
