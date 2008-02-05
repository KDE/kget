/* This file is part of the KDE project

   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QWidget>

class QTcpServer;

class HttpServer : public QObject
{
    Q_OBJECT

public:
    HttpServer(QWidget *parent = 0);

private slots:
    void handleRequest();

private:
    QTcpServer *tcpServer;
};

#endif
