#include "recoverywindow.h"
#include "ui_recoverywindow.h"

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
