#include "admin.h"
#include <QJsonDocument>
#include <QDebug>

Admin::Admin(const std::string &username) : User(username)
{
}

std::string Admin::getAccountNumberOf(const std::string &targetUsername, Database &db) const
{
    QJsonObject account = db.findAccountbyUsername(targetUsername);
    if (account.isEmpty()) {
        return "";
    }
    return account["accountNumber"].toString().toStdString();
}

QString Admin::viewBankDatabase(Database &db) const
{
    QJsonArray accounts = db.loadDB();
    QJsonDocument doc(accounts);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

int32_t Admin::viewBalance(const std::string &accountNumber, Database &db)
{
    QJsonObject account = db.findAccountbyNumber(accountNumber);
    if (account.isEmpty()) {
        return 0;
    }
    return account["balance"].toInt();
}

QString Admin::viewTransactionHistory(const std::string &accountNumber, int count, Database &db)
{
    return db.getTransactionHistory(accountNumber, count);
}

bool Admin::createNewUser(const QString &userDataJson, Database &db) const
{
    QJsonDocument doc = QJsonDocument::fromJson(userDataJson.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "createNewUser: invalid JSON";
        return false;
    }

    QJsonObject data = doc.object();

    if (!data.contains("username") || !data.contains("password") || !data.contains("accountNumber")) {
        qDebug() << "createNewUser: missing required fields";
        return false;
    }

    std::string username = data["username"].toString().toStdString();
    std::string password = data["password"].toString().toStdString();
    std::string accountNumber = data["accountNumber"].toString().toStdString();

    return db.addAccount(username, password, accountNumber);
}

bool Admin::deleteUser(const std::string &accountNumber, Database &db) const
{
    return db.deleteAccount(accountNumber);
}

bool Admin::updateUser(const std::string &accountNumber, const QString &newDataJson, Database &db) const
{
    QJsonDocument doc = QJsonDocument::fromJson(newDataJson.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "updateUser: invalid JSON";
        return false;
    }

    QJsonObject newData = doc.object();
    return db.updateAccount(accountNumber, newData);
}