#include "signupwindow.h"
#include "ui_signupwindow.h"
#include "loginwindow.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include "core/authmanager.h"

SignUpWindow::SignUpWindow(QWidget* parent)
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

void SignUpWindow::on_backToLoginButton_clicked()
{
    emit switchToLogin();
}

void SignUpWindow::on_pushButton_clicked()
{
    QString err;
    bool ok = AuthManager::instance().signUp(
        ui->nameLineEdit->text(),
        ui->usernameLineEdit->text(),
        ui->phoneLineEdit->text(),
        ui->emailLineEdit->text(),
        ui->passwordLineEdit->text(),
        err
        );

    if (!ok) {
        ui->errorLabel->setText("⚠️ " + err);
        return;
    }

    QMessageBox::information(this, "موفق", "ثبت‌نام انجام شد!");
    emit switchToLogin();
    emit signUpSuccess() ;
}