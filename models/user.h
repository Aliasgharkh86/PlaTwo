#ifndef USER_H
#define USER_H

#include <QString>

// Simple data model representing a registered user.
// Both Person A (Login) and Person B (Signup/ForgotPassword) use this same struct,
// so do not change field names without telling your teammate.
struct User
{
    int     id = -1;          // database row id, -1 means "not saved / not found"
    QString name;
    QString username;
    QString phone;
    QString email;
    QString passwordHash;     // NEVER store plain text password here

    bool isValid() const { return id != -1; }
};

#endif // USER_H