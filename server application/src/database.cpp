#include "database.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>
#include <QDateTime>
#include <QDebug>

Database::Database(const QString &filePath) : QObject(nullptr), m_filePath(filePath)
{
}

QJsonArray Database::loadDB()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open" << m_filePath;
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return QJsonArray();
    }

    return doc.object()["accounts"].toArray();
}

QJsonArray Database::loadAdmins()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open" << m_filePath;
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return QJsonArray();
    }

    return doc.object()["admins"].toArray();
}

void Database::saveDB(const QJsonArray &accounts)
{
    QFile readFile(m_filePath);
    QJsonObject root;
    if (readFile.open(QIODevice::ReadOnly)) {
        QJsonDocument existingDoc = QJsonDocument::fromJson(readFile.readAll());
        readFile.close();
        if (existingDoc.isObject()) {
            root = existingDoc.object();
        }
    }

    root["accounts"] = accounts;

    QJsonDocument doc(root);
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not write to" << m_filePath;
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void Database::saveAdmins(const QJsonArray &admins)
{
    QFile readFile(m_filePath);
    QJsonObject root;
    if (readFile.open(QIODevice::ReadOnly)) {
        QJsonDocument existingDoc = QJsonDocument::fromJson(readFile.readAll());
        readFile.close();
        if (existingDoc.isObject()) {
            root = existingDoc.object();
        }
    }

    root["admins"] = admins;

    QJsonDocument doc(root);
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not write to" << m_filePath;
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

bool Database::verifyAccountNumber(const std::string &accountNumber)
{
    QJsonArray accounts = loadDB();
    for (int i = 0; i < accounts.size(); i++) {
        QJsonObject account = accounts[i].toObject();
        if (account["accountNumber"].toString().toStdString() == accountNumber) {
            return true;
        }
    }
    return false;
}

bool Database::verifyUsername(const std::string &userName)
{
    QJsonArray accounts = loadDB();
    for (int i = 0; i < accounts.size(); i++) {
        QJsonObject account = accounts[i].toObject();
        if (account["username"].toString().toStdString() == userName) {
            return true;
        }
    }
    return false;
}

QJsonObject Database::findAccountbyUsername(const std::string &userName)
{
    QJsonArray accounts = loadDB();
    for (int i = 0; i < accounts.size(); i++) {
        QJsonObject account = accounts[i].toObject();
        if (account["username"].toString().toStdString() == userName) {
            return account;
        }
    }
    return QJsonObject();
}

QJsonObject Database::findAccountbyNumber(const std::string &accountNumber)
{
    QJsonArray accounts = loadDB();
    for (int i = 0; i < accounts.size(); i++) {
        QJsonObject account = accounts[i].toObject();
        if (account["accountNumber"].toString().toStdString() == accountNumber) {
            return account;
        }
    }
    return QJsonObject();
}

QJsonObject Database::findAdminByUsername(const std::string &userName)
{
    QJsonArray admins = loadAdmins();
    for (int i = 0; i < admins.size(); i++) {
        QJsonObject admin = admins[i].toObject();
        if (admin["username"].toString().toStdString() == userName) {
            return admin;
        }
    }
    return QJsonObject();
}

bool Database::updateAccount(const std::string &accountNumber, const QJsonObject &newData)
{
    QJsonArray accounts = loadDB();
    for (int i = 0; i < accounts.size(); i++) {
        QJsonObject account = accounts[i].toObject();
        if (account["accountNumber"].toString().toStdString() == accountNumber) {
            for (const QString &key : newData.keys()) {
                account[key] = newData[key];
            }
            accounts[i] = account;
            saveDB(accounts);
            return true;
        }
    }
    return false;
}

bool Database::addAccount(const std::string &username, const std::string &password, const std::string &accountNumber)
{
    if (verifyAccountNumber(accountNumber) || verifyUsername(username)) {
        return false;
    }

    QJsonArray accounts = loadDB();

    QJsonObject newAccount;
    newAccount["username"] = QString::fromStdString(username);
    newAccount["password"] = QString::fromStdString(password);
    newAccount["accountNumber"] = QString::fromStdString(accountNumber);
    newAccount["balance"] = 0;
    newAccount["transactions"] = QJsonArray();

    accounts.append(newAccount);
    saveDB(accounts);
    return true;
}

bool Database::deleteAccount(const std::string &accountNumber)
{
    QJsonArray accounts = loadDB();
    for (int i = 0; i < accounts.size(); i++) {
        QJsonObject account = accounts[i].toObject();
        if (account["accountNumber"].toString().toStdString() == accountNumber) {
            accounts.removeAt(i);
            saveDB(accounts);
            return true;
        }
    }
    return false;
}

void Database::appendTransaction(const std::string &accountNumber, int32_t amount)
{
    QJsonArray accounts = loadDB();
    for (int i = 0; i < accounts.size(); i++) {
        QJsonObject account = accounts[i].toObject();
        if (account["accountNumber"].toString().toStdString() == accountNumber) {
            QJsonArray transactions = account["transactions"].toArray();

            QJsonObject entry;
            entry["date"] = QDateTime::currentDateTime().toString("dd-MM-yyyy");
            entry["Amount"] = amount;
            transactions.append(entry);

            account["transactions"] = transactions;
            accounts[i] = account;
            saveDB(accounts);
            return;
        }
    }
}

QString Database::getTransactionHistory(const std::string &accountNumber, int count)
{
    QJsonArray accounts = loadDB();
    for (int i = 0; i < accounts.size(); i++) {
        QJsonObject account = accounts[i].toObject();
        if (account["accountNumber"].toString().toStdString() == accountNumber) {
            QJsonArray transactions = account["transactions"].toArray();

            QJsonArray result;
            int taken = 0;
            for (int j = transactions.size() - 1; j >= 0 && taken < count; j--, taken++) {
                result.append(transactions[j]);
            }

            QJsonDocument doc(result);
            return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        }
    }
    return "[]";
}

bool Database::login(const std::string &username, const std::string &password, bool &outIsAdmin)
{

    QJsonObject account = findAccountbyUsername(username);
    if (!account.isEmpty()) {
        if (account["password"].toString().toStdString() == password) {
            outIsAdmin = false;
            return true;
        }
        return false;
    }


    QJsonObject admin = findAdminByUsername(username);
    if (!admin.isEmpty()) {
        if (admin["password"].toString().toStdString() == password) {
            outIsAdmin = true;
            return true;
        }
        return false;
    }

    return false;
}