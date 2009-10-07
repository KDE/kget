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
#include <QtXml/QDomElement>
#include <KUrl>

class Transfer;

class KGET_EXPORT NepomukHandler : public QObject
{
    Q_OBJECT

    public:
        NepomukHandler(Transfer *transfer);
        virtual ~NepomukHandler();

        virtual QStringList tags() const;
        virtual int rating() const;

        void setNewDestination(const KUrl &newDestination);

    public slots:
        virtual void setRating(int rating);
        virtual void addTag(const QString &newTag);
        virtual void addTags(const QStringList &newTags);
        virtual void removeTag(const QString &oldTag);

        /**
         * Set a property of the resource.
         *
         * @param uri The URI identifying the property.
         * @param value The value of the property (i.e. the object of the RDF triple(s))
         */
        virtual void setProperty(const QUrl &uri, const Nepomuk::Variant &value);

        virtual void saveFileProperties();

        /**
         * Removes the resource if the isValid() is false
         * @note call this method at the end of the deinit of the transfer,
         * otherwise it might not work correctly
         */
        virtual void deinit();

    protected:
        bool isValid() const;
        void saveFileProperties(const Nepomuk::Resource &res);
        Transfer *m_transfer;

    private:
        KUrl m_destination;
        Nepomuk::Resource m_resource;
};

#endif
