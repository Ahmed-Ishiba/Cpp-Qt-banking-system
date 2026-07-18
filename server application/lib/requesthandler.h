#pragma once
#include <QJsonObject>
#include <QTcpSocket>
#include <QMap>
#include <QMutex>
#include <string>
#include "database.h"

class RequestHandler
{
public:
    explicit RequestHandler(Database *db);

    QJsonObject handle(QTcpSocket *client, const QJsonObject &request);
    void removeSession(QTcpSocket *client);

private:
    struct Session {
        bool loggedIn = false;
        bool isAdmin = false;
        std::string username;
    };

    Database *m_db;
    QMutex m_mutex;
    QMap<QTcpSocket*, Session> m_sessions;

    QJsonObject handleLogin(Session &session, const QJsonObject &req);
    QJsonObject handleGetAccountNumber(Session &session, const QJsonObject &req);
    QJsonObject handleGetAccountNumberAdmin(Session &session, const QJsonObject &req);
    QJsonObject handleViewBalance(Session &session, const QJsonObject &req);
    QJsonObject handleViewHistory(Session &session, const QJsonObject &req);
    QJsonObject handleMakeTransaction(Session &session, const QJsonObject &req);
    QJsonObject handleTransfer(Session &session, const QJsonObject &req);
    QJsonObject handleViewDatabase(Session &session, const QJsonObject &req);
    QJsonObject handleCreateUser(Session &session, const QJsonObject &req);
    QJsonObject handleDeleteUser(Session &session, const QJsonObject &req);
    QJsonObject handleUpdateUser(Session &session, const QJsonObject &req);
};