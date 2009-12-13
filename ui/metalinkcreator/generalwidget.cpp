/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
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

#include "generalwidget.h"

#include <KSystemTimeZone>

#include "metalinker.h"

GeneralWidget::GeneralWidget(QWidget *parent)
  : QWidget (parent)
{
    ui.setupUi(this);

    ui.dynamic->setToolTip(ui.labelDynamic->toolTip());

    connect(ui.publishedGroupBox, SIGNAL(toggled(bool)), this, SLOT(slotPublishedEnabled(bool)));
    connect(ui.updatedGroupBox, SIGNAL(toggled(bool)), this, SLOT(slotUpdatedEnabled(bool)));
}

void GeneralWidget::load(const KGetMetalink::Metalink &metalink) const
{
    ui.origin->setUrl(metalink.origin);
    ui.dynamic->setChecked(metalink.dynamic);

    ui.publishedGroupBox->setChecked(metalink.published.isValid());
    ui.use_publishedtimeoffset->setChecked(metalink.published.timeZoneOffset.isValid());
    if (metalink.published.isValid()) {
        ui.published->setDateTime(metalink.published.dateTime);
        ui.publishedoffset->setTime(metalink.published.timeZoneOffset);
        ui.publishedNegative->setChecked(metalink.published.negativeOffset);
    } else {
        ui.published->setDateTime(QDateTime::currentDateTime());
        int offset = KSystemTimeZones::local().currentOffset();
        const bool negativeOffset = (offset < 0);
        offset = abs(offset);
        QTime time = QTime(0, 0, 0);
        time = time.addSecs(abs(offset));
        ui.publishedoffset->setTime(time);

        //to not enable publishedNegative block the signals
        ui.use_publishedtimeoffset->blockSignals(true);
        ui.use_publishedtimeoffset->setChecked(true);
        ui.use_publishedtimeoffset->blockSignals(false);

        ui.publishedNegative->setChecked(negativeOffset);
    }

    ui.updatedGroupBox->setChecked(metalink.updated.isValid());
    ui.use_updatedtimeoffset->setChecked(metalink.updated.timeZoneOffset.isValid());
    if (metalink.updated.isValid()) {
        ui.updated->setDateTime(metalink.updated.dateTime);
        ui.updatedoffset->setTime(metalink.updated.timeZoneOffset);
        ui.updatedNegative->setChecked(metalink.updated.negativeOffset);
    } else {
        ui.updated->setDateTime(QDateTime::currentDateTime());
        int offset = KSystemTimeZones::local().currentOffset();
        const bool negativeOffset = (offset < 0);
        QTime time = QTime(0, 0, 0);
        time = time.addSecs(abs(offset));
        ui.updatedoffset->setTime(time);

        //to not enable publishedNegative block the signals
        ui.use_updatedtimeoffset->blockSignals(true);
        ui.use_updatedtimeoffset->setChecked(true);
        ui.use_updatedtimeoffset->blockSignals(false);

        ui.updatedNegative->setChecked(negativeOffset);
    }
}

void GeneralWidget::save(KGetMetalink::Metalink *metalink)
{
    metalink->origin = KUrl(ui.origin->text());
    metalink->dynamic = ui.dynamic->isChecked();

    metalink->published.clear();
    if (ui.publishedGroupBox->isChecked()) {
        metalink->published.dateTime = ui.published->dateTime();
        if (ui.use_publishedtimeoffset->isChecked()) {
            metalink->published.timeZoneOffset = ui.publishedoffset->time();
        }
    }

    metalink->updated.clear();
    if (ui.updatedGroupBox->isChecked()) {
        metalink->updated.dateTime = ui.updated->dateTime();
        if (ui.use_updatedtimeoffset->isChecked()) {
            metalink->updated.timeZoneOffset = ui.updatedoffset->time();
        }
    }
}

void GeneralWidget::slotPublishedEnabled(bool enabled)
{
    if (enabled) {
        ui.publishedNegative->setEnabled(ui.use_publishedtimeoffset->isChecked());
    }
}

void GeneralWidget::slotUpdatedEnabled(bool enabled)
{
    if (enabled) {
        ui.updatedNegative->setEnabled(ui.use_updatedtimeoffset->isChecked());
    }
}


#include "generalwidget.moc"
