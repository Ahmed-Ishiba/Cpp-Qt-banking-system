#ifndef DATABASE_H
#define DATABASE_H
#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <string>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(const QString &filePath);
    QJsonArray loadDB();
    QJsonArray loadAdmins();

    bool verifyAccountNumber(const std::string &accountNumber);
    bool verifyUsername(const std::string &userName);

    QJsonObject findAccountbyUsername(const std::string &userName);
    QJsonObject findAccountbyNumber(const std::string &accountNumber);
    QJsonObject findAdminByUsername(const std::string &userName);

    bool updateAccount(const std::string &accountNumber, const QJsonObject &newData);
    bool addAccount(const std::string &username, const std::string &password, const std::string &accountNumber);
    bool deleteAccount(const std::string &accountNumber);

    void appendTransaction(const std::string &accountNumber, int32_t amount);
    QString getTransactionHistory(const std::string &accountNumber, int count);

    bool login(const std::string &username, const std::string &password, bool &outIsAdmin);

private:
    QString m_filePath;
    void saveDB(const QJsonArray &accounts);
    void saveAdmins(const QJsonArray &admins);
signals:
};
#endif