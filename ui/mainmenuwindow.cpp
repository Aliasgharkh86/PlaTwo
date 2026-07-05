#include "mainmenuwindow.h"
#include "ui_mainmenuwindow.h"

mainmenuwindow::mainmenuwindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainmenuwindow)
{
    ui->setupUi(this);
}

mainmenuwindow::~mainmenuwindow()
{
    delete ui;
}
