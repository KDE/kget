/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGET_EXPORT_H
#define KGET_EXPORT_H

#include <KPluginFactory>

/**
 * @short Declares a class as plugin.
 *
 * Exports a common symbol in a shared library used as an entry point for
 * the creation of an instance of the class.
 *
 * Usage example:
 *     KGET_EXPORT_PLUGIN( CoolKgetPlugin );
 *     class CoolKgetPlugin : public KGetPlugin {
 *         ....
 *     }u
 */
#define KGET_EXPORT_PLUGIN( classname ) \
K_PLUGIN_FACTORY(KGetFactory, \
                 registerPlugin<TransferKioFactory>(); \
)
    

#define KGET_EXPORT_PLUGIN_CONFIG( classname ) \
    K_PLUGIN_FACTORY( KGetFactory, registerPlugin< classname >(); )

#endif
