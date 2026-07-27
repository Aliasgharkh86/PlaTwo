#include "ninemensmorriswidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QVariantMap>
#include <cmath>

// ─────────────────────────────────────────────
// مختصات هر نقطه روی یه grid منطقی ۷×۷ (ستون، ردیف
// از ۰ تا ۶) — همون چیدمان استانداردی که برای adjacency
// توی NineMensMorrisGame استفاده شده:
//
//  0 ──────── 1 ──────── 2
//  │          │          │
//  │   3 ──── 4 ──── 5   │
//  │   │      │      │   │
//  │   │  6 ─ 7 ─ 8  │   │
//  9 ─10 ─11        12 ─13 ─14
//  │   │ 15 ─16 ─17  │   │
//  │   │      │      │   │
//  │   18 ──19 ──20  │
//  │          │          │
//  21 ─────── 22 ────── 23
// ─────────────────────────────────────────────

namespace {

struct GridPos { int col; int row; };

const QVector<GridPos>& gridPositions()
{
    static const QVector<GridPos> pos = {
                                         {0,0}, {3,0}, {6,0},
                                         {1,1}, {3,1}, {5,1},
                                         {2,2}, {3,2}, {4,2},
                                         {0,3}, {1,3}, {2,3},
                                         {4,3}, {5,3}, {6,3},
                                         {2,4}, {3,4}, {4,4},
                                         {1,5}, {3,5}, {5,5},
                                         {0,6}, {3,6}, {6,6},
                                         };
    return pos;
}

const QVector<QPair<int,int>>& boardLines()
{
    static const QVector<QPair<int,int>> lines = {
                                                   {0,1},{1,2},{0,9},{2,14},{1,4},
                                                   {3,4},{4,5},{3,10},{5,13},{4,7},
                                                   {6,7},{7,8},{6,11},{8,12},
                                                   {9,10},{10,11},{9,21},{11,15},
                                                   {12,13},{12,17},{13,14},{14,23},
                                                   {15,16},{16,17},{16,19},
                                                   {18,19},{10,18},{19,20},{19,22},{13,20},
                                                   {21,22},{22,23},
                                                   };
    return lines;
}

// ── رنگ‌های تم (Catppuccin Mocha) ─────────────
const QColor BG_DARK    {0x1e, 0x1e, 0x2e};       // پس‌زمینه
const QColor BG_BOARD   {0x28, 0x28, 0x3a};       // پس‌زمینه‌ی تخته
const QColor BG_STATUS  {0x18, 0x18, 0x25};       // نوار وضعیت
const QColor LINE_COLOR {0x58, 0x5b, 0x70};       // خطوط تخته
const QColor POINT_IDLE {0x45, 0x47, 0x5a};       // نقاط خالی
const QColor POINT_SEL  {0xf9, 0xe2, 0xaf};       // نقطه‌ی انتخاب‌شده (زرد)
const QColor PIECE_P0_A {0x89, 0xb4, 0xfa};       // مهره‌ی بازیکن ۰ (آبی)
const QColor PIECE_P0_B {0x1e, 0x66, 0xf5};       // سایه‌ی مهره‌ی بازیکن ۰
const QColor PIECE_P1_A {0xf3, 0x8b, 0xa8};       // مهره‌ی بازیکن ۱ (قرمز)
const QColor PIECE_P1_B {0xd2, 0x0f, 0x39};       // سایه‌ی مهره‌ی بازیکن ۱
const QColor MILL_GLOW  {0xa6, 0xe3, 0xa1, 160};  // هاله‌ی mill (سبز)
const QColor REMOVE_HINT{0xf3, 0x8b, 0xa8, 100};  // هینت حذف
const QColor TEXT_MAIN  {0xcd, 0xd6, 0xf4};       // متن اصلی
const QColor TEXT_DIM   {0x6c, 0x70, 0x86};       // متن کم‌رنگ

} // namespace

// ── Constructor (بدون تغییر) ──────────────────
NineMensMorrisWidget::NineMensMorrisWidget(NineMensMorrisGame* game,
                                           int myPlayer,
                                           QWidget* parent)
    : QWidget(parent)
    , m_game(game)
    , m_myPlayer(myPlayer)
    , m_selectedPoint(-1)
{
    setMinimumSize(420, 420);
    connect(m_game, &Game::boardChanged, this, &NineMensMorrisWidget::onBoardChanged);
    connect(m_game, &Game::gameEnded,    this, &NineMensMorrisWidget::onGameEnded);
}

void NineMensMorrisWidget::onBoardChanged()
{
    m_selectedPoint = -1;
    update();
}

void NineMensMorrisWidget::onGameEnded(int winner)
{
    Q_UNUSED(winner)
    update();
}

// ── isMyTurn (بدون تغییر) ────────────────────
bool NineMensMorrisWidget::isMyTurn() const
{
    if (m_game->isGameOver())
        return false;
    if (m_myPlayer == -1)
        return true;
    return m_game->currentPlayer() == m_myPlayer;
}

// ── pointPosition (بدون تغییر) ───────────────
QPoint NineMensMorrisWidget::pointPosition(int index) const
{
    const GridPos g = gridPositions()[index];
    const int margin  = 50;
    const int topBarH = 36;
    const int usableW = width()  - 2 * margin;
    const int usableH = height() - 2 * margin - 52 - topBarH;

    const int x = margin + (g.col * usableW) / 6;
    const int y = margin + topBarH + (g.row * usableH) / 6;
    return QPoint(x, y);
}

// ── pointAt (بدون تغییر) ─────────────────────
int NineMensMorrisWidget::pointAt(const QPoint& pos) const
{
    const int threshold = 20;
    for (int i = 0; i < 24; ++i) {
        const QPoint p = pointPosition(i);
        const int dx = p.x() - pos.x();
        const int dy = p.y() - pos.y();
        if (std::sqrt(double(dx * dx + dy * dy)) <= threshold)
            return i;
    }
    return -1;
}

// ── پس‌زمینه ──────────────────────────────────
void NineMensMorrisWidget::drawBackground(QPainter& p) const
{
    // پس‌زمینه‌ی اصلی
    p.fillRect(rect(), BG_DARK);

    // ناحیه‌ی تخته با گوشه‌های گرد — پایین‌تر از نوار امتیاز
    const int margin  = 30;
    const int statusH = 52;
    const int topBarH = 36;
    QRect boardRect(margin, margin + topBarH,
                    width() - 2*margin,
                    height() - 2*margin - statusH - topBarH);

    p.setPen(QPen(QColor(0x45, 0x47, 0x5a), 1));
    p.setBrush(BG_BOARD);
    p.drawRoundedRect(boardRect, 12, 12);
}

// ── خطوط تخته ─────────────────────────────────
void NineMensMorrisWidget::drawBoardLines(QPainter& p) const
{
    // خطوط اصلی
    p.setPen(QPen(LINE_COLOR, 1.5, Qt::SolidLine, Qt::RoundCap));
    for (const auto& line : boardLines())
        p.drawLine(pointPosition(line.first), pointPosition(line.second));

    // خطوط حلقه‌ی بیرونی ضخیم‌تر
    const QVector<QPair<int,int>> outerLines = {
        {0,1},{1,2},{2,14},{14,23},{23,22},{22,21},{21,9},{9,0}
    };
    p.setPen(QPen(QColor(0x6c, 0x70, 0x86), 2.5, Qt::SolidLine, Qt::RoundCap));
    for (const auto& line : outerLines)
        p.drawLine(pointPosition(line.first), pointPosition(line.second));
}

// ── نقاط تقاطع ────────────────────────────────
void NineMensMorrisWidget::drawPoints(QPainter& p) const
{
    for (int i = 0; i < 24; ++i) {
        const QPoint pt = pointPosition(i);

        if (i == m_selectedPoint) {
            // هاله‌ی انتخاب
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0xf9, 0xe2, 0xaf, 60));
            p.drawEllipse(pt, 14, 14);
            // نقطه
            p.setPen(QPen(POINT_SEL, 2));
            p.setBrush(POINT_SEL);
            p.drawEllipse(pt, 6, 6);
        } else {
            // نقطه‌ی عادی با خط کوچیک
            p.setPen(QPen(LINE_COLOR, 1));
            p.setBrush(POINT_IDLE);
            p.drawEllipse(pt, 4, 4);
        }
    }
}

// ── هاله‌ی mill ───────────────────────────────
void NineMensMorrisWidget::drawMillHighlight(QPainter& p) const
{
    if (!m_game->isPendingRemoval()) return;

    // همه‌ی مهره‌های بازیکنی که mill ساخته رو با هاله‌ی سبز نشون بده
    const QVariantMap state = m_game->getBoardState().toMap();
    const QVariantList board = state.value("board").toList();
    const int millerVal = m_game->currentPlayer() + 1;

    p.setPen(Qt::NoPen);
    for (int i = 0; i < 24; ++i) {
        if (board.at(i).toInt() == millerVal) {
            p.setBrush(MILL_GLOW);
            p.drawEllipse(pointPosition(i), 16, 16);
        }
    }
}

// ── هینت حذف مهره ────────────────────────────
void NineMensMorrisWidget::drawRemovableHint(QPainter& p) const
{
    if (!m_game->isPendingRemoval() || !isMyTurn()) return;

    // مهره‌های حریف رو با حلقه‌ی قرمز هایلایت کن
    const QVariantMap state = m_game->getBoardState().toMap();
    const QVariantList board = state.value("board").toList();
    const int opponentVal = (m_game->currentPlayer() == 0) ? 2 : 1;

    p.setPen(QPen(PIECE_P1_A, 2, Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    for (int i = 0; i < 24; ++i) {
        if (board.at(i).toInt() == opponentVal)
            p.drawEllipse(pointPosition(i), 16, 16);
    }
}

// ── مهره‌ها ────────────────────────────────────
void NineMensMorrisWidget::drawPieces(QPainter& p) const
{
    const QVariantMap state = m_game->getBoardState().toMap();
    const QVariantList board = state.value("board").toList();

    for (int i = 0; i < 24; ++i) {
        const int val = board.at(i).toInt();
        if (val == 0) continue;

        const QPoint pt = pointPosition(i);
        const bool isP0 = (val == 1);

        // سایه‌ی مهره
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 60));
        p.drawEllipse(pt + QPoint(2, 3), 13, 13);

        // مهره‌ی اصلی
        QRadialGradient grad(pt - QPoint(3, 4), 14);
        grad.setColorAt(0.0, isP0 ? PIECE_P0_A : PIECE_P1_A);
        grad.setColorAt(1.0, isP0 ? PIECE_P0_B : PIECE_P1_B);

        p.setPen(QPen(isP0 ? PIECE_P0_B : PIECE_P1_B, 1.5));
        p.setBrush(grad);
        p.drawEllipse(pt, 13, 13);

        // نقطه‌ی براق روی مهره
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 70));
        p.drawEllipse(pt - QPoint(4, 4), 5, 5);
    }
}

// ── شمارش مهره‌ها ─────────────────────────────
void NineMensMorrisWidget::drawPieceCount(QPainter& p) const
{
    const QVariantMap state = m_game->getBoardState().toMap();
    const int p0Place = state.value("piecesToPlace0").toInt();
    const int p1Place = state.value("piecesToPlace1").toInt();
    const int p0Board = state.value("piecesOnBoard0").toInt();
    const int p1Board = state.value("piecesOnBoard1").toInt();

    // نوار بالای صفحه — بالای تخته
    const int topBarH = 36;
    QRect topBar(0, 0, width(), topBarH);
    p.fillRect(topBar, BG_STATUS);
    p.setPen(QPen(QColor(0x31, 0x32, 0x44), 1));
    p.drawLine(0, topBarH, width(), topBarH);

    QFont f = p.font();
    f.setPointSize(9);
    f.setBold(true);
    p.setFont(f);

    // بازیکن ۰ — چپ
    p.setPen(PIECE_P0_A);
    p.drawText(QRect(10, 0, width() / 2 - 10, topBarH),
               Qt::AlignVCenter | Qt::AlignLeft,
               QString("⬤ بازیکن ۱  روی تخته: %1  مانده: %2")
                   .arg(p0Board).arg(p0Place));

    // بازیکن ۱ — راست
    p.setPen(PIECE_P1_A);
    p.drawText(QRect(width() / 2, 0, width() / 2 - 10, topBarH),
               Qt::AlignVCenter | Qt::AlignRight,
               QString("⬤ بازیکن ۲  روی تخته: %1  مانده: %2")
                   .arg(p1Board).arg(p1Place));
}

// ── نوار وضعیت ────────────────────────────────
void NineMensMorrisWidget::drawStatusText(QPainter& p) const
{
    const QVariantMap state = m_game->getBoardState().toMap();
    QString text;
    QColor  textColor = TEXT_MAIN;

    if (m_game->isGameOver()) {
        const int winner = m_game->getWinner();
        text      = (winner == m_myPlayer) ? "🎉  شما بردید!" : "😔  حریف برد.";
        textColor = (winner == m_myPlayer) ? QColor(0xa6, 0xe3, 0xa1)
                                           : QColor(0xf3, 0x8b, 0xa8);
    } else if (m_game->isPendingRemoval()) {
        if (isMyTurn()) {
            text      = "🔴  شما mill ساختید! یکی از مهره‌های حریف را انتخاب کنید.";
            textColor = QColor(0xf3, 0x8b, 0xa8);
        } else {
            text      = "⏳  حریف mill ساخت. در انتظار حذف مهره...";
            textColor = TEXT_DIM;
        }
    } else {
        const int effectivePlayer = (m_myPlayer == -1) ? m_game->currentPlayer() : m_myPlayer;
        const int toPlace = state.value(effectivePlayer == 0 ? "piecesToPlace0"
                                                             : "piecesToPlace1").toInt();
        if (m_myPlayer == -1) {
            text = QString("نوبت بازیکن %1 — %2")
                       .arg(m_game->currentPlayer() + 1)
                       .arg(toPlace > 0 ? "یک مهره بگذارید." : "یک مهره را حرکت دهید.");
        } else if (isMyTurn()) {
            text      = toPlace > 0 ? "✨  نوبت شماست — یک مهره بگذارید."
                               : "✨  نوبت شماست — یک مهره را حرکت دهید.";
            textColor = QColor(0x89, 0xb4, 0xfa);
        } else {
            text      = "⏳  در انتظار حرکت حریف...";
            textColor = TEXT_DIM;
        }
    }

    // پنل نوار وضعیت
    const QRect statusRect(0, height() - 46, width(), 46);
    p.fillRect(statusRect, BG_STATUS);

    // خط جداکننده
    p.setPen(QPen(QColor(0x31, 0x32, 0x44), 1));
    p.drawLine(0, height() - 46, width(), height() - 46);

    // متن
    p.setPen(textColor);
    QFont f = p.font();
    f.setPointSize(11);
    f.setBold(isMyTurn() && !m_game->isGameOver());
    p.setFont(f);
    p.drawText(statusRect, Qt::AlignCenter, text);
}

// ── paintEvent ────────────────────────────────
void NineMensMorrisWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    drawBackground(p);
    drawBoardLines(p);
    drawMillHighlight(p);
    drawRemovableHint(p);
    drawPoints(p);
    drawPieces(p);
    drawPieceCount(p);
    drawStatusText(p);
}

// ── mousePressEvent (بدون تغییر) ─────────────
void NineMensMorrisWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_game || !isMyTurn())
        return;

    const int effectivePlayer = (m_myPlayer == -1) ? m_game->currentPlayer() : m_myPlayer;
    const int clicked = pointAt(event->pos());
    if (clicked < 0 || clicked > 23)
        return;

    // ── ۱. فاز حذف مهره‌ی حریف بعد از تشکیل mill ──
    if (m_game->isPendingRemoval()) {
        QVariantMap move;
        move["remove"] = clicked;
        if (m_game->makeMove(effectivePlayer, move)) {
            m_selectedPoint = -1;
            move["keepTurn"] = false;
            emit moveReadyToSend(move);
        }
        update();
        return;
    }

    const QVariantMap state = m_game->getBoardState().toMap();
    const int toPlace = state.value(effectivePlayer == 0 ? "piecesToPlace0"
                                                         : "piecesToPlace1").toInt();

    // ── ۲. فاز قرار دادن مهره (Placing) ──
    if (toPlace > 0) {
        QVariantMap move;
        move["place"] = clicked;
        if (m_game->makeMove(effectivePlayer, move)) {
            m_selectedPoint = -1;
            move["keepTurn"] = m_game->isPendingRemoval();
            emit moveReadyToSend(move);
        }
        update();
        return;
    }

    // ── ۳. فاز حرکت دادن یا پرواز (Moving / Flying) ──
    const QVariantList board  = state.value("board").toList();
    const int clickedVal      = board.at(clicked).toInt();
    const int myVal           = effectivePlayer + 1;

    if (m_selectedPoint == -1) {
        if (clickedVal == myVal)
            m_selectedPoint = clicked;
        update();
        return;
    }

    if (clicked == m_selectedPoint) {
        m_selectedPoint = -1;
        update();
        return;
    }

    if (clickedVal == myVal) {
        m_selectedPoint = clicked;
        update();
        return;
    }

    if (clickedVal == 0) {
        QVariantMap move;
        move["from"] = m_selectedPoint;
        move["to"]   = clicked;
        if (m_game->makeMove(effectivePlayer, move)) {
            m_selectedPoint = -1;
            move["keepTurn"] = m_game->isPendingRemoval();
            emit moveReadyToSend(move);
        }
    }

    update();
}
