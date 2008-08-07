/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "scriptconfigadaptor.h"

#include <QFileInfo>
#include <QString>

#include <KStandardDirs>
#include <KDebug>

bool ScriptConfigAdaptor::setFile(const QString &filename,
                                  const QString &path)
{
    if (!m_config)
    {
        if (!path.isEmpty())
        {
            QFileInfo info(path);
            if (info.isDir() && info.exists())
            {
                // check for the ending slash
                if (path.endsWith('/'))
                {
                    m_config = new KConfig(path + filename);
                }
                else
                {
                    m_config = new KConfig(path + '/' + filename);
                }
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            QString default_path;
            // put the config file in default user directory
            // like ~/.kde4/share/app/kget/content_scrips_setting/
            default_path = KStandardDirs::locateLocal("appdata", "contentfetch_scripts_setting/");
            m_config = new KConfig(default_path + filename);
            return true;
        }
    }
    return false;
}

void ScriptConfigAdaptor::unsetFile()
{
    delete m_config;
    m_config = 0;
}

QVariant ScriptConfigAdaptor::read(const QString& group,
                                   const QString& key,
                                   const QVariant& defaultvalue)
{
    m_group = m_config->group(group);
    QVariant value;
    switch(defaultvalue.type())
    {
        case QVariant::Int:
            value = m_group.readEntry(key, defaultvalue.toInt());
            break;
        case QVariant::Double:
            value = m_group.readEntry(key, defaultvalue.toDouble());
            break;
        case QVariant::StringList:
            value = m_group.readEntry(key, defaultvalue.toStringList());
            break;
        case QVariant::List:
            value = m_group.readEntry(key, defaultvalue.toList());
            break;
        default:
            value = m_group.readEntry(key, defaultvalue.toString());
            break;
    };
    return value;
}

void ScriptConfigAdaptor::write(const QString& group,
                                const QString& key,
                                const QVariant& value)
{
    m_group = m_config->group(group);
    switch(value.type())
    {
        case QVariant::Int:
            m_group.writeEntry(key, value.toInt());
            break;
        case QVariant::Double:
            m_group.writeEntry(key, value.toDouble());
            break;
        case QVariant::StringList:
            m_group.writeEntry(key, value.toStringList());
            break;
        case QVariant::List:
            m_group.writeEntry(key, value.toList());
            break;
        default:
            m_group.writeEntry(key, value.toString());
            break;
    };
}

void ScriptConfigAdaptor::save()
{
    m_config->sync();
}

void ScriptConfigAdaptor::reset()
{
    m_config->markAsClean();
    m_config->reparseConfiguration();
}
