#include "gameboardwindow.h"
#include "chatwidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDateTime>

GameBoardWindow::GameBoardWindow(Game* game, QWidget* boardWidget, GameClient* client,
                                 int myPlayer, const User& currentUser,
                                 const QString& gameTypeStr,
                                 const QString& opponentUsername,
                                 QWidget* parent)
    : QWidget(parent)
    , m_game(game)
    , m_boardWidget(boardWidget)
    , m_client(client)
    , m_myPlayer(myPlayer)
    , m_currentUser(currentUser)
    , m_gameTypeStr(gameTypeStr)
    , m_opponentUsername(opponentUsername)
    , m_chatWidget(nullptr)
{
    const QString titleText = m_game ? m_game->gameName() : "بازی";

    // ── layout اصلی افقی: (تیتر + تخته + دکمه) | چت ──
    auto* rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    // ── ستون چپ ─────────────────────────────────
    auto* leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(6);

    // تیتر
    auto* titleLabel = new QLabel("🎲  " + titleText, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont f = titleLabel->font();
    f.setPointSize(13);
    f.setBold(true);
    titleLabel->setFont(f);
    leftLayout->addWidget(titleLabel);

    // تخته‌ی بازی
    if (m_boardWidget) {
        m_boardWidget->setParent(this);
        leftLayout->addWidget(m_boardWidget, 1);

        connect(m_boardWidget, SIGNAL(moveReadyToSend(QVariantMap)),
                this, SLOT(onMoveReadyToSend(QVariantMap)));
    }

    // دکمه‌ی بازگشت
    auto* backBtn = new QPushButton("بازگشت به منو", this);
    backBtn->setStyleSheet(
        "QPushButton { background-color: #4a90e2; color: white; border-radius: 6px;"
        "              padding: 8px 20px; font-weight: bold; font-size: 11pt; }"
        "QPushButton:hover   { background-color: #3a80d2; }"
        "QPushButton:pressed { background-color: #2a70c2; }");
    leftLayout->addWidget(backBtn);

    rootLayout->addLayout(leftLayout, 1);

    // ── ستون راست: چت ───────────────────────────
    if (m_client) {
        m_chatWidget = new ChatWidget(m_client->username(), this);
        m_chatWidget->setClient(m_client);
        rootLayout->addWidget(m_chatWidget);
    }

    // ── وقتی بازی روی کلاینت تموم شد، به سرور اطلاع بده ──
    if (m_game && m_client) {
        connect(m_game, &Game::gameEnded, this, [this](int winnerSlot) {
            // فقط host (player 0) نتیجه رو به سرور می‌فرسته تا دوبار نفرستیم
            if (m_myPlayer != 0) return;

            QString winnerUsername;
            if (winnerSlot == 0)
                winnerUsername = m_currentUser.username;
            else if (winnerSlot == 1)
                winnerUsername = m_opponentUsername;
            else
                winnerUsername = ""; // مساوی

            m_client->sendGameOver(winnerUsername);
        });
    }
    // ── connects ────────────────────────────────
    connect(backBtn, &QPushButton::clicked,
            this, &GameBoardWindow::backToMenuRequested);

    if (m_client) {
        connect(m_client, &GameClient::opponentMoved,
                this, &GameBoardWindow::onOpponentMoved);
        connect(m_client, &GameClient::gameOver,
                this, &GameBoardWindow::onGameOver);
        connect(m_client, &GameClient::opponentDisconnected,
                this, &GameBoardWindow::onOpponentDisconnected);
    }

    setMinimumSize(740, 600);
    setWindowTitle("PlaTwo — " + titleText);
}

// ── ارسال حرکت خودی به GameClient ──────────────────────────
void GameBoardWindow::onMoveReadyToSend(const QVariantMap& moveData)
{
    if (!m_client || !m_client->isConnected())
        return;

    // تبدیل کامل moveData به JSON
    QJsonObject jsonObj = QJsonObject::fromVariantMap(moveData);
    QString extraStr = QString::fromUtf8(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));

    int row = -1;
    int col = -1;

    // نگاشت کلیدهای بازی‌های مختلف روی row و col جهت جلوگیری از مقدار ۱- و رد شدن توسط سرور
    if (moveData.contains("remove")) {
        row = moveData.value("remove").toInt();
    } else if (moveData.contains("place")) {
        row = moveData.value("place").toInt();
    } else if (moveData.contains("from")) {
        row = moveData.value("from").toInt();
        col = moveData.value("to", -1).toInt();
    } else {
        row = moveData.value("row", -1).toInt();
        col = moveData.value("col", -1).toInt();
    }

    // ارسال به GameClient
    m_client->sendMove(row, col, extraStr);
}

// ── دریافت حرکت حریف از GameClient ──────────────────────────
void GameBoardWindow::onOpponentMoved(int row, int col, const QString& extra)
{
    const int opponentPlayer = 1 - m_myPlayer;
    if (!m_game) return;

    QVariantMap moveData;

    // ۱. ابتدا تلاش می‌کنیمextra را به عنوان JSON پارس کنیم
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(extra.toUtf8(), &err);

    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        moveData = doc.object().toVariantMap();
    } else {
        // ۲. اگر extra حاوی JSON نبود، از row و col عادی استفاده می‌کنیم (برای بازی‌های ساده)
        if (row != -1) moveData["row"] = row;
        if (col != -1) moveData["col"] = col;
        if (!extra.isEmpty()) moveData["extra"] = extra;
    }

    // اعمال حرکت حریف روی منطق بازی
    m_game->makeMove(opponentPlayer, moveData);
}

// ── قطع اتصال حریف ──────────────────────────────────────────
void GameBoardWindow::onOpponentDisconnected()
{
    QMessageBox::warning(this, "قطع اتصال", "ارتباط حریف با سرور قطع شد.");
    emit backToMenuRequested();
}

// ── پایان بازی از شبکه ──────────────────────────────────────
void GameBoardWindow::onGameOver(const QString& winner, const QString& reason)
{
    Q_UNUSED(reason)

    // ── ذخیره‌ی رکورد بازی ──────────────────────
    GameRecord record;
    record.userId           = m_currentUser.id;
    record.gameType         = m_gameTypeStr;
    record.opponentUsername = m_opponentUsername;
    record.role             = (m_myPlayer == 0) ? "host" : "guest";
    record.playedAt         = QDateTime::currentDateTime();

    if (winner.isEmpty())
        record.result = GameResult::DRAW;
    else if (winner == m_currentUser.username)
        record.result = GameResult::WIN;
    else
        record.result = GameResult::LOSE;

    StorageManager::instance().saveGameRecord(record);
    // ─────────────────────────────────────────────

    QString msg;
    if (winner == m_currentUser.username)
        msg = "🎉  شما بردید!";
    else if (winner.isEmpty())
        msg = "🤝  مساوی!";
    else
        msg = "😔  حریف برد.";

    QMessageBox::information(this, "پایان بازی", msg);
}