#include "gamelobbywindow.h"
#include "ui_gamelobbywindow.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>

GameLobbyWindow::GameLobbyWindow(const User& currentUser,
                                 GameType gameType,
                                 QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::GameLobbyWindow)
    , m_currentUser(currentUser)
    , m_gameType(gameType)
    , m_client(new GameClient(this))
{
    ui->setupUi(this);
    setFixedSize(420, 460);
    setWindowTitle("PlaTwo – " + gameTypeName());

    // ── تنظیمات اولیه ─────────────────────────
    ui->headerLabel->setText("🎲  " + gameTypeName());

    // IP validator
    QRegularExpression ipRx(R"(^(\d{1,3}\.){0,3}\d{0,3}$)");
    ui->ipEdit->setValidator(
        new QRegularExpressionValidator(ipRx, ui->ipEdit));

    // شروع از صفحه 0
    ui->stackedWidget->setCurrentIndex(0);

    // ── connects: صفحه 0 ──────────────────────
    connect(ui->hostBtn,  &QPushButton::clicked,
            this, &GameLobbyWindow::onHostClicked);
    connect(ui->guestBtn, &QPushButton::clicked,
            this, &GameLobbyWindow::onGuestClicked);
    connect(ui->backBtn,  &QPushButton::clicked,
            this, &GameLobbyWindow::backToMenu);


    // ── connects: صفحه 3 (Guest) ──────────────
    connect(ui->connectBtn,      &QPushButton::clicked,
            this, &GameLobbyWindow::onConnectClicked);
    connect(ui->backFromGuestBtn,&QPushButton::clicked,
            this, &GameLobbyWindow::onBackFromGuestClicked);
    connect(ui->ipEdit, &QLineEdit::returnPressed,
            this, &GameLobbyWindow::onConnectClicked);

    // ── connects: صفحه 4 (Connecting) ─────────
    connect(ui->cancelBtn, &QPushButton::clicked,
            this, &GameLobbyWindow::onCancelConnectClicked);

    // ── connects: GameClient ───────────────────
    connect(m_client, &GameClient::connected,
            this, &GameLobbyWindow::onClientConnected);
    connect(m_client, &GameClient::connectionError,
            this, &GameLobbyWindow::onClientConnectionError);
    connect(m_client, &GameClient::gameStarted,
            this, &GameLobbyWindow::onGameStarted);
}

GameLobbyWindow::~GameLobbyWindow()
{
    if (m_client->isConnected())
        m_client->disconnectFromServer();
    // if (m_server) {
    //     m_server->stop();
    //     m_server->deleteLater();
    // }
    delete ui;
}

// ── صفحه 0 ────────────────────────────────────

void GameLobbyWindow::onHostClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void GameLobbyWindow::onGuestClicked()
{
    ui->guestErrorLabel->hide();
    ui->stackedWidget->setCurrentIndex(3);
}

// ── صفحه 1 (Host) ─────────────────────────────


// ── صفحه 2 (Waiting) ──────────────────────────



// ── صفحه 3 (Guest) ────────────────────────────

void GameLobbyWindow::onConnectClicked()
{
    ui->guestErrorLabel->hide();

    QString ip   = ui->ipEdit->text().trimmed();
    int     port = ui->guestPortSpin->value();

    if (ip.isEmpty()) {
        ui->guestErrorLabel->setText("⚠️  آدرس IP را وارد کنید.");
        ui->guestErrorLabel->show();
        return;
    }

    QRegularExpression ipCheck(
        R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    if (!ipCheck.match(ip).hasMatch()) {
        ui->guestErrorLabel->setText("⚠️  فرمت IP معتبر نیست.");
        ui->guestErrorLabel->show();
        return;
    }

    ui->connectingLabel->setText(
        QString("⏳  در حال اتصال به %1:%2 ...").arg(ip).arg(port));
    ui->stackedWidget->setCurrentIndex(4);
    m_client->connectToServer(ip, port, m_currentUser.username);
}

void GameLobbyWindow::onBackFromGuestClicked()
{
    ui->guestErrorLabel->hide();
    ui->stackedWidget->setCurrentIndex(0);
}

// ── صفحه 4 (Connecting) ───────────────────────

void GameLobbyWindow::onCancelConnectClicked()
{
    m_client->disconnectFromServer();
    // ui->stackedWidget->setCurrentIndex(m_server ? 2 : 3);
}

// ── GameClient slots ───────────────────────────

void GameLobbyWindow::onClientConnected()
{
    ui->connectingLabel->setText(
        "✅  اتصال برقرار شد!\nدر انتظار شروع بازی...");
}

void GameLobbyWindow::onClientConnectionError(const QString& error)
{
    ui->stackedWidget->setCurrentIndex(3);
    ui->guestErrorLabel->setText("⚠️  " + error);
    ui->guestErrorLabel->show();
}

void GameLobbyWindow::onGameStarted(const QString& p1, const QString& p2,
                                    int boardSize, bool hasTimer, int timerSecs)
{
    emit gameReady(m_client, p1, p2, boardSize, hasTimer, timerSecs);
    hide();
}

// ── Helper ────────────────────────────────────

QString GameLobbyWindow::gameTypeName() const
{
    switch (m_gameType) {
    case GameType::DOTS_AND_BOXES:   return "Dots and Boxes";
    case GameType::NINE_MENS_MORRIS: return "Nine Men's Morris";
    case GameType::FANORONA:         return "Fanorona";
    default:                         return "بازی";
    }
}
