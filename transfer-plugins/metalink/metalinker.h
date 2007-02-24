/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef Metalinker_H
#define Metalinker_H

class QDomElement;

class Metalinker
{
    public:
        Metalinker();
        void parseMetalinkFile(const KUrl& url);
    private:
        QDomElement readMetalinkFile(const KUrl& url);
};

#endif // Metalinker_H
