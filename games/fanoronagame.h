#ifndef FANORONAGAME_H
#define FANORONAGAME_H

#include "game.h"
#include <QObject>
#include <QVector>
#include <QVariant>
#include <QMap>
#include <QString>

class FanoronaGame : public Game
{
    Q_OBJECT

public:
    explicit FanoronaGame(QObject* parent = nullptr);

    static const int ROWS = 5;
    static const int COLS = 9;
    static const int CELLS = ROWS * COLS;

    void resetGame() override;
    int currentPlayer() const override;
    QString gameName() const override;
    bool isGameOver() const override;
    int getWinner() const override;
    QVariant getBoardState() const override;

    Q_INVOKABLE bool makeMove(int player, const QVariant& moveData) override;
    Q_INVOKABLE bool moveNeedsCaptureTypeChoice(int from, int to) const;

private:
    int m_board[CELLS];
    int m_currentPlayer;
    int m_piecesOnBoard[2];
    int m_winner; // -2: Playing, -1: Draw, 0: Player 0, 1: Player 1

    bool m_chainInProgress;
    int m_chainPosition;
    QVector<int> m_visitedThisTurn;
    int m_lastDirRow;
    int m_lastDirCol;

    int m_halfMoveClock;                  // برای قانون ۵۰ حرکت بدون زدن
    QMap<QString, int> m_positionHistory; // برای قانون تکرار ۳‌باره وضعیت

    static bool inBounds(int row, int col);
    static bool hasDiagonal(int row, int col);
    static bool isValidDirection(int fromRow, int fromCol, int dRow, int dCol);

    QVector<int> findCaptureLine(int row, int col, int dRow, int dCol, int opponentVal) const;
    bool moveHasCapture(int from, int to) const;
    bool hasAnyCaptureAvailable(int player) const;
    bool hasChainCaptureAvailable(int fromIdx) const; // متد اصلاح‌شده زنجیره
    bool hasAnyLegalMove(int player) const;

    void recordBoardState();
    void switchPlayer();
    void endTurn();
    void checkWinCondition();
};

#endif // FANORONAGAME_H