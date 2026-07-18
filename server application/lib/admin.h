#ifndef ADMIN_H
#define ADMIN_H
#include "user.h"

class Admin : public User
{
public:
    explicit Admin(const std::string &username);

    std::string getAccountNumberOf(const std::string &targetUsername, Database &db) const;

    QString viewBankDatabase(Database &db) const;
    int32_t viewBalance(const std::string &accountNumber, Database &db);
    QString viewTransactionHistory(const std::string &accountNumber, int count, Database &db);

    bool createNewUser(const QString &userDataJson, Database &db) const;
    bool deleteUser(const std::string &accountNumber, Database &db) const;
    bool updateUser(const std::string &accountNumber, const QString &newDataJson, Database &db) const;
};
#endif // ADMIN_H