#include "gamelobbywindow.h"
#include "ui_gamelobbywindow.h"
#include <QMessageBox>

GameLobbyWindow::GameLobbyWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameLobbyWindow),
    server(nullptr)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
}

GameLobbyWindow::~GameLobbyWindow()
{
    if (server) {
        server->close();
        delete server;
    }
    delete ui;
}

void GameLobbyWindow::on_hostButton_clicked()
{
    bool ok;
    quint16 port = ui->portLineEdit->text().toUShort(&ok);

    if (!ok || port == 0) {
        QMessageBox::warning(this, "خطا", "پورت نامعتبر است.");
        return;
    }

    if (!server) {
        server = new GameServer(this);
        connect(server, &GameServer::gameReady, this, &GameLobbyWindow::onGameReady);
        connect(server, &GameServer::gameAborted, this, &GameLobbyWindow::onGameAborted);
    }

    if (!server) {
        server = new GameServer(this);
        connect(server, &GameServer::gameReady, this, &GameLobbyWindow::onGameReady);
        connect(server, &GameServer::gameAborted, this, &GameLobbyWindow::onGameAborted);
    }

    // فراخوانی تابع (بدون قرار دادن در شرط)
    server->startServer(port);

    // بررسی وضعیت سرور با استفاده از isListening
    if (server->isListening()) {
        ui->statusLabel->setText("در حال انتظار برای کلاینت...");
        ui->hostButton->setEnabled(false);
        ui->portLineEdit->setEnabled(false);
    } else {
        QMessageBox::critical(this, "خطا", "راه‌اندازی سرور با مشکل مواجه شد.");
    }
}

void GameLobbyWindow::onGameReady()
{
    ui->statusLabel->setText("بازی آماده شروع است!");
    ui->stackedWidget->setCurrentIndex(1);
}

void GameLobbyWindow::onGameAborted()
{
    ui->statusLabel->setText("بازی لغو شد.");
    ui->hostButton->setEnabled(true);
    ui->portLineEdit->setEnabled(true);
}
