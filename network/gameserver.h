#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>
#include "gamemessage.h"
#include "gameroom.h"

// ─────────────────────────────────────────────
//  GameServer  –  نفر A این کلاس رو پیاده‌سازی می‌کنه
//
//  وظایف:
//    ۱. گوش دادن روی یه پورت
//    ۲. قبول کردن دو بازیکن
//    ۳. شروع بازی وقتی هر دو وصل شدند
//    ۴. دریافت حرکت از هر بازیکن، اعتبارسنجی، فوروارد به حریف
//    ۵. مدیریت تایمر
//    ۶. اعلام پایان بازی
// ─────────────────────────────────────────────

class GameServer : public QObject
{
    Q_OBJECT

public:
    explicit GameServer(QObject* parent = nullptr);
    ~GameServer();

    // شروع گوش دادن با تنظیمات میزبان
    bool startListening(const GameSettings& settings);

    // متوقف کردن سرور
    void stop();

    // آدرس IP سرور (برای نمایش به میزبان)
    QString serverIP() const;

    int serverPort() const { return m_room.settings.port; }

    bool isListening() const;

signals:
    // یه بازیکن جدید وصل شد (نام بازیکن + شماره اسلات 0 یا 1)
    void playerConnected(int slot, const QString& username);

    // هر دو بازیکن آماده‌اند، بازی شروع شد
    void gameStarted(const QString& p1, const QString& p2);

    // بازی تموم شد
    void gameEnded(const QString& winnerUsername, const QString& reason);

    // برای debug در UI
    void logMessage(const QString& msg);

private slots:
    void onNewConnection();
    void onDataReceived();
    void onClientDisconnected();
    void onTimerTick();

private:
    // ── Message handlers ──────────────────────
    void handleMessage(QTcpSocket* sender, const GameMessage& msg);
    void handlePlayerJoined(QTcpSocket* sender, const QJsonObject& data);
    void handleMove(QTcpSocket* sender, const QJsonObject& data);
    void handleChat(QTcpSocket* sender, const QJsonObject& data);

    // ── Network helpers ───────────────────────
    void sendTo(QTcpSocket* socket, const GameMessage& msg);
    void sendToAll(const GameMessage& msg);
    void sendToOpponent(QTcpSocket* sender, const GameMessage& msg);

    // ── Game logic ────────────────────────────
    void tryStartGame();           // اگه هر دو وصل شدند، بازی رو شروع کن
    void switchTurn();
    void endGame(const QString& winnerUsername, const QString& reason);

    // ── State ─────────────────────────────────
    QTcpServer* m_server;
    GameRoom    m_room;
    QTimer*     m_turnTimer;       // تایمر هر نوبت

    // buffer برای هر سوکت (داده‌های ناقص TCP)
    QMap<QTcpSocket*, QByteArray> m_buffers;
};

#endif // GAMESERVER_H

