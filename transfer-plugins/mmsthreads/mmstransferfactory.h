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

#ifndef MMSTRANSFERFACTORY_H
#define MMSTRANSFERFACTORY_H

#include "core/plugin/transferfactory.h"
#include "core/transferhandler.h"

class Transfer;
class TransferGroup;
class Scheduler;

class MmsTransferFactory : public TransferFactory
{
    Q_OBJECT
public:
    MmsTransferFactory(QObject *parent, const QVariantList &args);
    ~MmsTransferFactory() override;

    Transfer *createTransfer(const QUrl &srcUrl, const QUrl &destUrl, TransferGroup *parent, Scheduler *scheduler, const QDomElement *e = nullptr) override;

    TransferHandler *createTransferHandler(Transfer *transfer, Scheduler *scheduler) override
    {
        return new TransferHandler(transfer, scheduler);
    }
    QWidget *createDetailsWidget(TransferHandler *transfer) override;

    const QList<QAction *> actions(TransferHandler *handler = nullptr) override;

    bool isSupported(const QUrl &url) const override;

    QString displayName() const override
    {
        return "mms";
    }
};

#endif
