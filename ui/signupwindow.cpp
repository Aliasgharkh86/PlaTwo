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

SignUpWindow::SignUpWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SignUpWindow)
{
    ui->setupUi(this);
}

SignUpWindow::~SignUpWindow()
{
    delete ui;
}

void SignUpWindow::on_backToLoginButton_clicked()
{
    LoginWindow *login = new LoginWindow();
    login->show();
    this->hide();
}


void SignUpWindow::on_pushButton_clicked()
{
    QString name = ui->nameLineEdit->text();
    QString username = ui->usernameLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString phonnumber = ui->phoneLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    if(name.isEmpty() || username.isEmpty() || email.isEmpty() || phonnumber.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this,"warning","Please fill in the blanks.");
        return;
    }

    if(password.length() < 8){
        QMessageBox::warning(this,"warning","The password must have eigth characters at least.");
        return;
    }
    // چک کردن فرمت ایمیل
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}$");
    QRegularExpressionMatch match = emailRegex.match(email);

    if (!match.hasMatch()) {
        QMessageBox::warning(this, "warning", "The format of email must be valid.");
        return;
    }

    // هش کردن پسوورد
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hashedBytes = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    QString hashedPassword = QString(hashedBytes.toHex());

    //ذخیره اطلاعات کاربر
    QString filePath = QDir::currentPath() + "/users.json";
    QFile file(filePath);
    QJsonArray usersArray;

    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                usersArray = doc.array();
            }
            file.close();
        }
    }

    for (const QJsonValue &val : usersArray) {
        QJsonObject obj = val.toObject();
        if (obj["username"].toString() == username) {
            QMessageBox::warning(this, "Error", "Username already exists!");
            return;
        }
        if (obj["email"].toString() == email) {
            QMessageBox::warning(this, "Error", "Email is already registered!");
            return;
        }
        if (obj["phonnumber"].toString() == phonnumber){
            QMessageBox::warning(this, "Error", "Phonnumber is already exists!");
            return;
        }
    }

    QJsonObject newUser;
    newUser["name"] = name;
    newUser["username"] = username;
    newUser["email"] = email;
    newUser["phonnumber"] = phonnumber;
    newUser["password"] = hashedPassword;

    usersArray.append(newUser);
    QJsonDocument newDoc(usersArray);

    if (file.open(QIODevice::WriteOnly)) {
        file.write(newDoc.toJson());
        file.close();

        QMessageBox::information(this, "Success", "Registration successful!");

    } else {
        QMessageBox::critical(this, "Error", "Could not save user data!");
    }
}