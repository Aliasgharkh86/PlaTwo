#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QString>
#include <QJsonArray>
#include <optional>
#include "../models/user.h"

class StorageManager
{
public:
    static StorageManager& instance();

    bool createUser(const User& user, QString& outError);
    bool usernameExists(const QString& username);
    bool phoneExists(const QString& phone);
    bool emailExists(const QString& email);
    std::optional<User> findByUsernameOrPhone(const QString& val);
    bool updatePassword(const QString& phone, const QString& newHash);
    User findById(int id);
    bool updateUser(const User& updatedUser, QString& outError);
    bool updateUser(const QString& oldUsername, const User& updatedUser);


private:
    StorageManager() = default;
    QString    filePath() const;
    QJsonArray loadAll() const;
    bool       saveAll(const QJsonArray& arr);
};

#endif