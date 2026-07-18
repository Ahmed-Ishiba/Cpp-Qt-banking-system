#include "networkserver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QRunnable>
#include <QMetaObject>

namespace {
class RequestTask : public QRunnable
{
public:
    RequestTask(NetworkServer *server, RequestHandler *handler, QTcpSocket *client, QByteArray line)
        : m_server(server), m_handler(handler), m_client(client), m_line(std::move(line)) {}

    void run() override
    {
        QJsonDocument doc = QJsonDocument::fromJson(m_line);
        if (!doc.isObject()) return;

        QJsonObject response = m_handler->handle(m_client, doc.object());
        QJsonDocument responseDoc(response);
        QByteArray out = responseDoc.toJson(QJsonDocument::Compact) + "\n";

        QMetaObject::invokeMethod(m_server, "sendResponse", Qt::QueuedConnection,
                                  Q_ARG(QTcpSocket*, m_client), Q_ARG(QByteArray, out));
    }

private:
    NetworkServer *m_server;
    RequestHandler *m_handler;
    QTcpSocket *m_client;
    QByteArray m_line;
};
}

NetworkServer::NetworkServer(RequestHandler *handler, QObject *parent)
    : QObject(parent), m_handler(handler)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &NetworkServer::onNewConnection);
}

void NetworkServer::start(quint16 port)
{
    if (m_server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server listening on port" << port;
    } else {
        qDebug() << "Failed to start server:" << m_server->errorString();
    }
}

void NetworkServer::onNewConnection()
{
    QTcpSocket *client = m_server->nextPendingConnection();
    m_buffers[client] = QByteArray();

    connect(client, &QTcpSocket::readyRead, this, &NetworkServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &NetworkServer::onDisconnected);

    qDebug() << "Client connected:" << client->peerAddress().toString();
}

void NetworkServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    m_buffers[client].append(client->readAll());

    while (m_buffers[client].contains('\n')) {
        int idx = m_buffers[client].indexOf('\n');
        QByteArray line = m_buffers[client].left(idx);
        m_buffers[client].remove(0, idx + 1);

        RequestTask *task = new RequestTask(this, m_handler, client, line);
        m_pool.start(task);
    }
}

void NetworkServer::sendResponse(QTcpSocket *client, QByteArray data)
{
    if (client->state() == QAbstractSocket::ConnectedState) {
        client->write(data);
    }
}

void NetworkServer::onDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    m_buffers.remove(client);
    m_handler->removeSession(client);
    qDebug() << "Client disconnected";
    client->deleteLater();
}