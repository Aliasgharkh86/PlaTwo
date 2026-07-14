#include "storagemanager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QCoreApplication>

StorageManager& StorageManager::instance()
{
    static StorageManager s;
    return s;
}

QString StorageManager::filePath() const
{
    // این دستور فایل را دقیقاً در کنار فایل اجرایی برنامه می‌سازد
    QString dir = QCoreApplication::applicationDirPath();

    QString fullPath = dir + "/users.json";
    qDebug() << "Storage path:" << fullPath;
    return fullPath;
}

QJsonArray StorageManager::loadAll() const
{
    QFile f(filePath());
    if (!f.open(QIODevice::ReadOnly)) return QJsonArray();
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    return doc.isArray() ? doc.array() : QJsonArray();
}

bool StorageManager::saveAll(const QJsonArray& arr)
{
    QFile f(filePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    f.write(QJsonDocument(arr).toJson());
    return true;
}

bool StorageManager::usernameExists(const QString& username)
{
    for (const auto& v : loadAll())
        if (v.toObject()["username"].toString().toLower()
            == username.toLower()) return true;
    return false;
}

bool StorageManager::phoneExists(const QString& phone)
{
    for (const auto& v : loadAll())
        if (v.toObject()["phone"].toString() == phone) return true;
    return false;
}

bool StorageManager::emailExists(const QString& email)
{
    for (const auto& v : loadAll())
        if (v.toObject()["email"].toString().toLower()
            == email.toLower()) return true;
    return false;
}

bool StorageManager::createUser(const User& user, QString& outError)
{
    if (usernameExists(user.username)) {
        outError = "این نام کاربری قبلاً ثبت شده است."; return false;
    }
    if (phoneExists(user.phone)) {
        outError = "این شماره تلفن قبلاً ثبت شده است."; return false;
    }
    if (emailExists(user.email)) {
        outError = "این ایمیل قبلاً ثبت شده است."; return false;
    }

    QJsonArray arr = loadAll();
    QJsonObject obj;
    obj["name"]     = user.name;
    obj["username"] = user.username;
    obj["phone"]    = user.phone;
    obj["email"]    = user.email;
    obj["password"] = user.passwordHash;
    arr.append(obj);
    return saveAll(arr);
}

std::optional<User> StorageManager::findByUsernameOrPhone(const QString& val)
{
    // مقدار ورودی کاربر را برای مقایسه نام کاربری کوچک می‌کنیم
    QString searchVal = val.toLower();

    for (const auto& v : loadAll()) {
        QJsonObject obj = v.toObject();

        // نام کاربری ذخیره شده را هم موقع مقایسه کوچک می‌کنیم
        if (obj["username"].toString().toLower() == searchVal ||
            obj["phone"].toString()    == val) {

            User u;
            u.name         = obj["name"].toString();
            u.username     = obj["username"].toString();
            u.phone        = obj["phone"].toString();
            u.email        = obj["email"].toString();
            u.passwordHash = obj["password"].toString();
            return u;
        }
    }
    return std::nullopt;
}

bool StorageManager::updatePassword(const QString& phone,
                                    const QString& newHash)
{
    QJsonArray arr = loadAll();
    for (int i = 0; i < arr.size(); i++) {
        QJsonObject obj = arr[i].toObject();
        if (obj["phone"].toString() == phone) {
            obj["password"] = newHash;
            arr[i] = obj;
            return saveAll(arr);
        }
    }
    return false;
}
QString StorageManager::getHistoryFilePath()
{
    return QStandardPaths::writableLocation(
               QStandardPaths::AppDataLocation) + "/history.json";
}

bool StorageManager::saveGameRecord(const GameRecord& record)
{
    QFile file(getHistoryFilePath());
    QJsonArray arr;

    if (file.open(QIODevice::ReadOnly)) {
        arr = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    QJsonObject obj;
    obj["id"]             = arr.size() + 1;
    obj["userId"]         = record.userId;
    obj["gameType"]       = record.gameType;
    obj["opponent"]       = record.opponentUsername;
    obj["result"]         = record.result == GameResult::WIN  ? "win"  :
                        record.result == GameResult::LOSE ? "lose" : "draw";
    obj["myScore"]        = record.myScore;
    obj["opponentScore"]  = record.opponentScore;
    obj["role"]           = record.role;
    obj["playedAt"]       = record.playedAt.toString(Qt::ISODate);
    arr.append(obj);

    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(arr).toJson());
    return true;
}

QList<GameRecord> StorageManager::getGameHistory(int userId,
                                                 const QString& gameType)
{
    QList<GameRecord> records;
    QFile file(getHistoryFilePath());
    if (!file.open(QIODevice::ReadOnly)) return records;

    QJsonArray arr = QJsonDocument::fromJson(file.readAll()).array();

    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        if (obj["userId"].toInt() != userId) continue;
        if (!gameType.isEmpty() &&
            obj["gameType"].toString() != gameType) continue;

        GameRecord r;
        r.id               = obj["id"].toInt();
        r.userId           = obj["userId"].toInt();
        r.gameType         = obj["gameType"].toString();
        r.opponentUsername = obj["opponent"].toString();
        QString res        = obj["result"].toString();
        r.result           = res == "win"  ? GameResult::WIN  :
                       res == "lose" ? GameResult::LOSE :
                       GameResult::DRAW;
        r.myScore          = obj["myScore"].toInt();
        r.opponentScore    = obj["opponentScore"].toInt();
        r.role             = obj["role"].toString();
        r.playedAt         = QDateTime::fromString(
            obj["playedAt"].toString(), Qt::ISODate);
        records.append(r);
    }
    return records;
}

int StorageManager::getWinCount(int userId, const QString& gameType)
{
    int count = 0;
    for (const auto& r : getGameHistory(userId, gameType))
        if (r.result == GameResult::WIN) count++;
    return count;
}

int StorageManager::getLoseCount(int userId, const QString& gameType)
{
    int count = 0;
    for (const auto& r : getGameHistory(userId, gameType))
        if (r.result == GameResult::LOSE) count++;
    return count;
}

int StorageManager::getDrawCount(int userId, const QString& gameType)
{
    int count = 0;
    for (const auto& r : getGameHistory(userId, gameType))
        if (r.result == GameResult::DRAW) count++;
    return count;
}

