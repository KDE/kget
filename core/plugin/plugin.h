/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   based on amarok code by Mark Kretschmann (C) Copyright 2004           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KGET_PLUGIN_H
#define KGET_PLUGIN_H

/* KGet plugins -How they work- [enrico: 0.6]
 *
 * Here is a generic framework for plugins usage. Since the purpose of a
 * plugin is providing some type of well known functionality, there are
 * some requirements that must be satisfied. In fact a plugin must:
 * - inherit KGetPlugin interface or a subclass of that
 * - declare that the class IS a kget plugin (using a macro)
 * - declare its 'type' to the loader (using a .desktop file)
 *
 * Plugins providing the same functionality (TransferFactory for example)
 * must inherit the same interface and be declared as plugins of the same
 * type. In that case all the plugins will inherit and implement the
 * TransferFactory class and provide a .desktop file that identify them
 * as belonging to the same type (X-KDE-KGet-pluginType=TransferFactory).
 *
 * Loading. This operation is done by using KDE framework. So we define a
 * new ServiceType in the kget_plugin.desktop file. KGet plugins service
 * is called "KGet/Plugin". In the Desktop file that describes a plugin
 * the "X-KDE-ServiceType" is set to to "KGet/Plugin" and other fields
 * are set as described in the service type definition.
 * As an example say that we need "InputFilter" plugins. In that case we
 * can use KTrader to enumerate the plugins of that type installed in the
 * system and after getting the name of the libraries they're in, load
 * them. In that case the Input object that loaded the plugins must know
 * how to treat them (and that is easy, since they all reimplemented an
 * 'input plugin class' providing necessary information).
 *
 * @see: kget_plugin.desktop - for servicetype definition.
 * @see: other headers in the dir - for plugin types definition.
 */

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
        KGetPlugin * create_plugin() { return new classname; } \
    }

/**
 * Bump this number whenever the plugin framework gets 
 * incompatible with older versions 
 */
 const int FrameworkVersion = 1;

/**
 * @short Base class for kget plugins.
 * ...
 */
class KGetPlugin
{
    public:
        KGetPlugin();
        virtual ~KGetPlugin();

        /*
        // set and retrieve properties
        void addPluginProperty( const QString & key, const QString & value );
        bool hasPluginProperty( const QString & key );
        QString pluginProperty( const QString & key );

         reimplement this to set the type of the plugin
        enum PluginType { PreProcessing, Factory, PostProcessing }
        virtual PluginType pluginType() = 0;
        */

    private:
        //QMap< QString, QString > m_properties;


};

#endif
