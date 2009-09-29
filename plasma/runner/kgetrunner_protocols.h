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

#ifndef KGETRUNNER_PROTOCOLS_H
#define KGETRUNNER_PROTOCOLS_H

#include <QString>


//  KGETRUNNER_PROTOCOLS defines the default protocols for
//  KGetRunner::reloadConfiguration() and KGetRunnerConfig::defaults()
//  This file can be removed when we have a way of asking kget for the
//  protocols it supports.
static const char* KGETRUNNER_PROTOCOLS = "http https ftp ftps";


#endif
