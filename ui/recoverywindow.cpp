#include "recoverywindow.h"
#include "ui_recoverywindow.h"
#include "core/authmanager.h"
#include <QMessageBox>

RecoveryWindow::RecoveryWindow(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RecoveryWindow)
{
    ui->setupUi(this);
    setFixedSize(420, 420);
    setWindowTitle("PlaTwo - بازیابی رمز عبور");

    // در ابتدا لیبل‌های خطا و موفقیت رو مخفی کن
    ui->errorLabel->hide();
    ui->successLabel->hide();
}

RecoveryWindow::~RecoveryWindow()
{
    delete ui;
}

void RecoveryWindow::on_confirmBtn_clicked()
{
    // مخفی کردن پیام‌های قبلی
    ui->errorLabel->hide();
    ui->successLabel->hide();

    QString phone       = ui->phoneLineEdit->text().trimmed();
    QString newPass     = ui->newPasswordLineEdit->text();
    QString confirmPass = ui->confirmPasswordLineEdit->text();

    // --- چک خالی نبودن فیلدها ---
    if (phone.isEmpty() || newPass.isEmpty() || confirmPass.isEmpty()) {
        ui->errorLabel->setText("⚠️ لطفاً همه فیلدها را پر کنید.");
        ui->errorLabel->show();
        return;
    }

    // --- چک تطابق رمز عبور ---
    if (newPass != confirmPass) {
        ui->errorLabel->setText("⚠️ رمز عبور جدید و تکرار آن یکسان نیستند.");
        ui->errorLabel->show();
        return;
    }

    // --- تغییر رمز از طریق AuthManager ---
    QString err;
    bool ok = AuthManager::instance().resetPassword(phone, newPass, err);

    if (!ok) {
        ui->errorLabel->setText("⚠️ " + err);
        ui->errorLabel->show();
        return;
    }

    // --- موفق ---
    ui->successLabel->setText("✅ رمز عبور با موفقیت تغییر یافت.");
    ui->successLabel->show();

    // پاک کردن فیلدها
    ui->phoneLineEdit->clear();
    ui->newPasswordLineEdit->clear();
    ui->confirmPasswordLineEdit->clear();

    // بعد از ۱.۵ ثانیه برگرد به Login
    QMessageBox::information(this, "موفق",
                             "رمز عبور با موفقیت تغییر یافت.\nاکنون وارد شوید.");
    emit switchToLogin();
}

void RecoveryWindow::on_backBtn_clicked()
{
    // پاک کردن فیلدها موقع برگشت
    ui->phoneLineEdit->clear();
    ui->newPasswordLineEdit->clear();
    ui->confirmPasswordLineEdit->clear();
    ui->errorLabel->hide();
    ui->successLabel->hide();

    emit switchToLogin();
}
