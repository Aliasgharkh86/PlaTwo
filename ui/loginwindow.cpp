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

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
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
        QString username = ui->usernameLineEdit->text();
        QString password = ui->passwordLineEdit->text();

        if(username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter both username and password.");
            return;
        }

        QByteArray passwordBytes = password.toUtf8();
        QByteArray hashedBytes = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
        QString hashedPassword = QString(hashedBytes.toHex());

        QString filePath = QDir::currentPath() + "/users.json";
        QFile file(filePath);

        if (!file.exists()) {
            QMessageBox::warning(this, "Error", "No users found. Please sign up first.");
            return;
        }

        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Could not open database file.");
            return;
        }

        QByteArray fileData = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        QJsonArray usersArray = doc.array();

        bool loginSuccessful = false;

        for (int i = 0; i < usersArray.size(); ++i) {
            QJsonObject userObj = usersArray[i].toObject();

            if (userObj["Username"].toString() == username && userObj["Password"].toString() == hashedPassword) {
                loginSuccessful = true;
                break;
            }
        }

        if (loginSuccessful) {
            QMessageBox::information(this, "Success", "Login Successful!");

        } else {
            QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
        }
}


