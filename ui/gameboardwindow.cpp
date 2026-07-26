#include "gameboardwindow.h"
#include <QJsonDocument>
#include <QJsonObject>

GameBoardWindow::GameBoardWindow(Game*       game,
                                 QWidget*    boardWidget,
                                 GameClient* client,
                                 int         myPlayer,
                                 QWidget*    parent)
    : QWidget(parent)
    , m_game(game)
    , m_boardWidget(boardWidget)
    , m_client(client)
    , m_myPlayer(myPlayer)
{
    // ── چیدمان داینامیک (بدون .ui) ─────────────
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // تیتر
    const QString titleText = m_game ? m_game->gameName() : "بازی";
    auto* titleLabel = new QLabel("🎲  " + titleText, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont f = titleLabel->font();
    f.setPointSize(13);
    f.setBold(true);
    titleLabel->setFont(f);
    mainLayout->addWidget(titleLabel);

    // تخته‌ی بازی
    if (m_boardWidget) {
        m_boardWidget->setParent(this);
        mainLayout->addWidget(m_boardWidget, 1);

        // دریافت حرکت از ویجت بازی
        connect(m_boardWidget, SIGNAL(moveReadyToSend(QVariantMap)),
                this, SLOT(onMoveReadyToSend(QVariantMap)));
    }

    // دکمه‌ی بازگشت
    auto* backBtn = new QPushButton("بازگشت به منو", this);
    mainLayout->addWidget(backBtn);

    connect(backBtn, &QPushButton::clicked,
            this, &GameBoardWindow::backToMenuRequested);

    // ── اتصالات شبکه مطابق با GameClient شما ────────────
    if (m_client) {
        // ۱. دریافت حرکت حریف از شبکه
        connect(m_client, &GameClient::opponentMoved,
                this, &GameBoardWindow::onOpponentMoved);

        // ۲. پایان بازی از طرف شبکه
        connect(m_client, &GameClient::gameOver,
                this, &GameBoardWindow::onGameOver);

        // ۳. قطع اتصال حریف
        connect(m_client, &GameClient::opponentDisconnected,
                this, &GameBoardWindow::onOpponentDisconnected);
    }

    setMinimumSize(520, 600);
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

    // نگاشت کلیدهای دوز روی row و col برای جلوگیری از مقدار 1- و رد شدن توسط سرور
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

    // ۱. ابتدا تلاش می‌کنیم extra را به عنوان JSON پارس کنیم
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
    QString msg;
    if (winner == m_client->username())
        msg = "🎉  شما بردید!";
    else if (winner.isEmpty())
        msg = "🤝  مساوی!";
    else
        msg = "😔  حریف برد.";

    QMessageBox::information(this, "پایان بازی", msg);
}