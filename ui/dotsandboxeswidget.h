#ifndef DOTSANDBOXESWIDGET_H
#define DOTSANDBOXESWIDGET_H

#include <QWidget>
#include "../games/dotsandboxesgame.h"

// ─────────────────────────────────────────────
// DotsAndBoxesWidget — رسم تخته و گرفتن کلیک ماوس
//
// منطق بازی کاملاً توی DotsAndBoxesGame هست.
// این کلاس فقط View و ورودیه.
//
// myPlayer = -1 → حالت تست محلی (hotseat)
// myPlayer = 0/1 → حالت شبکه
// ─────────────────────────────────────────────

class DotsAndBoxesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DotsAndBoxesWidget(QWidget* parent = nullptr);
    explicit DotsAndBoxesWidget(DotsAndBoxesGame* game,
                                int myPlayer,
                                QWidget* parent = nullptr);

    void setGame(DotsAndBoxesGame* game, int myPlayer);

signals:
    void moveReadyToSend(const QVariantMap& moveData);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onBoardChanged();
    void onGameEnded(int winner);

private:
    DotsAndBoxesGame* m_game     = nullptr;
    int               m_myPlayer = -1;

    // هاور — خط زیر موس (قبل از کلیک)
    bool m_hoverIsHorizontal = true;
    int  m_hoverRow          = -1;
    int  m_hoverCol          = -1;

    // ── هندسه ──
    QPoint dotPosition(int row, int col) const;
    int    cellSize() const;

    // ── تشخیص خط نزدیک به موس ──
    bool nearestLine(const QPoint& pos,
                     bool& outIsH, int& outRow, int& outCol) const;

    // ── رسم ──
    void drawBackground(QPainter& p) const;
    void drawLines(QPainter& p) const;
    void drawBoxes(QPainter& p) const;
    void drawDots(QPainter& p) const;
    void drawHover(QPainter& p) const;
    void drawScoreBar(QPainter& p) const;
    void drawStatusText(QPainter& p) const;

    bool isMyTurn() const;
};

#endif // DOTSANDBOXESWIDGET_H
