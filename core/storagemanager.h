#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QString>
#include <QJsonArray>
#include <QList>
#include <optional>
#include "../models/user.h"
#include "../models/gamerecord.h"

class StorageManager
{
public:
    static StorageManager& instance();

    // ---- کاربران ----
    bool createUser(const User& user, QString& outError);
    bool usernameExists(const QString& username);
    bool phoneExists(const QString& phone);
    bool emailExists(const QString& email);
    std::optional<User> findByUsernameOrPhone(const QString& val);
    bool updatePassword(const QString& phone, const QString& newHash);
    User findById(int id);
    bool updateUser(const User& updatedUser, QString& outError);
    bool updateUser(const QString& oldUsername, const User& updatedUser);


    // ---- تاریخچه بازی‌ها ---- ← اینا رو اضافه کن
    bool saveGameRecord(const GameRecord& record);
    QList<GameRecord> getGameHistory(int userId,
                                     const QString& gameType = "");
    int getWinCount(int userId, const QString& gameType = "");
    int getLoseCount(int userId, const QString& gameType = "");
    int getDrawCount(int userId, const QString& gameType = "");

private:
    StorageManager() = default;
    QString    filePath() const;
    QJsonArray loadAll() const;
    bool       saveAll(const QJsonArray& arr);
    QString    getHistoryFilePath(); // ← اضافه کن
};

#endif // STORAGEMANAGER_H