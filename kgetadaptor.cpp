/*
 * This file was generated by dbusidl2cpp version 0.5
 * when processing input file org.kde.kget.xml
 *
 * dbusidl2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file.
 */

#include "kgetadaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class KGetAdaptor
 */

KGetAdaptor::KGetAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

KGetAdaptor::~KGetAdaptor()
{
    // destructor
}

bool KGetAdaptor::isDropTargetVisible()
{
    // handle method call org.kde.kget.isDropTargetVisible
    bool out0;
    QMetaObject::invokeMethod(parent(), "isDropTargetVisible", Q_RETURN_ARG(bool, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->isDropTargetVisible();
    return out0;
}

bool KGetAdaptor::isOfflineMode()
{
    // handle method call org.kde.kget.isOfflineMode
    bool out0;
    QMetaObject::invokeMethod(parent(), "isOfflineMode", Q_RETURN_ARG(bool, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->isOfflineMode();
    return out0;
}

void KGetAdaptor::setDropTargetVisible(bool in0)
{
    // handle method call org.kde.kget.setDropTargetVisible
    QMetaObject::invokeMethod(parent(), "setDropTargetVisible", Q_ARG(bool, in0));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setDropTargetVisible(in0);
}

void KGetAdaptor::setOfflineMode(bool in0)
{
    // handle method call org.kde.kget.setOfflineMode
    QMetaObject::invokeMethod(parent(), "setOfflineMode", Q_ARG(bool, in0));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setOfflineMode(in0);
}


#include "kgetadaptor.moc"
