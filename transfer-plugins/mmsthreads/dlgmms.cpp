/*
    This file is part of the KDE project
    Copyright (C) 2011 Ernesto Rodriguez Ortiz <eortiz@uci.cu>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "dlgmms.h"
#include "mmssettings.h"
#include "kget_export.h"

KGET_EXPORT_PLUGIN_CONFIG(DlgMmsSettings)

DlgMmsSettings::DlgMmsSettings(QWidget *parent, const QVariantList &args)
    : KCModule(KGetFactory::componentData(), parent, args)
{
    ui.setupUi(this);
    connect(ui.numThreadSpinBox, SIGNAL(valueChanged(int)), SLOT(changed()));
}

void DlgMmsSettings::load()
{
    ui.numThreadSpinBox->setValue(MmsSettings::threads());
}

void DlgMmsSettings::save()
{
    qDebug() << "Saving Multithreaded config";
    MmsSettings::setThreads(ui.numThreadSpinBox->value());
    MmsSettings::self()->writeConfig();
}

#include "dlgmms.moc"

