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
#include <nepomuk/resource.h>
#include <QObject>
class Transfer;

class KGET_EXPORT NepomukHandler : public QObject
{
    Q_OBJECT
    public:
        NepomukHandler(Transfer *transfer, QObject *parent);
        ~NepomukHandler();

        QStringList tags() const;
        int rating() const;

    public slots:
        void setRating(int rating);
        void addTag(const QString &newTag);
        void removeTag(const QString &oldTag);
        void saveFileProperties();

    protected:
        virtual void saveFileProperties(const Nepomuk::Resource &res);

    private:
        Nepomuk::Resource m_resource;
        Transfer *m_transfer;
};

#endif
