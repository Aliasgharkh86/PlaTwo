#ifndef FANORONAWIDGET_H
#define FANORONAWIDGET_H

#include <QWidget>
#include "../games/fanoronagame.h"

class QPushButton;

// ─────────────────────────────────────────────
// ویجت رسم و کنترل تخته‌ی Fanorona (۵×۹).
//
// مثل NineMensMorrisWidget، فقط View و ورودیه —
// منطق واقعی توی FanoronaGame::makeMove انجام می‌شه.
//
// دو تا نکته‌ی خاص Fanorona که این کلاس باید مدیریت کنه:
//   ۱) وقتی هم approach هم withdrawal ممکنه، باید از
//      کاربر بپرسیم کدوم رو می‌خواد (QMessageBox)
//   ۲) وقتی وسط یه زنجیره‌ی capture‌ایم، یه دکمه‌ی
//      "پایان نوبت" نشون داده می‌شه تا بشه داوطلبانه
//      زنجیره رو تموم کرد
// ─────────────────────────────────────────────

class FanoronaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FanoronaWidget(QWidget* parent = nullptr);
    explicit FanoronaWidget(FanoronaGame* game, int myPlayer, QWidget* parent = nullptr);

    // برای زمانی که widget با Promote to توی Designer ساخته شده
    void setGame(FanoronaGame* game, int myPlayer);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onBoardChanged();
    void onGameEnded(int winner);
    void onEndTurnClicked();

private:
    static constexpr int ROWS = 5;
    static constexpr int COLS = 9;

    FanoronaGame* m_game;
    int  m_myPlayer;
    int  m_selectedPoint;
    QPushButton* m_endTurnBtn;
    QString m_feedbackMessage; // پیام کوتاه وقتی یه حرکت رد می‌شه (مثلاً چون capture اجباریه)

    // ── هندسه‌ی تخته ──
    QPoint pointPosition(int index) const;
    int    pointAt(const QPoint& pos) const;

    // ── رسم ──
    void drawBoardLines(class QPainter& p) const;
    void drawPoints(class QPainter& p) const;
    void drawPieces(class QPainter& p) const;
    void drawStatusText(class QPainter& p) const;

    bool isMyTurn() const;
    void attemptMove(int from, int to);
    void updateEndTurnButtonVisibility();
};

#endif // FANORONAWIDGET_H
