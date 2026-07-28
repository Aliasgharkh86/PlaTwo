#include "gameserver.h"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>
#include "../games/dotsandboxesgame.h"


// ─────────────────────────────────────────────
//  GameServer – Implementation
// ─────────────────────────────────────────────

GameServer::GameServer(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_turnTimer(new QTimer(this))
{
    m_turnTimer->setInterval(1000); // هر ثانیه یه بار tick

    connect(m_server,    &QTcpServer::newConnection,
            this,         &GameServer::onNewConnection);

    connect(m_turnTimer, &QTimer::timeout,
            this,         &GameServer::onTimerTick);
}

GameServer::~GameServer()
{
    stop();
}

// ─── Public ───────────────────────────────────────────────────────────────────

bool GameServer::startListening(const GameSettings& settings)
{
    m_room          = GameRoom{};          // ریست اتاق
    m_room.settings = settings;

    if (!m_server->listen(QHostAddress::Any,
                          static_cast<quint16>(settings.port))) {
        emit logMessage("خطا در شروع سرور: " + m_server->errorString());
        return false;
    }

    emit logMessage(QString("سرور روی پورت %1 شروع به کار کرد.")
                        .arg(settings.port));
    return true;
}

void GameServer::stop()
{
    m_turnTimer->stop();

    // قطع همه بازیکنان
    for (auto& p : m_room.players)
        if (p.socket) p.socket->disconnectFromHost();

    m_server->close();
    m_buffers.clear();
}

bool GameServer::isListening() const
{
    return m_server->isListening();
}

QString GameServer::serverIP() const
{
    // اولین IP غیر loopback IPv4
    const auto addrs = QNetworkInterface::allAddresses();
    for (const QHostAddress& addr : addrs) {
        if (addr != QHostAddress::LocalHost && addr.toIPv4Address())
            return addr.toString();
    }
    return "127.0.0.1";
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void GameServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();

        // اگه اتاق پره، رد کن
        if (m_room.isFull()) {
            sendTo(socket, GameMessage::makeError("اتاق پر است."));
            socket->disconnectFromHost();
            socket->deleteLater();
            emit logMessage("اتصال رد شد: اتاق پر است.");
            continue;
        }

        // پیدا کردن اسلات خالی
        int slot = (m_room.players[0].socket == nullptr) ? 0 : 1;

        m_room.players[slot].socket = socket;
        m_room.players[slot].isHost = (slot == 0);
        m_buffers[socket]           = QByteArray();

        connect(socket, &QTcpSocket::readyRead,
                this,   &GameServer::onDataReceived);

        connect(socket, &QTcpSocket::disconnected,
                this,   &GameServer::onClientDisconnected);

        emit logMessage(QString("اتصال جدید در اسلات %1").arg(slot));
    }
}

void GameServer::onDataReceived()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    m_buffers[socket] += socket->readAll();

    // جدا کردن پیام‌های کامل با \n
    while (m_buffers[socket].contains('\n')) {
        int        idx  = m_buffers[socket].indexOf('\n');
        QByteArray line = m_buffers[socket].left(idx).trimmed();
        m_buffers[socket] = m_buffers[socket].mid(idx + 1);

        if (!line.isEmpty()) {
            GameMessage msg = GameMessage::fromBytes(line);
            handleMessage(socket, msg);
        }
    }
}

void GameServer::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    int slot = m_room.indexOfSocket(socket);
    emit logMessage(QString("بازیکن اسلات %1 قطع شد.").arg(slot));

    // اگه بازی در جریان بود، به حریف اطلاع بده
    if (m_room.gameStarted) {
        QTcpSocket* opp = m_room.opponentOf(socket);
        if (opp) {
            sendTo(opp, GameMessage{MessageType::PLAYER_DISCONNECTED, {}});
        }

        // برنده کسیه که قطع نشده
        if (slot>= 0) {
            QString winner = m_room.players[1 - slot].username;
            endGame(winner, "disconnect");
        }
    }

    m_buffers.remove(socket);
    if (slot >= 0) m_room.players[slot] = RoomPlayer{};
    socket->deleteLater();
}

void GameServer::onTimerTick()
{
    // فقط وقتی تایمر فعاله
    if (!m_room.settings.hasTimer || !m_room.gameStarted) return;

    int cur = m_room.currentTurn;

    static QMap<int, int> timeLeft;
    if (!timeLeft.contains(cur))
        timeLeft[cur] = m_room.settings.timerSecs;

    timeLeft[cur]--;

    sendToAll(GameMessage::makeTimerUpdate(cur, timeLeft[cur]));

    if (timeLeft[cur] <= 0) {
        timeLeft.clear();
        m_turnTimer->stop();
        // بازنده کسیه که وقتش تموم شد
        QString winner = m_room.players[1 - cur].username;
        endGame(winner, "timeout");
    }
}

// ─── Message handlers ─────────────────────────────────────────────────────────

void GameServer::handleMessage(QTcpSocket* sender, const GameMessage& msg)
{
    switch (msg.type) {
    case MessageType::PLAYER_JOINED:
        handlePlayerJoined(sender, msg.data);
        break;
    case MessageType::MOVE:
        handleMove(sender, msg.data);
        break;
    case MessageType::CHAT_MESSAGE:
        handleChat(sender, msg.data);
        break;
    case MessageType::GAME_OVER:
        if (m_room.gameStarted) {
            QString winner = msg.data["winner"].toString();
            endGame(winner, "normal");
        }
        break;
    default:
        emit logMessage("پیام ناشناخته از کلاینت.");
        break;
    }
}

void GameServer::handlePlayerJoined(QTcpSocket* socket, const QJsonObject& data)
{
    int slot = m_room.indexOfSocket(socket);
    if (slot < 0) return;

    m_room.players[slot].username = data["username"].toString();
    emit logMessage(QString("بازیکن '%1' در اسلات %2 معرفی شد.")
                        .arg(m_room.players[slot].username).arg(slot));

    emit playerConnected(slot, m_room.players[slot].username);

    // اگه هر دو آماده‌اند، بازی رو شروع کن
    tryStartGame();
}

void GameServer::handleMove(QTcpSocket* socket, const QJsonObject& data)
{
    if (!m_room.gameStarted) return;

    int slot = m_room.indexOfSocket(socket);
    if (slot < 0) return;

    // چک نوبت
    if (slot != m_room.currentTurn) {
        sendTo(socket, GameMessage::makeError("نوبت شما نیست."));
        return;
    }

    int     row   = data["row"].toInt();
    int     col   = data["col"].toInt();
    QString extra = data["extra"].toString();

    // ── اعتبارسنجی حرکت ──────────────────────
    bool valid = true; // تنظیم روی true تا حرکات Fanorona و بقیه بازی‌ها بدون مشکل ریلی شوند[span_2](start_span)[span_2](end_span)

    if (m_room.game) {
        QJsonDocument doc = QJsonDocument::fromJson(extra.toUtf8());
        QVariantMap moveData;
        if (!doc.isNull() && doc.isObject()) {
            moveData = doc.object().toVariantMap();
        } else {
            moveData["row"] = row;
            moveData["col"] = col;
        }
        valid = m_room.game->makeMove(slot, QVariant(moveData));
    }

    // نتیجه رو به فرستنده بده
    GameMessage result;
    result.type         = MessageType::MOVE_RESULT;
    result.data["valid"] = valid;
    result.data["row"]   = row;
    result.data["col"]   = col;
    result.data["extra"] = extra;
    sendTo(socket, result);

    if (!valid) return;

    // حرکت معتبر رو به حریف بفرست
    sendToOpponent(socket, GameMessage::makeMove(row, col, extra));

    emit logMessage(QString("حرکت [%1][%2] از بازیکن '%3' فوروارد شد.")
                        .arg(row).arg(col)
                        .arg(m_room.players[slot].username));

    QJsonDocument extraDoc = QJsonDocument::fromJson(extra.toUtf8());
    bool keepTurn = false;
    if (!extraDoc.isNull() && extraDoc.isObject())
        keepTurn = extraDoc.object()["keepTurn"].toBool();

    // نوبت رو فقط وقتی عوض کن که keepTurn نخواسته باشیم (برای زنجیره زدن در فنورونا)[span_3](start_span)[span_3](end_span)[span_4](start_span)[span_4](end_span)
    if (!keepTurn)
        switchTurn();
}

void GameServer::handleChat(QTcpSocket* socket, const QJsonObject& data)
{
    // فقط پیام رو به حریف فوروارد کن
    GameMessage msg;
    msg.type = MessageType::CHAT_MESSAGE;
    msg.data = data;
    sendToOpponent(socket, msg);
}

// ─── Game logic ───────────────────────────────────────────────────────────────

void GameServer::tryStartGame()
{
    if (!m_room.isFull()) return;

    // چک کن هر دو username داشته باشند
    if (m_room.players[0].username.isEmpty() ||
        m_room.players[1].username.isEmpty()) return;

    m_room.gameStarted  = true;
    if (m_room.game) {
        delete m_room.game;
        m_room.game = nullptr;
    }

    if (m_room.settings.gameType == GameType::DOTS_AND_BOXES) {
        m_room.game = new DotsAndBoxesGame();
    }
    // برای فنورونا نیازی به ساخت شیء درون سرور نیست چون اعتبارسنجی کامل در کلاینت انجام می‌شود

    m_room.currentTurn  = 0;   // میزبان اول بازی می‌کنه[span_5](start_span)[span_5](end_span)[span_6](start_span)[span_6](end_span)

    QString p1 = m_room.players[0].username;
    QString p2 = m_room.players[1].username;

    emit logMessage(QString("بازی شروع شد: %1 vs %2").arg(p1, p2));

    // ارسال GAME_START به هر دو بازیکن[span_7](start_span)[span_7](end_span)[span_8](start_span)[span_8](end_span)
    GameMessage startMsg = GameMessage::makeGameStart(
        p1, p2,
        m_room.settings.boardSize,
        m_room.settings.hasTimer,
        m_room.settings.timerSecs
        );
    sendToAll(startMsg);

    emit gameStarted(p1, p2);

    // شروع تایمر اگه لازمه[span_9](start_span)[span_9](end_span)[span_10](start_span)[span_10](end_span)
    if (m_room.settings.hasTimer)
        m_turnTimer->start();
}

void GameServer::switchTurn()
{
    m_room.currentTurn = 1 - m_room.currentTurn;
}

void GameServer::endGame(const QString& winnerUsername, const QString& reason)
{
    m_turnTimer->stop();
    m_room.gameStarted = false;

    sendToAll(GameMessage::makeGameOver(winnerUsername, reason));
    emit gameEnded(winnerUsername, reason);

    emit logMessage(QString("بازی تمام شد. برنده: %1 (%2)")
                        .arg(winnerUsername, reason));
}

// ─── Network helpers ────────────────────────────────__________________________

void GameServer::sendTo(QTcpSocket* socket, const GameMessage& msg)
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(msg.toBytes());
        socket->flush();
    }
}

void GameServer::sendToAll(const GameMessage& msg)
{
    for (auto& p : m_room.players)
        sendTo(p.socket, msg);
}

void GameServer::sendToOpponent(QTcpSocket* senderSocket, const GameMessage& msg)
{
    QTcpSocket* opp = m_room.opponentOf(senderSocket);
    sendTo(opp, msg);
}
