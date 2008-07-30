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
#include <kconfig.h>
#include <kconfigbase.h>
#include <kconfiggroup.h>

class ScriptConfigAdaptor : public QObject
{
        Q_OBJECT
    public:

        explicit ScriptConfigAdaptor(const QString& groupname, QObject* parent = 0) : QObject(parent)
        {
            // TODO: Fix hard coding
            m_config = new KConfig("kgetscripts.rc");
            setGroup(groupname);
        }

        virtual ~ScriptConfigAdaptor()
        {
            //m_config->sync();
            delete m_config;
        }

    public slots:

        void setGroup(const QString& groupname)
        {
            m_group = m_config->group(groupname);
        }

        QVariant read(const QString& name, const QVariant& defaultvalue = QVariant())
        {
            QVariant value;
            switch(defaultvalue.type()) {
                case QVariant::Int:
                    value = m_group.readEntry(name, defaultvalue.toInt());
                    break;
                case QVariant::Double:
                    value = m_group.readEntry(name, defaultvalue.toDouble());
                    break;
                case QVariant::StringList:
                    value = m_group.readEntry(name, defaultvalue.toStringList());
                    break;
                case QVariant::List:
                    value = m_group.readEntry(name, defaultvalue.toList());
                    break;
                default:
                    value = m_group.readEntry(name, defaultvalue.toString());
                    break;
            };
            return value;
        }

        void write(const QString& name, const QVariant& value)
        {
            switch(value.type()) {
                case QVariant::Int:
                    m_group.writeEntry(name, value.toInt());
                    break;
                case QVariant::Double:
                    m_group.writeEntry(name, value.toDouble());
                    break;
                case QVariant::StringList:
                    m_group.writeEntry(name, value.toStringList());
                    break;
                case QVariant::List:
                    m_group.writeEntry(name, value.toList());
                    break;
                default:
                    m_group.writeEntry(name, value.toString());
                    break;
            };
        }

    private:
        KConfig* m_config;
        KConfigGroup m_group;
};

#endif // SCRIPT_CONFIGADAPTOR_H
