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

#include "kgetrunner_config.h"
#include "kgetrunner_protocols.h"
#include <QGridLayout>
#include <KConfigGroup>
#include <Plasma/AbstractRunner>


K_EXPORT_RUNNER_CONFIG(kget, KGetRunnerConfig)


KGetRunnerConfigForm::KGetRunnerConfigForm(QWidget* parent)
    : QWidget(parent)
{
  setupUi(this);
}


KGetRunnerConfig::KGetRunnerConfig(QWidget* parent, const QVariantList& args)
    : KCModule(ConfigFactory::componentData(), parent, args)
{
    m_ui = new KGetRunnerConfigForm(this);
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->editProtocols, SIGNAL(textChanged(const QString&)), this, SLOT(changed()));

    load();
}

KGetRunnerConfig::~KGetRunnerConfig()
{
}

void KGetRunnerConfig::load()
{
    KCModule::load();

    //FIXME: This shouldn't be hardcoded!
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig("krunnerrc");
    KConfigGroup conf = cfg->group("Runners");
    KConfigGroup grp = KConfigGroup(&conf, "KGet");

    const QString protocols = grp.readEntry("protocols", KGETRUNNER_PROTOCOLS);
    m_ui->editProtocols->setText(protocols);

    emit changed(false);
}

void KGetRunnerConfig::save()
{
    //FIXME: This shouldn't be hardcoded!
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig("krunnerrc");
    KConfigGroup conf = cfg->group("Runners");
    KConfigGroup grp = KConfigGroup(&conf, "KGet");

    grp.writeEntry("protocols", m_ui->editProtocols->text());
    grp.sync();

    emit changed(false);
}

void KGetRunnerConfig::defaults()
{
    m_ui->editProtocols->setText(KGETRUNNER_PROTOCOLS);
    
    emit changed(true);
}


#include "kgetrunner_config.moc"
