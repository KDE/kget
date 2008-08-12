/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef HANDLER_H
#define HANDLER_H

class Scheduler;

class Handler
{
    public:
        Handler(Scheduler * scheduler);
        virtual ~Handler();

        virtual void start() = 0;
        virtual void stop() = 0;

    protected:
        Scheduler * m_scheduler;
};

#endif
