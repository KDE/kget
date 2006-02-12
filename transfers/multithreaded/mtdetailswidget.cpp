/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "mtdetailswidget.h"
#include "mtdetailswidget.moc"

MTDetailsWidget::MTDetailsWidget()
{
    frm.setupUi(this);
    frm.numberThreadSpinBox->setValue(Settings::mtThreads());
    connect(frm.pushButton, SIGNAL(clicked()),SLOT(slotSetThreads()));
}

void MTDetailsWidget::slotSetThreads()
{
    QVariant value(frm.numberThreadSpinBox->value());
    int nt = value.toInt();
    Settings::setMtThreads( nt );
}
