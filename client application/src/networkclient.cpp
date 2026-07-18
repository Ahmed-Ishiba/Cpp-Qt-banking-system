#include "networkclient.h"
#include <QJsonDocument>
#include <QDebug>

NetworkClient::NetworkClient(QObject *parent) : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::handleReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::handleConnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkClient::handleError);
}

bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::connectToServer(const QString &host, int port)
{
    m_socket->connectToHost(host, port);
}

void NetworkClient::handleConnected()
{
    emit connectedChanged();
}

void NetworkClient::sendMessage(const QJsonObject &message)
{
    QJsonDocument doc(message);
    m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void NetworkClient::login(const QString &username, const QString &password)
{
    QJsonObject msg{{"action", "login"}, {"username", username}, {"password", password}};
    sendMessage(msg);
}

void NetworkClient::getAccountNumber(const QString &username)
{
    QJsonObject msg{{"action", "getAccountNumber"}, {"username", username}};
    sendMessage(msg);
}

void NetworkClient::getAccountNumberAdmin(const QString &username)
{
    QJsonObject msg{{"action", "getAccountNumberAdmin"}, {"username", username}};
    sendMessage(msg);
}

void NetworkClient::viewBalance(const QString &accountNumber)
{
    QJsonObject msg{{"action", "viewBalance"}, {"accountNumber", accountNumber}};
    sendMessage(msg);
}

void NetworkClient::viewTransactionHistory(const QString &accountNumber, int count)
{
    QJsonObject msg{{"action", "viewTransactionHistory"}, {"accountNumber", accountNumber}, {"count", count}};
    sendMessage(msg);
}

void NetworkClient::makeTransaction(int amount)
{
    QJsonObject msg{{"action", "makeTransaction"}, {"amount", amount}};
    sendMessage(msg);
}

void NetworkClient::transferAmount(const QString &toAccountNumber, int amount)
{
    QJsonObject msg{{"action", "transferAmount"}, {"toAccountNumber", toAccountNumber}, {"amount", amount}};
    sendMessage(msg);
}

void NetworkClient::viewBankDatabase()
{
    QJsonObject msg{{"action", "viewBankDatabase"}};
    sendMessage(msg);
}

void NetworkClient::createNewUser(const QString &userDataJson)
{
    QJsonObject msg{{"action", "createNewUser"}, {"userData", userDataJson}};
    sendMessage(msg);
}

void NetworkClient::deleteUser(const QString &accountNumber)
{
    QJsonObject msg{{"action", "deleteUser"}, {"accountNumber", accountNumber}};
    sendMessage(msg);
}

void NetworkClient::updateUser(const QString &accountNumber, const QString &newDataJson)
{
    QJsonObject msg{{"action", "updateUser"}, {"accountNumber", accountNumber}, {"newData", newDataJson}};
    sendMessage(msg);
}

void NetworkClient::handleReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QByteArray line = m_buffer.left(idx);
        m_buffer.remove(0, idx + 1);

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isObject()) {
            processMessage(doc.object());
        }
    }
}

void NetworkClient::processMessage(const QJsonObject &message)
{
    QString action = message["action"].toString();

    if (action == "login") {
        emit loginResult(message["success"].toBool(), message["isAdmin"].toBool());
    }
    else if (action == "getAccountNumber" || action == "getAccountNumberAdmin") {
        emit accountNumberResult(message["accountNumber"].toString());
    }
    else if (action == "viewBalance") {
        emit balanceResult(message["success"].toBool(), message["balance"].toInt());
    }
    else if (action == "viewTransactionHistory") {
        emit historyResult(message["history"].toString());
    }
    else if (action == "makeTransaction") {
        emit transactionResult(message["success"].toBool());
    }
    else if (action == "transferAmount") {
        emit transferResult(message["success"].toBool());
    }
    else if (action == "viewBankDatabase") {
        emit bankDatabaseResult(message["data"].toString());
    }
    else if (action == "createNewUser") {
        emit createUserResult(message["success"].toBool());
    }
    else if (action == "deleteUser") {
        emit deleteUserResult(message["success"].toBool());
    }
    else if (action == "updateUser") {
        emit updateUserResult(message["success"].toBool());
    }
}

void NetworkClient::handleError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit connectionFailed(m_socket->errorString());
}