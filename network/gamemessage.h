#ifndef GAMEMESSAGE_H
#define GAMEMESSAGE_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

// مشترک بین GameServer (نفر A) و GameClient (نفر B)
// بدون هماهنگی تغییر ندید

enum class MessageType {
    PLAYER_JOINED       = 1,
    GAME_START          = 2,
    PLAYER_DISCONNECTED = 3,
    MOVE                = 4,
    MOVE_RESULT         = 5,
    GAME_OVER           = 6,
    TIMER_UPDATE        = 7,
    GAME_SAVED          = 8,
    CHAT_MESSAGE        = 9,
    ERROR_MSG           = 10
};

struct GameMessage {
    MessageType type;
    QJsonObject data;

    // تبدیل به bytes برای ارسال روی socket
    QByteArray toBytes() const {
        QJsonObject obj;
        obj["type"] = static_cast<int>(type);
        obj["data"] = data;
        return QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
    }

    // تبدیل از bytes دریافت‌شده
    static GameMessage fromBytes(const QByteArray& bytes) {
        GameMessage msg;
        QJsonDocument doc = QJsonDocument::fromJson(bytes.trimmed());
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            msg.type = static_cast<MessageType>(obj["type"].toInt());
            msg.data = obj["data"].toObject();
        }
        return msg;
    }

    // سازنده‌های کمکی برای ساخت سریع پیام
    static GameMessage makeMove(int row, int col, const QString& extra = "") {
        GameMessage msg;
        msg.type        = MessageType::MOVE;
        msg.data["row"]   = row;
        msg.data["col"]   = col;
        msg.data["extra"] = extra;
        return msg;
    }

    static GameMessage makeGameStart(const QString& p1, const QString& p2,
                                     int boardSize, bool hasTimer, int timerSec) {
        GameMessage msg;
        msg.type                  = MessageType::GAME_START;
        msg.data["player1"]       = p1;
        msg.data["player2"]       = p2;
        msg.data["boardSize"]     = boardSize;
        msg.data["hasTimer"]      = hasTimer;
        msg.data["timerSeconds"]  = timerSec;
        return msg;
    }

    static GameMessage makeGameOver(const QString& winner, const QString& reason) {
        GameMessage msg;
        msg.type           = MessageType::GAME_OVER;
        msg.data["winner"] = winner;
        msg.data["reason"] = reason; // "normal" | "timeout" | "disconnect"
        return msg;
    }

    static GameMessage makeTimerUpdate(int playerIndex, int remaining) {
        GameMessage msg;
        msg.type                  = MessageType::TIMER_UPDATE;
        msg.data["playerIndex"]   = playerIndex;
        msg.data["remaining"]     = remaining;
        return msg;
    }

    static GameMessage makeChat(const QString& sender, const QString& text) {
        GameMessage msg;
        msg.type           = MessageType::CHAT_MESSAGE;
        msg.data["sender"] = sender;
        msg.data["text"]   = text;
        return msg;
    }

    static GameMessage makeError(const QString& errText) {
        GameMessage msg;
        msg.type              = MessageType::ERROR_MSG;
        msg.data["message"]   = errText;
        return msg;
    }
};

#endif // GAMEMESSAGE_H
