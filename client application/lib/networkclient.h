#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QJsonObject>

class NetworkClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit NetworkClient(QObject *parent = nullptr);
    bool isConnected() const;

    Q_INVOKABLE void connectToServer(const QString &host, int port);

    Q_INVOKABLE void login(const QString &username, const QString &password);
    Q_INVOKABLE void getAccountNumber(const QString &username);
    Q_INVOKABLE void getAccountNumberAdmin(const QString &username);
    Q_INVOKABLE void viewBalance(const QString &accountNumber);
    Q_INVOKABLE void viewTransactionHistory(const QString &accountNumber, int count);
    Q_INVOKABLE void makeTransaction(int amount);
    Q_INVOKABLE void transferAmount(const QString &toAccountNumber, int amount);
    Q_INVOKABLE void viewBankDatabase();
    Q_INVOKABLE void createNewUser(const QString &userDataJson);
    Q_INVOKABLE void deleteUser(const QString &accountNumber);
    Q_INVOKABLE void updateUser(const QString &accountNumber, const QString &newDataJson);

signals:
    void connectedChanged();
    void connectionFailed(const QString &error);

    void loginResult(bool success, bool isAdmin);
    void accountNumberResult(QString accountNumber);
    void balanceResult(bool success, int balance);
    void historyResult(QString historyJson);
    void transactionResult(bool success);
    void transferResult(bool success);
    void bankDatabaseResult(QString data);
    void createUserResult(bool success);
    void deleteUserResult(bool success);
    void updateUserResult(bool success);

private slots:
    void handleReadyRead();
    void handleConnected();
    void handleError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket;
    QByteArray m_buffer;

    void sendMessage(const QJsonObject &message);
    void processMessage(const QJsonObject &message);
};