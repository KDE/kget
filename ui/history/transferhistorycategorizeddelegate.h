/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERHISTORYCATEGORIZEDDELEGATE_H
#define TRANSFERHISTORYCATEGORIZEDDELEGATE_H

#include <QObject>

class QStandardItem;

class TransferHistoryCategorizedDelegate : public QObject
{
Q_OBJECT
public:
    enum AlternativeRoles {
        RoleSize = 0xFF,
        RoleDate = 0xFE,
        RoleUrl = 0xFD,
        RoleDest = 0xFA
    };

    TransferHistoryCategorizedDelegate();
    ~TransferHistoryCategorizedDelegate();

    virtual void categorizeItem(QStandardItem *item)
    {
        Q_UNUSED(item)
    };
};

/** Categorizes the transfers in date ranges **/
class DateCategorizedDelegate : public TransferHistoryCategorizedDelegate
{
public:
    DateCategorizedDelegate();
    ~DateCategorizedDelegate();

    void categorizeItem(QStandardItem *item);
};

/** Categorzes the transfers in size ranges **/
class SizeCategorizedDelegate : public TransferHistoryCategorizedDelegate
{
public:
    SizeCategorizedDelegate();
    ~SizeCategorizedDelegate();

    void categorizeItem(QStandardItem *item);
};

/** Categorizes the transfer by source host **/
class HostCategorizedDelegate : public TransferHistoryCategorizedDelegate
{
public:
    HostCategorizedDelegate();
    ~HostCategorizedDelegate();

    void categorizeItem(QStandardItem *item);
};

#endif
