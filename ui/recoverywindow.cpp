#include "recoverywindow.h"
#include "ui_recoverywindow.h"
#include "loginwindow.h"

RecoveryWindow::RecoveryWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RecoveryWindow)
{
    ui->setupUi(this);
}

RecoveryWindow::~RecoveryWindow()
{
    delete ui;
}

void RecoveryWindow::on_backToLoginButton_clicked()
{
    LoginWindow *login = new LoginWindow();
    login->show();
    this->hide();
}

