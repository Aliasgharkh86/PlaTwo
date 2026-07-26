#ifndef DOTSANDBOXESGAME_H
#define DOTSANDBOXESGAME_H

#include "game.h"
#include <QVector>
#include <QVariantMap>

class DotsAndBoxesGame : public Game
{
    Q_OBJECT
public:
    // به‌طور پیش‌فرض بازی را ۵ در ۵ (مربع) در نظر می‌گیریم
    explicit DotsAndBoxesGame(int boxRows = 5, int boxCols = 5, QObject *parent = nullptr);

    // توابعی که از کلاس Game باید Override شوند
    int currentPlayer() const override;
    bool makeMove(int player, const QVariant& moveData) override;
    bool isGameOver() const override;
    int getWinner() const override;
    QVariant getBoardState() const override;
    void resetGame() override;
    QString gameName() const override;

    // توابع کمکی برای خواندن وضعیت برد جهت رسم در رابط کاربری
    int getRows() const { return m_boxRows; }
    int getCols() const { return m_boxCols; }
    bool hasHLine(int r, int c) const { return (r >= 0 && r <= m_boxRows && c >= 0 && c < m_boxCols) ? m_hLines[r][c] : false; }
    bool hasVLine(int r, int c) const { return (r >= 0 && r < m_boxRows && c >= 0 && c <= m_boxCols) ? m_vLines[r][c] : false; }
    int boxOwner(int r, int c) const { return (r >= 0 && r < m_boxRows && c >= 0 && c < m_boxCols) ? m_boxes[r][c] : -1; }


private:
    int m_boxRows;
    int m_boxCols;
    int m_currentPlayer; // 0 برای بازیکن اول، 1 برای بازیکن دوم
    int m_winner;        // -2: در حال اجرا، -1: مساوی، 0: برد بازیکن اول، 1: برد بازیکن دوم
    int m_score[2];      // امتیازات دو بازیکن

    // وضعیت خطوط افقی و عمودی (آیا کشیده شده‌اند یا نه؟)
    QVector<QVector<bool>> m_hLines; // ابعاد: [boxRows + 1][boxCols]
    QVector<QVector<bool>> m_vLines; // ابعاد: [boxRows][boxCols + 1]

    // وضعیت خانه‌ها (-1: خالی، 0: مال بازیکن اول، 1: مال بازیکن دوم)
    QVector<QVector<int>> m_boxes;   // ابعاد: [boxRows][boxCols]

    // تابع کمکی برای بررسی اینکه آیا پس از یک حرکت، خانه‌ای کامل شده است یا خیر
    bool checkAndClaimBoxes(int r, int c, bool isHorizontal, int player);
};

#endif // DOTSANDBOXESGAME_H
