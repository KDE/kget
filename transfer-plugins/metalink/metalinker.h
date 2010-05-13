/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef Metalinker_H
#define Metalinker_H

#include <KUrl>

class MlinkFileData
{
    public:
        MlinkFileData() {}

        /**
         * Controlls if the name attribute is valid, i.e. it is not empty and
         * does not contain any directory traversal directives or information
         * In case of faulty fileNames the MlinkFile gets discarded
         */
        bool isValidNameAttribute() const;

        QString fileName;
        QString md5;
        QString sha256;
        KUrl::List urls;
};


class Metalinker
{
    public:
        Metalinker();
        static QList<MlinkFileData> parseMetalinkFile(const QByteArray& data);
        static bool verifyMD5(QIODevice& file, const QString& md5);
};

#endif // Metalinker_H
