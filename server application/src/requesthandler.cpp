#include "requesthandler.h"
#include "user.h"
#include "admin.h"
#include <QMutexLocker>

RequestHandler::RequestHandler(Database *db) : m_db(db)
{
}

QJsonObject RequestHandler::handle(QTcpSocket *client, const QJsonObject &request)
{
    QMutexLocker locker(&m_mutex); // serializes all requests — simple and safe for a learning project
    Session &session = m_sessions[client];
    QString action = request["action"].toString();

    if (action == "login") return handleLogin(session, request);
    if (action == "getAccountNumber") return handleGetAccountNumber(session, request);
    if (action == "getAccountNumberAdmin") return handleGetAccountNumberAdmin(session, request);
    if (action == "viewBalance") return handleViewBalance(session, request);
    if (action == "viewTransactionHistory") return handleViewHistory(session, request);
    if (action == "makeTransaction") return handleMakeTransaction(session, request);
    if (action == "transferAmount") return handleTransfer(session, request);
    if (action == "viewBankDatabase") return handleViewDatabase(session, request);
    if (action == "createNewUser") return handleCreateUser(session, request);
    if (action == "deleteUser") return handleDeleteUser(session, request);
    if (action == "updateUser") return handleUpdateUser(session, request);

    QJsonObject resp;
    resp["action"] = action;
    resp["success"] = false;
    resp["error"] = "unknown action";
    return resp;
}

void RequestHandler::removeSession(QTcpSocket *client)
{
    QMutexLocker locker(&m_mutex);
    m_sessions.remove(client);
}

QJsonObject RequestHandler::handleLogin(Session &session, const QJsonObject &req)
{
    std::string username = req["username"].toString().toStdString();
    std::string password = req["password"].toString().toStdString();

    bool isAdmin = false;
    bool ok = m_db->login(username, password, isAdmin);

    QJsonObject resp;
    resp["action"] = "login";
    resp["success"] = ok;

    if (ok) {
        session.loggedIn = true;
        session.isAdmin = isAdmin;
        session.username = username;
        resp["isAdmin"] = isAdmin;
    }
    return resp;
}

QJsonObject RequestHandler::handleGetAccountNumber(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "getAccountNumber";

    if (!session.loggedIn || session.isAdmin) {
        resp["accountNumber"] = "";
        return resp;
    }

    std::string targetUsername = req["username"].toString().toStdString();
    QJsonObject account = m_db->findAccountbyUsername(targetUsername);
    resp["accountNumber"] = account.isEmpty() ? "" : account["accountNumber"].toString();
    return resp;
}

QJsonObject RequestHandler::handleGetAccountNumberAdmin(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "getAccountNumberAdmin";

    if (!session.loggedIn || !session.isAdmin) {
        resp["accountNumber"] = "";
        return resp;
    }

    Admin admin(session.username);
    std::string targetUsername = req["username"].toString().toStdString();
    resp["accountNumber"] = QString::fromStdString(admin.getAccountNumberOf(targetUsername, *m_db));
    return resp;
}

QJsonObject RequestHandler::handleViewBalance(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "viewBalance";

    if (!session.loggedIn) {
        resp["success"] = false;
        return resp;
    }

    std::string accountNumber = req["accountNumber"].toString().toStdString();
    QJsonObject account = m_db->findAccountbyNumber(accountNumber);

    if (account.isEmpty()) {
        resp["success"] = false;
        return resp;
    }

    resp["success"] = true;
    resp["balance"] = account["balance"].toInt();
    return resp;
}

QJsonObject RequestHandler::handleViewHistory(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "viewTransactionHistory";

    if (!session.loggedIn) {
        resp["history"] = "[]";
        return resp;
    }

    std::string accountNumber = req["accountNumber"].toString().toStdString();
    int count = req["count"].toInt();
    resp["history"] = m_db->getTransactionHistory(accountNumber, count);
    return resp;
}

QJsonObject RequestHandler::handleMakeTransaction(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "makeTransaction";

    if (!session.loggedIn || session.isAdmin) {
        resp["success"] = false;
        return resp;
    }

    User user(session.username, *m_db);
    int32_t amount = req["amount"].toInt();
    resp["success"] = user.makeTransaction(amount, *m_db);
    return resp;
}

QJsonObject RequestHandler::handleTransfer(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "transferAmount";

    if (!session.loggedIn || session.isAdmin) {
        resp["success"] = false;
        return resp;
    }

    User user(session.username, *m_db);
    std::string to = req["toAccountNumber"].toString().toStdString();
    int32_t amount = req["amount"].toInt();
    resp["success"] = user.transferAmount(to, amount, *m_db);
    return resp;
}

QJsonObject RequestHandler::handleViewDatabase(Session &session, const QJsonObject &req)
{
    Q_UNUSED(req);
    QJsonObject resp;
    resp["action"] = "viewBankDatabase";

    if (!session.loggedIn || !session.isAdmin) {
        resp["data"] = "[]";
        return resp;
    }

    Admin admin(session.username);
    resp["data"] = admin.viewBankDatabase(*m_db);
    return resp;
}

QJsonObject RequestHandler::handleCreateUser(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "createNewUser";

    if (!session.loggedIn || !session.isAdmin) {
        resp["success"] = false;
        return resp;
    }

    Admin admin(session.username);
    resp["success"] = admin.createNewUser(req["userData"].toString(), *m_db);
    return resp;
}

QJsonObject RequestHandler::handleDeleteUser(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "deleteUser";

    if (!session.loggedIn || !session.isAdmin) {
        resp["success"] = false;
        return resp;
    }

    Admin admin(session.username);
    std::string accountNumber = req["accountNumber"].toString().toStdString();
    resp["success"] = admin.deleteUser(accountNumber, *m_db);
    return resp;
}

QJsonObject RequestHandler::handleUpdateUser(Session &session, const QJsonObject &req)
{
    QJsonObject resp;
    resp["action"] = "updateUser";

    if (!session.loggedIn || !session.isAdmin) {
        resp["success"] = false;
        return resp;
    }

    Admin admin(session.username);
    std::string accountNumber = req["accountNumber"].toString().toStdString();
    resp["success"] = admin.updateUser(accountNumber, req["newData"].toString(), *m_db);
    return resp;
}