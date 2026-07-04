#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "signupwindow.h"
#include "recoverywindow.h"

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

