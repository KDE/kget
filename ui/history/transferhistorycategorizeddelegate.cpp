/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "transferhistorycategorizeddelegate.h"

#include <kcategorizedsortfilterproxymodel.h>

#include <QStandardItem>
#include <QVariant>
#include <QDate>
#include <QUrl>

#include <KLocale>

TransferHistoryCategorizedDelegate::TransferHistoryCategorizedDelegate()
{
}

TransferHistoryCategorizedDelegate::~TransferHistoryCategorizedDelegate()
{
}

DateCategorizedDelegate::DateCategorizedDelegate() : TransferHistoryCategorizedDelegate()
{
}

DateCategorizedDelegate::~DateCategorizedDelegate()
{
}

void DateCategorizedDelegate::categorizeItem(QStandardItem *item)
{
    int categorySort = 0;

    QDate date = item->data(TransferHistoryCategorizedDelegate::RoleDate).toDate();
    QVariant value;

    if (date == QDate::currentDate()) {
        value = QVariant(i18n("Today"));
    }
    else if (date.daysTo(QDate::currentDate()) <= 7) {
        categorySort = 1;
        value = QVariant(i18n("Last week"));
    }
    else if (date.daysTo(QDate::currentDate()) <= 30) {
        categorySort = 2;
        value = QVariant(i18n("Last Month"));
    }
    else {
        categorySort = 3;
        value = QVariant(i18n("A long time ago"));
    }

    item->setData(value, KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(categorySort, KCategorizedSortFilterProxyModel::CategorySortRole);
}

SizeCategorizedDelegate::SizeCategorizedDelegate() : TransferHistoryCategorizedDelegate()
{}

SizeCategorizedDelegate::~SizeCategorizedDelegate()
{}

void SizeCategorizedDelegate::categorizeItem(QStandardItem *item)
{
    int size = item->data(TransferHistoryCategorizedDelegate::RoleSize).toInt();
    int categorySort = 0;
    QVariant value;

    if (size < 10 * 1024 * 1024) {
        value = QVariant(i18n("Under 10MiB"));
    }
    else if (size >= 10 * 1024 * 1024  && size < 50 * 1024 *  1024) {
        value = QVariant(i18n("Between 10MiB and 50MiB"));
        categorySort = 1;
    }
    else if (size >= 50 * 1024 * 1024  && size < 100 * 1024 *  1024) {
        value = QVariant(i18n("Between 50MiB and 100MiB"));
        categorySort = 2;
    }
    else {
        categorySort = 3;
        value = QVariant(i18n("More than 100MiB"));
    }

    item->setData(value, KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(categorySort, KCategorizedSortFilterProxyModel::CategorySortRole);
}

HostCategorizedDelegate::HostCategorizedDelegate() : TransferHistoryCategorizedDelegate()
{
}

HostCategorizedDelegate::~HostCategorizedDelegate()
{
}

void HostCategorizedDelegate::categorizeItem(QStandardItem *item)
{
    QUrl host(item->data(TransferHistoryCategorizedDelegate::RoleUrl).toString());

    item->setData(host.host(), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(host.host(), KCategorizedSortFilterProxyModel::CategorySortRole);
}
