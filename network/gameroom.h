#ifndef GAMEROOM_H
#define GAMEROOM_H

#include <QString>
#include <QTcpSocket>

enum class GameType {
    DOTS_AND_BOXES   = 0,
    NINE_MENS_MORRIS = 1,
    FANORONA         = 2
};

struct GameSettings {
    GameType gameType  = GameType::DOTS_AND_BOXES;
    int      boardSize = 6;      // 6، 7 یا 8 (فقط برای Dots and Boxes)
    bool     hasTimer  = false;
    int      timerSecs = 60;     // زمان هر نوبت به ثانیه
    int      port      = 5000;
};

struct RoomPlayer {
    QTcpSocket* socket   = nullptr;
    QString     username;
    bool        isHost   = false;
};

struct GameRoom {
    RoomPlayer   players[2];     // 0: میزبان، 1: مهمان
    GameSettings settings;
    bool         gameStarted = false;
    int          currentTurn = 0;

    bool isFull() const {
        return players[0].socket != nullptr
               && players[1].socket != nullptr;
    }

    int indexOfSocket(QTcpSocket* s) const {
        if (players[0].socket == s) return 0;
        if (players[1].socket == s) return 1;
        return -1;
    }

    QTcpSocket* opponentOf(QTcpSocket* s) const {
        int idx = indexOfSocket(s);
        if (idx < 0) return nullptr;
        return players[1 - idx].socket;
    }
};

#endif // GAMEROOM_H
