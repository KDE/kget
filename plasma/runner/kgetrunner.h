/*
 *   This file is part of the KDE project.
 *
 *   Copyright (C) 2009 Tomas Van Verrewegen <tomasvanverrewegen@telenet.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published
 *   by the Free Software Foundation; either version 2 of the License,
 *   or (at your option) any later version.
 */

#ifndef KGETRUNNER_H
#define KGETRUNNER_H

#include <Plasma/AbstractRunner>
#include <KIcon>
class QDBusPendingCallWatcher;


class KGetRunner
    : public Plasma::AbstractRunner
{
    Q_OBJECT

    public:
        
    KGetRunner(QObject* parent, const QVariantList& args);
    ~KGetRunner();

    void match(Plasma::RunnerContext& context);
    void run(const Plasma::RunnerContext& context, const Plasma::QueryMatch& match);
    void reloadConfiguration();
    
    private slots:
    
    void showNewTransferDialog();
    void callFinished(QDBusPendingCallWatcher* call);
        
    private:
    
    QStringList parseUrls(const QString& text) const;
    
    KIcon m_icon;
    QStringList m_urls;
    QStringList m_protocols;
};


K_EXPORT_PLASMA_RUNNER(kget, KGetRunner)


#endif
