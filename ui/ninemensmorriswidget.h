#ifndef NINEMENSMORRISWIDGET_H
#define NINEMENSMORRISWIDGET_H

#include <QWidget>
#include "../games/ninemensmorrisgame.h"

// ─────────────────────────────────────────────
// ویجت رسم و کنترل تخته‌ی Nine Men's Morris.
//
// این کلاس فقط مسئول View و ورودی کاربره — هیچ منطق
// بازی‌ای اینجا نیست، همه‌چیز از طریق game->makeMove()
// انجام می‌شه. این جداسازی (Model/View) باعث می‌شه بشه
// NineMensMorrisGame رو بدون UI هم تست کرد.
//
// m_myPlayer: مشخص می‌کنه این نمونه از widget مال کدوم
// بازیکنه (0 یا 1) — فقط وقتی نوبت همین بازیکنه اجازه‌ی
// کلیک کردن میده (نسخه‌ی هر کلاینت روی سیستم خودش این
// عدد رو بر اساس این‌که Host یا Guest بوده تنظیم می‌کنه).
// ─────────────────────────────────────────────

class NineMensMorrisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NineMensMorrisWidget(NineMensMorrisGame* game,
                                  int myPlayer,
                                  QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onBoardChanged();
    void onGameEnded(int winner);

private:
    NineMensMorrisGame* m_game;
    int  m_myPlayer;      // 0 یا 1
    int  m_selectedPoint; // نقطه‌ی مبدا انتخاب‌شده در فاز Moving/Flying، -1 یعنی هیچی
    QVariantMap m_pendingMillMove; // ذخیره حرکت دوز تا زمان انتخاب مهره حذف

    // ── هندسه‌ی تخته ──
    QPoint pointPosition(int index) const;   // مختصات پیکسلی نقطه‌ی i روی widget فعلی
    int    pointAt(const QPoint& pos) const; // نزدیک‌ترین نقطه به کلیک ماوس (یا -1)

    // ── رسم ──
    void drawBoardLines(class QPainter& p) const;
    void drawPoints(class QPainter& p) const;
    void drawPieces(class QPainter& p) const;
    void drawStatusText(class QPainter& p) const;

    void drawBackground(class QPainter& p) const;
    void drawMillHighlight(class QPainter& p) const;
    void drawRemovableHint(class QPainter& p) const;
    void drawPieceCount(class QPainter& p) const;

    bool isMyTurn() const;
signals:
    void moveReadyToSend(const QVariantMap& moveData);
};

#endif // NINEMENSMORRISWIDGET_H
