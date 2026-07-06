#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "signupwindow.h"
#include "recoverywindow.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include "core/authmanager.h"
#
LoginWindow::LoginWindow(QDialog *parent)
    : QDialog(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    connect(ui->signupBtn, &QPushButton::clicked,
            this, &LoginWindow::switchToSignup);
    connect(ui->loginBtn, &QPushButton::clicked,
            this, &LoginWindow::on_loginButton_clicked);
    connect(ui->recoveryBtn, &QPushButton::clicked,
            this, &LoginWindow::switchToRecovery);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_signUpButton_clicked()
{
    SignUpWindow *signup = new SignUpWindow();
    signup->show();
    this->hide();
}

void LoginWindow::on_recoveryButton_clicked()
{
    RecoveryWindow *recover = new RecoveryWindow();
    recover->show();
    this->hide();
}

void LoginWindow::on_loginButton_clicked(){
    ui->errorLabel->setText("");

    QString err;
    User u = AuthManager::instance().login(
        ui->usernameLineEdit->text(),
        ui->passwordLineEdit->text(),
        err
        );

    // به جای u.isValid، بررسی می‌کنیم که آیا متغیر ارور متنی دارد یا خیر
    if (!err.isEmpty()) {
        ui->errorLabel->setText("⚠️ " + err);
        return;
    }

    // اگر متغیر ارور خالی باشد، یعنی لاگین صد در صد موفق بوده است
    emit loginSuccess(u);
}


