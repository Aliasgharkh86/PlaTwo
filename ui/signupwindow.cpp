#include "signupwindow.h"
#include "ui_signupwindow.h"
#include "../core/authmanager.h"
#include <QMessageBox>

SignUpWindow::SignUpWindow(QDialog* parent)
    : QDialog(parent)
    , ui(new Ui::SignUpWindow)   // اول اینجا initialize کن
{
    ui->setupUi(this);
    setFixedSize(380, 560);
    setWindowTitle("PlaTwo - ثبت‌نام");
    connect(ui->loginBtn, &QPushButton::clicked,
            this, &SignUpWindow::switchToLogin);
}
SignUpWindow::~SignUpWindow()
{
    delete ui;
}

void SignUpWindow::onSignupClicked()
{
    ui->errorLabel->hide();

    // if (ui->passwordEdit->text() != ui->confirmPassEdit->text()) {
    //     ui->errorLabel->setText("رمز عبور و تکرار آن یکسان نیستند.");
    //     ui->errorLabel->show();
    //     return;
    // }

    QString err;
    bool ok = AuthManager::instance().signUp(
        ui->nameEdit->text(),
        ui->usernameEdit->text(),
        ui->phoneEdit->text(),
        ui->emailEdit->text(),
        ui->passwordEdit->text(),
        err
        );

    if (!ok) {
        ui->errorLabel->setText("⚠️ " + err);
        ui->errorLabel->show();
        return;
    }

    QMessageBox::information(this, "موفق",
                             "ثبت‌نام با موفقیت انجام شد!");
    emit signUpSuccess();
}