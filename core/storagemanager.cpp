#include "storagemanager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

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
        if (v.toObject()["username"].toString().toLower() == username.toLower())
            return true;
    return false;
}

bool StorageManager::phoneExists(const QString& phone)
{
    for (const auto& v : loadAll())
        if (v.toObject()["phone"].toString() == phone)
            return true;
    return false;
}

bool StorageManager::emailExists(const QString& email)
{
    for (const auto& v : loadAll())
        if (v.toObject()["email"].toString().toLower() == email.toLower())
            return true;
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
    obj["id"]       = user.id; // اضافه کردن آیدی برای سازگاری با بقیه توابع
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
    QString searchVal = val.toLower();

    for (const auto& v : loadAll()) {
        QJsonObject obj = v.toObject();

        if (obj["username"].toString().toLower() == searchVal ||
            obj["phone"].toString()    == val) {

            User u;
            u.id           = obj["id"].toInt(); // خواندن آیدی
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

bool StorageManager::updatePassword(const QString& phone, const QString& newHash)
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

User StorageManager::findById(int id)
{
    QJsonArray users = loadAll();
    for (const QJsonValue& value : users) {
        QJsonObject obj = value.toObject();
        if (obj["id"].toInt() == id) {
            User user;
            user.id = obj["id"].toInt();
            user.name = obj["name"].toString();
            user.username = obj["username"].toString();
            user.phone = obj["phone"].toString();
            user.email = obj["email"].toString();
            user.passwordHash = obj["password"].toString(); // اصلاح شد به password
            return user;
        }
    }
    return User(); // Return an invalid user if not found
}

// Updates an existing user's data in the JSON file
bool StorageManager::updateUser(const User& updatedUser, QString& outError)
{
    QJsonArray users = loadAll();
    int userIndex = -1;

    // First, check for uniqueness constraints against other users
    for (int i = 0; i < users.size(); ++i) {
        QJsonObject obj = users[i].toObject();
        if (obj["id"].toInt() == updatedUser.id) {
            userIndex = i;
            continue; // Skip checking against the user itself
        }
        if (obj["username"].toString().toLower() == updatedUser.username.toLower()) {
            outError = "This username is already taken by another user.";
            return false;
        }
        if (obj["email"].toString().toLower() == updatedUser.email.toLower()) {
            outError = "This email is already taken by another user.";
            return false;
        }
        if (obj["phone"].toString() == updatedUser.phone) {
            outError = "This phone number is already taken by another user.";
            return false;
        }
    }

    if (userIndex == -1) {
        outError = "User to update was not found.";
        return false;
    }

    // Create a new JSON object for the updated user
    QJsonObject updatedObj;
    updatedObj["id"] = updatedUser.id;
    updatedObj["name"] = updatedUser.name;
    updatedObj["username"] = updatedUser.username;
    updatedObj["phone"] = updatedUser.phone;
    updatedObj["email"] = updatedUser.email;
    updatedObj["password"] = updatedUser.passwordHash; // اصلاح شد به password

    // Replace the old user object with the new one
    users[userIndex] = updatedObj;

    // Save the modified array back to the file
    // خطای کامپایل در این قسمت برطرف شد
    if (!saveAll(users)) {
        outError = "Failed to save data.";
        return false;
    }

    return true;
} // <--- خطای کامپایل (نبود آکولاد بسته) در اینجا برطرف شد

bool StorageManager::updateUser(const QString& oldUsername, const User& updatedUser)
{
    QJsonArray users = loadAll();
    bool found = false;

    for (int i = 0; i < users.size(); ++i) {
        QJsonObject userObj = users[i].toObject();
        // پیدا کردن کاربر بر اساس نام کاربری قبلی
        if (userObj["username"].toString().compare(oldUsername, Qt::CaseInsensitive) == 0) {
            userObj["name"] = updatedUser.name;
            userObj["username"] = updatedUser.username;
            userObj["phone"] = updatedUser.phone;
            userObj["email"] = updatedUser.email;
            userObj["password"] = updatedUser.passwordHash; // هش رمز عبور

            users[i] = userObj;
            found = true;
            break;
        }
    }

    if (found) {
        return saveAll(users);
    }
    return false;
}
