#include "gameclient.h"
#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>

GameClient::GameClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setInterval(3000); // هر ۳ ثانیه یه بار تلاش برای reconnect
    m_reconnectTimer->setSingleShot(false);

    connect(m_socket, &QTcpSocket::connected,
            this, &GameClient::onConnected);

    connect(m_socket, &QTcpSocket::disconnected,
            this, &GameClient::onDisconnected);

    connect(m_socket, &QTcpSocket::readyRead,
            this, &GameClient::onDataReceived);

    connect(m_socket, &QAbstractSocket::errorOccurred,
            this, &GameClient::onSocketError);

    connect(m_reconnectTimer, &QTimer::timeout,
            this, &GameClient::onReconnectTimer);
}

GameClient::~GameClient()
{
    disconnectFromServer();
}

void GameClient::connectToServer(const QString& serverIP,
                                 int port,
                                 const QString& username)
{
    m_username              = username;
    m_lastServerIP          = serverIP;
    m_lastPort              = port;
    m_intentionalDisconnect = false;
    m_reconnectAttempts     = 0;

    emit logMessage(QString("در حال اتصال به %1:%2 ...").arg(serverIP).arg(port));

    // اگه قبلاً وصل بودیم، اول قطع کن
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected(1000);
    }

    m_socket->connectToHost(serverIP, static_cast<quint16>(port));
}

void GameClient::disconnectFromServer()
{
    m_intentionalDisconnect = true;
    m_reconnectTimer->stop();

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

void GameClient::sendMove(int row, int col, const QString& extra)
{
    if (!isConnected()) {
        emit logMessage("خطا: اتصال برقرار نیست.");
        return;
    }
    sendMessage(GameMessage::makeMove(row, col, extra));
}

void GameClient::sendChat(const QString& text)
{
    if (!isConnected() || text.trimmed().isEmpty()) return;
    sendMessage(GameMessage::makeChat(m_username, text));
}

bool GameClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

// ─── private slots ────────────────────────────────────────────────────────────

void GameClient::onConnected()
{
    m_reconnectTimer->stop();
    m_reconnectAttempts = 0;
    m_buffer.clear();

    emit logMessage("اتصال برقرار شد. در انتظار شروع بازی...");
    emit connected();

    // اولین پیام بعد از اتصال: معرفی خودمون به سرور
    GameMessage joinMsg;
    joinMsg.type              = MessageType::PLAYER_JOINED;
    joinMsg.data["username"]  = m_username;
    sendMessage(joinMsg);
}

void GameClient::onDisconnected()
{
    emit logMessage("اتصال قطع شد.");
    emit disconnected();

    // اگه خودمون قطع نکردیم، سعی کن دوباره وصل بشی
    if (!m_intentionalDisconnect && m_reconnectAttempts < 5) {
        emit logMessage("تلاش برای اتصال مجدد...");
        m_reconnectTimer->start();
        emit opponentDisconnected();
    }
}

void GameClient::onDataReceived()
{
    // داده رو به buffer اضافه کن
    m_buffer += m_socket->readAll();

    // پردازش پیام‌های کامل
    processBuffer();
}

void GameClient::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QString errMsg = m_socket->errorString();
    emit logMessage("خطای شبکه: " + errMsg);
    emit connectionError(errMsg);
}

void GameClient::onReconnectTimer()
{
    if (isConnected()) {
        m_reconnectTimer->stop();
        return;
    }

    m_reconnectAttempts++;
    emit logMessage(QString("تلاش %1 از 5 برای اتصال مجدد...")
                        .arg(m_reconnectAttempts));

    m_socket->connectToHost(m_lastServerIP,
                            static_cast<quint16>(m_lastPort));

    if (m_reconnectAttempts >= 5) {
        m_reconnectTimer->stop();
        emit connectionError("اتصال مجدد ناموفق بود.");
    }
}

// ─── private helpers ──────────────────────────────────────────────────────────

void GameClient::sendMessage(const GameMessage& msg)
{
    if (!isConnected()) return;
    m_socket->write(msg.toBytes());
    m_socket->flush();
}

void GameClient::processBuffer()
{
    // پیام‌ها با \n از هم جدا میشن (همون چیزی که toBytes() اضافه می‌کنه)
    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer = m_buffer.mid(idx + 1);

        if (!line.isEmpty()) {
            GameMessage msg = GameMessage::fromBytes(line);
            handleMessage(msg);
        }
    }
}

void GameClient::handleMessage(const GameMessage& msg)
{
    switch (msg.type) {

    case MessageType::GAME_START: {
        QString p1      = msg.data["player1"].toString();
        QString p2      = msg.data["player2"].toString();
        int boardSize   = msg.data["boardSize"].toInt();
        bool hasTimer   = msg.data["hasTimer"].toBool();
        int timerSecs   = msg.data["timerSeconds"].toInt();

        emit logMessage(QString("بازی شروع شد: %1 vs %2").arg(p1, p2));
        emit gameStarted(p1, p2, boardSize, hasTimer, timerSecs);
        break;
    }

    case MessageType::MOVE_RESULT: {
        // نتیجه حرکت خودمون
        bool   isValid = msg.data["valid"].toBool();
        int    row     = msg.data["row"].toInt();
        int    col     = msg.data["col"].toInt();
        QString extra  = msg.data["extra"].toString();
        emit moveResult(isValid, row, col, extra);
        break;
    }

    case MessageType::MOVE: {
        // حرکت حریف
        int    row    = msg.data["row"].toInt();
        int    col    = msg.data["col"].toInt();
        QString extra = msg.data["extra"].toString();
        emit opponentMoved(row, col, extra);
        break;
    }

    case MessageType::TIMER_UPDATE: {
        int playerIndex = msg.data["playerIndex"].toInt();
        int remaining   = msg.data["remaining"].toInt();
        emit timerUpdated(playerIndex, remaining);
        break;
    }

    case MessageType::GAME_OVER: {
        QString winner = msg.data["winner"].toString();
        QString reason = msg.data["reason"].toString();
        emit logMessage(QString("بازی تموم شد. برنده: %1").arg(winner));
        emit gameOver(winner, reason);
        break;
    }

    case MessageType::PLAYER_DISCONNECTED: {
        emit logMessage("حریف قطع شد.");
        emit opponentDisconnected();
        break;
    }

    case MessageType::CHAT_MESSAGE: {
        QString sender = msg.data["sender"].toString();
        QString text   = msg.data["text"].toString();
        emit chatReceived(sender, text);
        break;
    }

    case MessageType::ERROR_MSG: {
        QString errMsg = msg.data["message"].toString();
        emit logMessage("خطا از سرور: " + errMsg);
        emit connectionError(errMsg);
        break;
    }

    default:
        emit logMessage("پیام ناشناخته دریافت شد.");
        break;
    }
}