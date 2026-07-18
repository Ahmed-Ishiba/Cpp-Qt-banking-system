#include "user.h"
#include <QDebug>

User::User(const std::string &username, Database &db) : QObject(nullptr), m_username(username)
{
    QJsonObject account = db.findAccountbyUsername(username);
    if (!account.isEmpty()) {
        m_accountNumber = account["accountNumber"].toString().toStdString();
    } else {
        qDebug() << "User not found for username:" << QString::fromStdString(username);
        m_accountNumber = "";
    }
}

User::User(const std::string &username, const std::string &accountNumber)
    : QObject(nullptr), m_username(username), m_accountNumber(accountNumber)
{
}

User::User(const std::string &username)
    : QObject(nullptr), m_username(username), m_accountNumber("")
{
}

std::string User::getAccountNumber()
{
    return m_accountNumber;
}

int32_t User::viewBalance(Database &db)
{
    QJsonObject account = db.findAccountbyNumber(m_accountNumber);
    if (account.isEmpty()) {
        return 0;
    }
    return account["balance"].toInt();
}

bool User::makeTransaction(int32_t amount, Database &db)
{
    int32_t currentBalance = viewBalance(db);

    if (amount < 0) {
        if (currentBalance <= 0 || currentBalance < -amount) {
            return false;
        }
    }

    int32_t newBalance = currentBalance + amount;

    QJsonObject newData;
    newData["balance"] = newBalance;

    if (!db.updateAccount(m_accountNumber, newData)) {
        return false;
    }

    db.appendTransaction(m_accountNumber, amount);
    return true;
}

bool User::transferAmount(const std::string &other_accountNumber, int32_t amount, Database &db)
{
    if (amount <= 0) {
        return false;
    }

    int32_t fromBalance = viewBalance(db);
    if (fromBalance < amount) {
        return false;
    }

    QJsonObject toAccount = db.findAccountbyNumber(other_accountNumber);
    if (toAccount.isEmpty()) {
        return false;
    }
    int32_t toBalance = toAccount["balance"].toInt();

    QJsonObject fromData;
    fromData["balance"] = fromBalance - amount;
    if (!db.updateAccount(m_accountNumber, fromData)) {
        return false;
    }

    QJsonObject toData;
    toData["balance"] = toBalance + amount;
    if (!db.updateAccount(other_accountNumber, toData)) {
        return false;
    }

    db.appendTransaction(m_accountNumber, -amount);
    db.appendTransaction(other_accountNumber, amount);

    return true;
}

QString User::viewTransactionHistory(int count, Database &db)
{
    return db.getTransactionHistory(m_accountNumber, count);
}