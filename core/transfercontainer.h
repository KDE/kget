/* This file is part of the KDE project

   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERCONTAINER_H
#define TRANSFERCONTAINER_H

#include <QObject>

/**
 * @brief TransferContainer class handles the transfers
 *
 * This class is the base for every single transfer.
 *
 */

class TransferContainer : public QObject
{
    Q_OBJECT

public:
    Q_FLAGS(TransferFlags)

    enum TransferFlag {
        /**  The transfer supports resuming and seeking  */
        Tf_SupportsSegments    = 0x01,
        /**  The container includes more than one file, e.g. metalink  */
        Tf_MultipleFiles       = 0x02
    };

    Q_DECLARE_FLAGS(TransferFlags, TransferFlag)

    TransferContainer();
    ~TransferContainer();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TransferContainer::TransferFlags)

#endif
