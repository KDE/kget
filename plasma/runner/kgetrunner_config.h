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

#ifndef KGETRUNNERCONFIG_H
#define KGETRUNNERCONFIG_H

#include <KCModule>
#include "ui_kgetrunner_config.h"


class KGetRunnerConfigForm
    : public QWidget, public Ui::KGetRunnerConfig
{
    Q_OBJECT
    
    public:
        
    explicit KGetRunnerConfigForm(QWidget* parent);
};

class KGetRunnerConfig
    : public KCModule
{
    Q_OBJECT
    
    public:
        
    explicit KGetRunnerConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());
    ~KGetRunnerConfig();

    public slots:
        
    void save();
    void load();
    void defaults();

    private:
        
    KGetRunnerConfigForm* m_ui;
};

#endif
