/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef NEPOMUKHANDLER_H
#define NEPOMUKHANDLER_H

#include "kget_export.h"

#include <QObject>
#include <KFileItemList>

namespace Nepomuk {
    class Variant;
}

class Transfer;

class KGET_EXPORT NepomukHandler : public QObject
{
    Q_OBJECT

    public:
        NepomukHandler(Transfer *transfer);
        virtual ~NepomukHandler();

        KFileItemList fileItems() const;

        /**
         * Sets properties to a list of files, if the list is empty m_destinations is used
         */
        void setProperties(const QList<QPair<QUrl, Nepomuk::Variant> > &properties, const QList<KUrl> &files = QList<KUrl>());

        void saveFileProperties();

        /**
         * Removes the resource if the isValid() is false
         * @note call this method at the end of the deinit of the transfer,
         * otherwise it might not work correctly
         */
        void deinit();

    private:
        Transfer *m_transfer;
};

#endif
