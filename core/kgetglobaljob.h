/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2009 by Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef KGETGLOBALJOB_H
#define KGETGLOBALJOB_H

#include <kio/job.h>

class KGetGlobalJob : public KJob
{
    Q_OBJECT
public:
    KGetGlobalJob(QObject *parent=0);
    ~KGetGlobalJob();

    void update();    
    
    void start() {};
};

#endif
