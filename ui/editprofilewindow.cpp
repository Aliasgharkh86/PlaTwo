#include "editprofilewindow.h"
#include "ui_editprofilewindow.h"
#include "core/authmanager.h" // برای استفاده از تابع updateUserProfile
#include <QMessageBox>

EditProfileWindow::EditProfileWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditProfileWindow)
{
    ui->setupUi(this);
    ui->errorLabel->clear();
}

EditProfileWindow::~EditProfileWindow()
{
    delete ui;
}

void EditProfileWindow::setCurrentUser(const User& user)
{
    m_currentUser = user;

    // پر کردن فیلدها با اطلاعات کاربری که از قبل ثبت شده است
    ui->nameLineEdit->setText(user.name);
    ui->usernameLineEdit->setText(user.username);
    ui->phoneLineEdit->setText(user.phone);
    ui->emailLineEdit->setText(user.email);

    // رمز عبور را خالی می‌گذاریم. اگر کاربر نخواست آن را تغییر دهد، خالی بماند
    ui->passwordLineEdit->clear();
    ui->passwordLineEdit->setPlaceholderText("برای عدم تغییر، خالی بگذارید");
}

void EditProfileWindow::on_backBtn_clicked()
{
    // ارسال سیگنال بازگشت و بستن پنجره
    emit backRequested();
    this->close();
}

void EditProfileWindow::on_saveBtn_clicked()
{
    ui->errorLabel->clear();

    // گرفتن مقادیر جدید از فرم
    QString name = ui->nameLineEdit->text().trimmed();
    QString username = ui->usernameLineEdit->text().trimmed();
    QString phone = ui->phoneLineEdit->text().trimmed();
    QString email = ui->emailLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text(); // می‌تواند خالی باشد

    QString errorMsg;

    // فراخوانی تابع updateUserProfile از کلاس AuthManager
    // نکته: ما m_currentUser.username (نام کاربری قدیمی) را می‌دهیم تا AuthManager بداند کدام کاربر را باید ویرایش کند
    bool success = AuthManager::instance().updateUserProfile(
        m_currentUser.username, // شناسه برای پیدا کردن کاربر در دیتابیس
        name,
        username,
        phone,
        email,
        password,
        errorMsg
        );

    if (success) {
        QMessageBox::information(this, "موفق", "پروفایل شما با موفقیت بروزرسانی شد.");

        // آپدیت کردن اطلاعات شیء کاربر در خود این کلاس
        m_currentUser.name = name;
        m_currentUser.username = username;
        m_currentUser.phone = phone;
        m_currentUser.email = email;

        // ارسال سیگنال به همراه اطلاعات جدید کاربر
        emit profileUpdated(m_currentUser);
        this->close();
    } else {
        // نمایش خطاهای ولیدیشن یا دیتابیس
        ui->errorLabel->setStyleSheet("color: red;");
        ui->errorLabel->setText(errorMsg);
    }
}
