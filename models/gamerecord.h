#ifndef GAMERECORD_H
#define GAMERECORD_H

#include <QString>
#include <QDateTime>

// نوع بازی به صورت رشته ذخیره میشه
// "dots_and_boxes" | "nine_mens_morris" | "fanorona"

// نتیجه بازی
enum class GameResult {
    WIN,
    LOSE,
    DRAW
};

// یه رکورد از تاریخچه بازی‌ها
struct GameRecord {
    int         id              = -1;
    int         userId          = -1;    // آیدی کاربر
    QString     gameType;               // نوع بازی
    QString     opponentUsername;       // یوزرنیم حریف
    GameResult  result          = GameResult::LOSE;
    int         myScore         = 0;    // امتیاز من
    int         opponentScore   = 0;    // امتیاز حریف
    QString     role;                   // "host" | "guest"
    QDateTime   playedAt;               // تاریخ و زمان بازی

    // متن نتیجه برای نمایش
    QString resultText() const {
        switch (result) {
        case GameResult::WIN:  return "برد";
        case GameResult::LOSE: return "باخت";
        case GameResult::DRAW: return "مساوی";
        }
        return "";
    }

    // نوع بازی به فارسی
    QString gameTypeText() const {
        if (gameType == "dots_and_boxes")   return "Dots and Boxes";
        if (gameType == "nine_mens_morris") return "Nine Men's Morris";
        if (gameType == "fanorona")         return "Fanorona";
        return gameType;
    }
};

#endif // GAMERECORD_H
