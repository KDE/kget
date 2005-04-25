/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERACTION_H
#define _TRANSFERACTION_H

#include <qvaluelist.h>

#include <kaction.h>

class Transfer;

class TransferAction : public KAction
{
    public:
        TransferAction( const QString& text, const QString& pix, 
                        const KShortcut& cut, KActionCollection* parent, 
                        const char* name );
        ~TransferAction() {}

        virtual void execute(const QValueList<TransferHandler *> & transfers) =0;

    public slots:
        void activate();
};

#endif
