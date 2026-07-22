#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "gamemessage.h"

// GameClient: مدیریت اتصال کلاینت به سرور
// نفر B این کلاس رو پیاده‌سازی می‌کنه
// نفر A فقط از GameServer استفاده می‌کنه

class GameClient : public QObject
{
    Q_OBJECT

public:
    explicit GameClient(QObject* parent = nullptr);
    ~GameClient();

    // اتصال به سرور
    // serverIP: آدرس IP سرور که میزبان داده
    // port: پورتی که میزبان انتخاب کرده
    // username: یوزرنیم کاربر فعلی
    void connectToServer(const QString& serverIP,
                         int port,
                         const QString& username);

    // قطع اتصال
    void disconnectFromServer();

    // ارسال حرکت به سرور
    void sendMove(int row, int col, const QString& extra = "");

    // ارسال پیام چت (امتیازی)
    void sendChat(const QString& text);

    // وضعیت اتصال
    bool isConnected() const;

    // گرفتن یوزرنیم خودمون
    QString username() const { return m_username; }

signals:
    // اتصال موفق بود
    void connected();

    // اتصال قطع شد
    void disconnected();

    // خطا در اتصال (مثلاً IP اشتباه)
    void connectionError(const QString& errorMessage);

    // بازی شروع شد - اطلاعات هر دو بازیکن و تنظیمات
    void gameStarted(const QString& player1,
                     const QString& player2,
                     int boardSize,
                     bool hasTimer,
                     int timerSeconds);

    // حرکت حریف دریافت شد
    void opponentMoved(int row, int col, const QString& extra);

    // نتیجه حرکت خودمون
    void moveResult(bool isValid, int row, int col, const QString& extra);

    // آپدیت تایمر
    // playerIndex: 0 یا 1 (ایندکس بازیکنی که تایمرش آپدیت شده)
    void timerUpdated(int playerIndex, int remainingSeconds);

    // بازی تموم شد
    // reason: "normal" | "timeout" | "disconnect"
    void gameOver(const QString& winnerUsername, const QString& reason);

    // حریف قطع شد
    void opponentDisconnected();

    // پیام چت دریافت شد (امتیازی)
    void chatReceived(const QString& senderUsername, const QString& text);

    // لاگ برای debug
    void logMessage(const QString& msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onDataReceived();
    void onSocketError(QAbstractSocket::SocketError error);
    void onReconnectTimer();

private:
    QTcpSocket* m_socket;
    QTimer*     m_reconnectTimer;   // برای reconnect بعد از قطع اتصال
    QString     m_username;
    QString     m_lastServerIP;
    int         m_lastPort = 0;
    bool        m_intentionalDisconnect = false; // آیا خودمون قطع کردیم؟
    int         m_reconnectAttempts = 0;
    QByteArray  m_buffer;           // buffer برای پیام‌های ناقص

    void sendMessage(const GameMessage& msg);
    void handleMessage(const GameMessage& msg);
    void processBuffer();           // پردازش buffer و جدا کردن پیام‌های کامل
};

#endif // GAMECLIENT_H

