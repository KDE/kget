/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btspeedlimits.h"

#include "ui_btspeedlimits.h"

#include <KDebug>

BTSpeedLimits::BTSpeedLimits(BTTransferHandler * handler, QWidget * parent)
  : KDialog(parent),
    m_handler(handler)
{
    Ui::BTSpeedLimits ui;
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    setMainWidget(widget);

    m_dlBox = ui.dlBox;
    m_ulBox = ui.ulBox;
    m_shareRatioSpin = ui.shareRatioSpin;
    m_dlBox->setValue(m_handler->visibleDownloadLimit());
    m_ulBox->setValue(m_handler->visibleUploadLimit());
    m_shareRatioSpin->setValue((double) m_handler->maxShareRatio());

    connect(this, SIGNAL(accepted()), SLOT(setSpeedLimitsAndClose()));
    connect(this, SIGNAL(accepted()), SLOT(onlyClose()));
}

void BTSpeedLimits::setSpeedLimitsAndClose()
{
    m_handler->setVisibleUploadLimit(m_ulBox->value());
    m_handler->setVisibleDownloadLimit(m_dlBox->value());
    m_handler->setMaxShareRatio(m_shareRatioSpin->value());
    emit aboutToClose();
}

void BTSpeedLimits::onlyClose()
{
    emit aboutToClose();
}
