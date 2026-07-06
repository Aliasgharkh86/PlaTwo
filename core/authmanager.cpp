#include "authmanager.h"
#include "storagemanager.h"
#include <QCryptographicHash>
#include <QRegularExpression>

AuthManager& AuthManager::instance()
{
    static AuthManager s;
    return s;
}
QString AuthManager::hashPassword(const QString& plain)
{
    return QString(QCryptographicHash::hash(
                       plain.toUtf8(),
                       QCryptographicHash::Sha256
                       ).toHex());
}
bool AuthManager::validatePhone(const QString& phone)
{
    // فرمت ایرانی: شروع با 09، جمعاً 11 رقم
    QRegularExpression rx("^09[0-9]{9}$");
    return rx.match(phone.trimmed()).hasMatch();
}

bool AuthManager::validateEmail(const QString& email)
{
    QRegularExpression rx(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)");
    return rx.match(email.trimmed()).hasMatch();
}

bool AuthManager::validatePassword(const QString& pass)
{
    if (pass.length() < 8) return false;
    bool hasLetter = false, hasDigit = false;
    for (const QChar& c : pass) {
        if (c.isLetter()) hasLetter = true;
        if (c.isDigit())  hasDigit  = true;
    }
    return hasLetter && hasDigit;
}

bool AuthManager::resetPassword(const QString& phone,
                                const QString& newPassword,
                                QString& outError)
{
    // چک معتبر بودن شماره تلفن
    if (!validatePhone(phone)) {
        outError = "شماره تلفن معتبر نیست.";
        return false;
    }

    // چک قوی بودن رمز جدید
    if (!validatePassword(newPassword)) {
        outError = "رمز عبور باید حداقل ۸ کاراکتر و شامل حرف و عدد باشد.";
        return false;
    }

    // چک وجود کاربر با این شماره
    if (!StorageManager::instance().phoneExists(phone)) {
        outError = "کاربری با این شماره تلفن یافت نشد.";
        return false;
    }

    // تغییر رمز توی دیتابیس/فایل
    bool ok = StorageManager::instance()
                  .updatePassword(phone, hashPassword(newPassword));

    if (!ok) {
        outError = "خطا در تغییر رمز عبور.";
        return false;
    }

    outError.clear();
    return true;
}

User AuthManager::login(const QString& usernameOrPhone,
                        const QString& password,
                        QString& outError)
{
    if (usernameOrPhone.trimmed().isEmpty()) {
        outError = "نام کاربری یا شماره تلفن را وارد کنید.";
        return User{};
    }
    if (password.isEmpty()) {
        outError = "رمز عبور را وارد کنید.";
        return User{};
    }

    auto result = StorageManager::instance()
                      .findByUsernameOrPhone(usernameOrPhone.trimmed());
    qDebug() << "Found user:" << result.has_value();

    if (!result.has_value()) {
        outError = "کاربری با این مشخصات یافت نشد.";
        qDebug() << "Stored hash:" << result->passwordHash;
        qDebug() << "Input hash:" << hashPassword(password);
        return User{};
    }

    User u = result.value();
    if (u.passwordHash != hashPassword(password)) {
        outError = "رمز عبور اشتباه است.";
        return User{};
    }

    return u;   // login موفق — User معتبر برمی‌گرده
}

bool AuthManager::signUp(const QString& name,
                         const QString& username,
                         const QString& phone,
                         const QString& email,
                         const QString& password,
                         QString& outError)
{
    // --- اعتبارسنجی ---
    if (name.trimmed().isEmpty()) {
        outError = "نام را وارد کنید."; return false;
    }
    if (username.trimmed().isEmpty()) {
        outError = "نام کاربری را وارد کنید."; return false;
    }
    if (!validatePhone(phone)) {
        outError = "شماره تلفن معتبر نیست. (مثال: 09123456789)"; return false;
    }
    if (!validateEmail(email)) {
        outError = "آدرس ایمیل معتبر نیست."; return false;
    }
    if (!validatePassword(password)) {
        outError = "رمز عبور باید حداقل ۸ کاراکتر و شامل حرف و عدد باشد.";
        return false;
    }

    auto& db = StorageManager::instance();
    if (db.usernameExists(username.trimmed())) {
        outError = "این نام کاربری قبلاً ثبت شده است."; return false;
    }
    if (db.phoneExists(phone.trimmed())) {
        outError = "این شماره تلفن قبلاً ثبت شده است."; return false;
    }
    if (db.emailExists(email.trimmed())) {
        outError = "این ایمیل قبلاً ثبت شده است."; return false;
    }

    // --- ذخیره کاربر ---
    User u;
    u.name         = name.trimmed();
    u.username     = username.trimmed();
    u.phone        = phone.trimmed();
    u.email        = email.trimmed();
    u.passwordHash = hashPassword(password);

    outError.clear();
    return db.createUser(u, outError);

}

