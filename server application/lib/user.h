#ifndef USER_H
#define USER_H
#include <QObject>
#include "database.h"

class User : public QObject
{
    Q_OBJECT
public:
    explicit User(const std::string &username, Database &db);
    User(const std::string &username, const std::string &accountNumber);

    std::string getAccountNumber();
    int32_t viewBalance(Database &db);
    bool makeTransaction(int32_t amount, Database &db);
    bool transferAmount(const std::string &other_accountNumber, int32_t amount, Database &db);
    QString viewTransactionHistory(int count, Database &db);

protected:
    std::string m_username;
    std::string m_accountNumber;

    explicit User(const std::string &username);

signals:
};
#endif // USER_H