/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef SCRIPT_CONFIGADAPTOR_H
#define SCRIPT_CONFIGADAPTOR_H

#include <QObject>
#include <QString>
#include <QVariant>

#include <kconfig.h>
#include <kconfiggroup.h>

class ScriptConfigAdaptor : public QObject
{
        Q_OBJECT
    public:
        ScriptConfigAdaptor(QObject* parent = 0) : QObject(parent), m_config(0) {}
        virtual ~ScriptConfigAdaptor()
        {
            delete m_config;
        }

    public slots:
        bool setFile(const QString &filename, const QString &path = QString());
        void unsetFile();
        QVariant read(const QString &group, const QString &key,
                      const QVariant &defaultvalue = QVariant());
        void write(const QString &group, const QString &key,
                   const QVariant &value);
        void save();
        /**
         * Discard current setting, reread from disk file.
         *
         */
        void reset();

    private:
        KConfig* m_config;
        KConfigGroup m_group;
};

#endif // SCRIPT_CONFIGADAPTOR_H
