#ifndef DOTSANDBOXESGAME_H
#define DOTSANDBOXESGAME_H

#include "game.h"
#include <QVector>
#include <QVariantMap>

// ─────────────────────────────────────────────
// Dots and Boxes
//
// تخته: شبکه‌ای از نقاط به اندازه‌ی boardSize × boardSize
// (boardSize از GameSettings میاد، بین ۶ تا ۸)
//
// خطوط:
//   افقی  (hLines): boardSize ردیف × (boardSize-1) ستون
//   عمودی (vLines): (boardSize-1) ردیف × boardSize ستون
//
// جعبه‌ها: (boardSize-1) × (boardSize-1) جعبه
//   یه جعبه‌ی (row,col) وقتی کامل می‌شه که ۴ ضلع داشته باشه:
//     - بالا : hLines[row][col]
//     - پایین: hLines[row+1][col]
//     - چپ  : vLines[row][col]
//     - راست : vLines[row][col+1]
//
// وقتی بازیکنی با یه خط یه جعبه رو کامل می‌کنه، امتیاز
// می‌گیره و دوباره حرکت می‌کنه (نوبتش تکرار می‌شه).
//
// نحوه‌ی استفاده از makeMove:
//   {"isHorizontal": true,  "row": r, "col": c}  ← خط افقی
//   {"isHorizontal": false, "row": r, "col": c}  ← خط عمودی
// ─────────────────────────────────────────────

class DotsAndBoxesGame : public Game
{
    Q_OBJECT

public:
    explicit DotsAndBoxesGame(int boardSize = 6, QObject* parent = nullptr);

    int     currentPlayer() const override;
    bool    makeMove(int player, const QVariant& moveData) override;
    bool    isGameOver() const override;
    int     getWinner() const override;
    QVariant getBoardState() const override;
    void    resetGame() override;
    QString gameName() const override;

    int  boardSize() const { return m_boardSize; }
    int  score(int player) const { return m_scores[player]; }

private:
    int  m_boardSize;      // تعداد نقطه در هر ضلع (مثلاً ۶)
    int  m_N;              // = boardSize (برای کوتاهی)

    // خطوط — true یعنی کشیده شده
    QVector<QVector<bool>> m_hLines; // [row][col]: row در [0,N-1], col در [0,N-2]
    QVector<QVector<bool>> m_vLines; // [row][col]: row در [0,N-2], col در [0,N-1]

    // جعبه‌ها — 0=خالی، 1=بازیکن ۰، 2=بازیکن ۱
    QVector<QVector<int>>  m_boxes;  // [row][col]: row,col در [0,N-2]

    int m_scores[2];
    int m_currentPlayer;
    int m_winner;          // -2=ادامه، -1=مساوی، 0/1=برنده

    // ── توابع کمکی ──
    bool isValidHLine(int row, int col) const;
    bool isValidVLine(int row, int col) const;
    bool isBoxComplete(int boxRow, int boxCol) const;

    // چک می‌کنه آیا کشیدن یه خط باعث کامل شدن جعبه‌ای می‌شه
    // و اگه شد، جعبه رو به player نسبت می‌ده و امتیاز می‌ده
    // برمی‌گردونه تعداد جعبه‌های تازه کامل‌شده
    int claimAdjacentBoxes(bool isHorizontal, int row, int col, int player);

    void checkGameOver();
};

#endif // DOTSANDBOXESGAME_H
