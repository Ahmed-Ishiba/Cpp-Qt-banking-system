#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QByteArray>
#include <QThreadPool>
#include "requesthandler.h"

class NetworkServer : public QObject
{
    Q_OBJECT
public:
    explicit NetworkServer(RequestHandler *handler, QObject *parent = nullptr);
    void start(quint16 port);

public slots:
    void sendResponse(QTcpSocket *client, QByteArray data);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer *m_server;
    RequestHandler *m_handler;
    QThreadPool m_pool;
    QMap<QTcpSocket*, QByteArray> m_buffers;
};