/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef KGET_EXPORT_H
#define KGET_EXPORT_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>

/* export statements for unix */
#define KGET_EXPORT KDE_EXPORT


/**
 * @short Declares a class as plugin.
 *
 * Exports a common symbol in a shared library used as an entry point for
 * the creation of an istance of the class.
 *
 * Usage example:
 *     KGET_EXPORT_PLUGIN( CoolKgetPlugin );
 *     class CoolKgetPlugin : public KGetPlugin {
 *         ....
 *     }
 */
#define KGET_EXPORT_PLUGIN( classname ) \
    extern "C" { \
        KDE_EXPORT KGetPlugin * create_plugin() { return new classname; } \
    }


#endif
