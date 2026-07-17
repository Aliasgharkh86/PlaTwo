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

bool isValidPassword(const QString& password) {
    // بررسی طول حداقل ۸ کاراکتر
    if (password.length() < 8) {
        return false;
    }

    bool hasLetter = false;
    bool hasDigit = false;

    // بررسی وجود حداقل یک حرف و یک عدد
    for (QChar c : password) {
        if (c.isLetter()) hasLetter = true;
        if (c.isDigit()) hasDigit = true;
    }

    return hasLetter && hasDigit;
}

bool isValidPhone(const QString& phone) {
    // بررسی شماره موبایل ایران (۱۱ رقم که با 09 شروع می‌شود)
    QRegularExpression rx("^09\\d{9}$");
    QRegularExpressionMatch match = rx.match(phone);
    return match.hasMatch();
}

bool isValidEmail(const QString& email) {
    // بررسی ساده فرمت ایمیل
    QRegularExpression rx("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    QRegularExpressionMatch match = rx.match(email);
    return match.hasMatch();
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
        // یک پیام دیباگ ساده و امن (اختیاری)
        qDebug() << "Login failed: User not found.";
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

// ----------------------------------------------------------------------
// --- توابع به‌روزرسانی پروفایل (خطاهای کامپایل در این بخش برطرف شد) ---
// ----------------------------------------------------------------------

bool AuthManager::updateUserProfile(int userId, const QString& name, const QString& username,
                                    const QString& phone, const QString& email,
                                    const QString& password, QString& outError)
{
    StorageManager& storage = StorageManager::instance();

    // پیدا کردن کاربر فعلی با استفاده از آیدی
    User user = storage.findById(userId);
    if (user.id == 0) { // اگر کاربر پیدا نشود (فرض بر اینکه آیدی 0 نامعتبر است)
        outError = "کاربر مورد نظر یافت نشد.";
        return false;
    }

    // بررسی خالی نبودن فیلدهای اجباری
    if (name.isEmpty() || username.isEmpty() || phone.isEmpty() || email.isEmpty()) {
        outError = "پر کردن تمامی فیلدها (به جز رمز عبور) الزامی است.";
        return false;
    }

    // اعتبارسنجی فرمت ایمیل و شماره تلفن
    if (!isValidPhone(phone)) {
        outError = "فرمت شماره تلفن نامعتبر است.";
        return false;
    }
    if (!isValidEmail(email)) {
        outError = "فرمت ایمیل نامعتبر است.";
        return false;
    }

    // جایگذاری اطلاعات جدید
    user.name = name;
    user.username = username;
    user.phone = phone;
    user.email = email;

    // اگر کاربر رمز عبور جدیدی وارد کرده باشد، آن را چک و هش می‌کنیم
    if (!password.isEmpty()) {
        if (!isValidPassword(password)) {
            outError = "رمز عبور جدید به اندازه کافی قوی نیست (حداقل 8 کاراکتر، شامل عدد و حروف).";
            return false;
        }
        user.passwordHash = hashPassword(password);
    } // <--- این آکولاد جا افتاده بود و باعث خطاهای شما شده بود!

    // ارسال به StorageManager برای ذخیره نهایی
    return storage.updateUser(user, outError);
}

bool AuthManager::updateUserProfile(const QString& oldUsername, const QString& newName,
                                    const QString& newUsername, const QString& newPhone,
                                    const QString& newEmail, const QString& newPassword,
                                    QString& errorMsg)
{
    // ۱. بررسی خالی نبودن فیلدهای ضروری
    if (newName.trimmed().isEmpty()) {
        errorMsg = "نام نمی‌تواند خالی باشد.";
        return false;
    }
    if (newUsername.trimmed().isEmpty()) {
        errorMsg = "نام کاربری نمی‌تواند خالی باشد.";
        return false;
    }

    // ۲. اعتبارسنجی فرمت‌ها
    if (!validatePhone(newPhone.trimmed())) {
        errorMsg = "شماره تلفن معتبر نیست.";
        return false;
    }
    if (!validateEmail(newEmail.trimmed())) {
        errorMsg = "آدرس ایمیل معتبر نیست.";
        return false;
    }

    // اگر کاربر رمز جدیدی وارد کرده بود، آن را هم اعتبارسنجی می‌کنیم
    if (!newPassword.isEmpty() && !validatePassword(newPassword)) {
        errorMsg = "رمز عبور جدید باید حداقل ۸ کاراکتر و شامل حرف و عدد باشد.";
        return false;
    }

    // ۳. دریافت اطلاعات فعلی کاربر
    std::optional<User> optUser = StorageManager::instance().findByUsernameOrPhone(oldUsername);
    if (!optUser.has_value()) {
        errorMsg = "کاربر یافت نشد.";
        return false;
    }

    User currentUser = optUser.value();
    QString tNewUsername = newUsername.trimmed();
    QString tNewPhone = newPhone.trimmed();
    QString tNewEmail = newEmail.trimmed();

    // ۴. بررسی تکراری نبودن اطلاعات جدید (استفاده از توابع صحیح به جای userExists)
    if (tNewUsername.compare(currentUser.username, Qt::CaseInsensitive) != 0 &&
        StorageManager::instance().usernameExists(tNewUsername)) {
        errorMsg = "این نام کاربری قبلاً ثبت شده است.";
        return false;
    }
    if (tNewPhone != currentUser.phone && StorageManager::instance().phoneExists(tNewPhone)) {
        errorMsg = "این شماره تلفن قبلاً ثبت شده است.";
        return false;
    }
    if (tNewEmail.compare(currentUser.email, Qt::CaseInsensitive) != 0 &&
        StorageManager::instance().emailExists(tNewEmail)) {
        errorMsg = "این ایمیل قبلاً ثبت شده است.";
        return false;
    }

    // ۵. اعمال تغییرات
    currentUser.name = newName.trimmed();
    currentUser.username = tNewUsername;
    currentUser.phone = tNewPhone;
    currentUser.email = tNewEmail;

    // اگر رمز عبور جدیدی وارد شده بود، هش جدید تولید می‌شود
    if (!newPassword.isEmpty()) {
        currentUser.passwordHash = hashPassword(newPassword);
    }

    // ۶. ذخیره در دیتابیس
    if (StorageManager::instance().updateUser(oldUsername, currentUser)) {
        return true;
    } else {
        errorMsg = "خطا در ذخیره اطلاعات.";
        return false;
    }
}
