#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QString>
#include "../models/user.h"

class AuthManager
{
private:
    QString hashPassword(const QString& plain);
public:
    static AuthManager& instance();


    User login(const QString& usernameOrPhone,
               const QString& password,
               QString& outError);

    bool signUp(const QString& name,
                const QString& username,
                const QString& phone,
                const QString& email,
                const QString& password,
                QString& outError);

    bool resetPassword(const QString& phone,
                       const QString& newPassword,
                       QString& outError);

    bool validatePhone(const QString& phone);
    bool validateEmail(const QString& email);
    bool validatePassword(const QString& pass);
    bool updateUserProfile(int userId, const QString& name, const QString& username,
                                        const QString& phone, const QString& email,
                                         const QString& password, QString& outError);
    bool updateUserProfile(const QString& oldUsername, const QString& newName, const QString& newUsername, const QString& newPhone, const QString& newEmail, const QString& newPassword, QString& errorMsg);

private:
    AuthManager() = default;
};

#endif // AUTHMANAGER_H
