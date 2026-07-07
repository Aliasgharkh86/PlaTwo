#include "gamelobbywindow.h"
#include "ui_gamelobbywindow.h"

GameLobbyWindow::GameLobbyWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameLobbyWindow)
{
    ui->setupUi(this);
}

GameLobbyWindow::~GameLobbyWindow()
{
    delete ui;
}
