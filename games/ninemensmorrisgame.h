#ifndef NINEMENSMORRISGAME_H
#define NINEMENSMORRISGAME_H

#include "game.h"
#include <QVector>
#include <QVariantMap>

// ─────────────────────────────────────────────
// Nine Men's Morris
//
// تخته: ۲۴ نقطه (index صفر تا ۲۳)
// هر بازیکن ۹ مهره داره و سه فاز وجود داره:
//   ۱) Placing : تا وقتی هر دو بازیکن ۹ مهره‌شون رو نذاشتن
//   ۲) Moving  : مهره‌ها فقط به نقطه‌ی مجاور (adjacency) حرکت می‌کنن
//   ۳) Flying  : وقتی بازیکنی فقط ۳ مهره داره، می‌تونه به هر
//                نقطه‌ی خالی بپره (نه فقط مجاور)
//
// وقتی یه بازیکن یه "mill" (سه‌تایی) بسازه، باید یه مهره از
// حریف حذف کنه. این کار توی makeMove با ارسال کلید "remove"
// (وقتی m_pendingRemoval == true) انجام می‌شه.
//
// نحوه‌ی استفاده از makeMove:
//   فاز Placing:   {"place": <0..23>}
//   فاز Moving/Flying: {"from": <0..23>, "to": <0..23>}
//   حذف مهره (بعد از mill): {"remove": <0..23>}
// ─────────────────────────────────────────────

class NineMensMorrisGame : public Game
{
    Q_OBJECT

public:
    explicit NineMensMorrisGame(QObject* parent = nullptr);

    int currentPlayer() const override;
    bool makeMove(int player, const QVariant& moveData) override;
    bool isGameOver() const override;
    int getWinner() const override;
    QVariant getBoardState() const override;
    void resetGame() override;
    QString gameName() const override;

    // برای UI: آیا الان منتظر انتخاب مهره‌ی حذفی هستیم؟
    bool isPendingRemoval() const { return m_pendingRemoval; }

private:
    // ── داده‌های ثابت تخته (adjacency و mill ها) ──
    static const QVector<QVector<int>>& adjacency();
    static const QVector<QVector<int>>& mills();

    // ── وضعیت بازی ──
    int  m_board[24];          // 0 = خالی، 1 = بازیکن 0، 2 = بازیکن 1
    int  m_currentPlayer;      // 0 یا 1
    int  m_piecesToPlace[2];   // مهره‌های باقی‌مونده برای گذاشتن
    int  m_piecesOnBoard[2];   // مهره‌های روی تخته
    bool m_pendingRemoval;     // آیا منتظر حذف مهره‌ی حریفیم
    int  m_winner;             // -2 = ادامه دارد، -1 = مساوی (بدون کاربرد اینجا)، 0/1 = برنده

    // ── توابع کمکی ──
    bool isMill(int position, int player) const;
    bool allPiecesInMills(int player) const;
    bool canFly(int player) const;
    bool hasAnyLegalMove(int player) const;
    void switchPlayer();
    void checkForStalemateLoss();
};

#endif // NINEMENSMORRISGAME_H
