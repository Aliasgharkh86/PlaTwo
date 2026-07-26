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
        {0,0}, {3,0}, {6,0},        // 0,1,2
        {1,1}, {3,1}, {5,1},        // 3,4,5
        {2,2}, {3,2}, {4,2},        // 6,7,8
        {0,3}, {1,3}, {2,3},        // 9,10,11
        {4,3}, {5,3}, {6,3},        // 12,13,14
        {2,4}, {3,4}, {4,4},        // 15,16,17
        {1,5}, {3,5}, {5,5},        // 18,19,20
        {0,6}, {3,6}, {6,6},        // 21,22,23
    };
    return pos;
}

// خط‌هایی که باید روی تخته رسم بشن (صرفاً برای نمایش —
// منطق مجاز بودن حرکت داخل NineMensMorrisGame چک می‌شه)
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

} // namespace

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
    m_selectedPoint = -1; // بعد از هر حرکت، انتخاب موقت رو پاک کن
    update();
}

void NineMensMorrisWidget::onGameEnded(int winner)
{
    Q_UNUSED(winner)
    update();
}

bool NineMensMorrisWidget::isMyTurn() const
{
    if (m_game->isGameOver())
        return false;
    // myPlayer == -1 یعنی «حالت تست محلی»: هر دو بازیکن روی همین
    // یه پنجره به‌نوبت کلیک می‌کنن، نوبت رو خود Game مدیریت می‌کنه
    if (m_myPlayer == -1)
        return true;
    return m_game->currentPlayer() == m_myPlayer;
}

QPoint NineMensMorrisWidget::pointPosition(int index) const
{
    const GridPos g = gridPositions()[index];
    const int margin = 40;
    const int usableW = width()  - 2 * margin;
    const int usableH = height() - 2 * margin - 30; // ۳۰px برای متن وضعیت پایین صفحه

    const int x = margin + (g.col * usableW) / 6;
    const int y = margin + (g.row * usableH) / 6;
    return QPoint(x, y);
}

int NineMensMorrisWidget::pointAt(const QPoint& pos) const
{
    const int threshold = 18;
    for (int i = 0; i < 24; ++i) {
        const QPoint p = pointPosition(i);
        const int dx = p.x() - pos.x();
        const int dy = p.y() - pos.y();
        if (std::sqrt(double(dx * dx + dy * dy)) <= threshold)
            return i;
    }
    return -1;
}

void NineMensMorrisWidget::drawBoardLines(QPainter& p) const
{
    p.setPen(QPen(Qt::black, 2));
    for (const auto& line : boardLines()) {
        p.drawLine(pointPosition(line.first), pointPosition(line.second));
    }
}

void NineMensMorrisWidget::drawPoints(QPainter& p) const
{
    p.setPen(QPen(Qt::black, 1));
    p.setBrush(Qt::white);
    for (int i = 0; i < 24; ++i) {
        const QPoint pt = pointPosition(i);

        if (i == m_selectedPoint) {
            p.setBrush(QColor(255, 230, 120)); // نقطه‌ی انتخاب‌شده هایلایت بشه
            p.drawEllipse(pt, 8, 8);
            p.setBrush(Qt::white);
        } else {
            p.drawEllipse(pt, 5, 5);
        }
    }
}

void NineMensMorrisWidget::drawPieces(QPainter& p) const
{
    const QVariantMap state = m_game->getBoardState().toMap();
    const QVariantList board = state.value("board").toList();

    for (int i = 0; i < 24; ++i) {
        const int val = board.at(i).toInt();
        if (val == 0)
            continue;

        const QPoint pt = pointPosition(i);
        p.setPen(QPen(Qt::black, 2));
        p.setBrush(val == 1 ? QColor(40, 40, 40) : QColor(220, 60, 60));
        p.drawEllipse(pt, 12, 12);
    }
}

void NineMensMorrisWidget::drawStatusText(QPainter& p) const
{
    const QVariantMap state = m_game->getBoardState().toMap();
    QString text;

    if (m_game->isGameOver()) {
        const int winner = m_game->getWinner();
        text = (winner == m_myPlayer)
                   ? "🎉  شما بردید!"
                   : "😔  حریف برد.";
    } else if (m_game->isPendingRemoval()) {
        if (isMyTurn()) {
            // این بازیکن mill ساخته — باید یه مهره‌ی حریف حذف کنه
            text = "🔴  شما mill ساختید! یکی از مهره‌های حریف را انتخاب کنید.";
        } else {
            // حریف mill ساخته — باید صبر کنیم
            text = "⏳  حریف mill ساخت. در انتظار حذف مهره...";
        }
    } else {
        const int effectivePlayer = (m_myPlayer == -1) ? m_game->currentPlayer() : m_myPlayer;
        const int toPlace = state.value(effectivePlayer == 0 ? "piecesToPlace0" : "piecesToPlace1").toInt();
        if (m_myPlayer == -1) {
            text = QString("نوبت بازیکن %1 — %2")
                       .arg(m_game->currentPlayer() + 1)
                       .arg(toPlace > 0 ? "یک مهره بگذارید." : "یک مهره را حرکت دهید.");
        } else if (isMyTurn()) {
            text = toPlace > 0 ? "نوبت شماست — یک مهره بگذارید."
                               : "نوبت شماست — یک مهره را حرکت دهید.";
        } else {
            text = "در انتظار حرکت حریف...";
        }
    }

    p.setPen(Qt::black);
    QFont f = p.font();
    f.setPointSize(11);
    p.setFont(f);
    p.drawText(QRect(0, height() - 28, width(), 28), Qt::AlignCenter, text);
}

void NineMensMorrisWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBoardLines(p);
    drawPoints(p);
    drawPieces(p);
    drawStatusText(p);
}
void NineMensMorrisWidget::mousePressEvent(QMouseEvent* event)
{
    // ۱. اگر موتور بازی وجود نداشت یا نوبت این بازیکن نبود، کلیک نادیده گرفته می‌شود
    if (!m_game || !isMyTurn())
        return;

    // در حالت تست محلی (myPlayer == -1) حرکت به نام بازیکنی که نوبتش است ثبت می‌شود؛
    // در حالت شبکه m_myPlayer ثابت است (0 یا 1).
    const int effectivePlayer = (m_myPlayer == -1) ? m_game->currentPlayer() : m_myPlayer;

    const int clicked = pointAt(event->pos());
    if (clicked < 0 || clicked > 23)
        return;

    // ── ۱. فاز حذف مهره‌ی حریف بعد از تشکیل mill ──────────────────────────────
    if (m_game->isPendingRemoval()) {
        QVariantMap move;
        move["remove"] = clicked;

        if (m_game->makeMove(effectivePlayer, move)) {
            m_selectedPoint = -1; // ریست هایلایت انتخاب
            move["keepTurn"] = false; // بعد از حذف مهره، نوبت باید به حریف منتقل بشه
            emit moveReadyToSend(move); // ارسال حرکت حذف به شبکه
        }
        update();
        return;
    }

    const QVariantMap state = m_game->getBoardState().toMap();
    const int toPlace = state.value(effectivePlayer == 0 ? "piecesToPlace0" : "piecesToPlace1").toInt();

    // ── ۲. فاز قرار دادن مهره (Placing) ───────────────────────────────────────
    if (toPlace > 0) {
        QVariantMap move;
        move["place"] = clicked;

        if (m_game->makeMove(effectivePlayer, move)) {
            m_selectedPoint = -1; // ریست هایلایت انتخاب
            // نوبت رو فقط وقتی نگه دار که mill تشکیل شده باشه (منتظر حذف مهره)
            move["keepTurn"] = m_game->isPendingRemoval();
            emit moveReadyToSend(move); // ارسال حرکت گذاشتن مهره به شبکه
        }
        update();
        return;
    }

    // ── ۳. فاز حرکت دادن یا پرواز (Moving / Flying) ───────────────────────────
    const QVariantList board = state.value("board").toList();
    const int clickedVal     = board.at(clicked).toInt();
    const int myVal          = effectivePlayer + 1; // 1 برای بازیکن 0، 2 برای بازیکن 1

    // ۳-۱. کلیک اول: انتخاب مهره خودی برای جابه‌جایی
    if (m_selectedPoint == -1) {
        if (clickedVal == myVal) {
            m_selectedPoint = clicked;
        }
        update();
        return;
    }

    // ۳-۲. کلیک روی همان مهره: لغو انتخاب
    if (clicked == m_selectedPoint) {
        m_selectedPoint = -1;
        update();
        return;
    }

    // ۳-۳. کلیک روی مهره دیگر خودی: تغییر مهره انتخاب‌شده
    if (clickedVal == myVal) {
        m_selectedPoint = clicked;
        update();
        return;
    }

    // ۳-۴. کلیک روی خانه خالی مقصد: جابه‌جایی مهره از مبدا به مقصد
    if (clickedVal == 0) {
        QVariantMap move;
        move["from"] = m_selectedPoint;
        move["to"]   = clicked;

        if (m_game->makeMove(effectivePlayer, move)) {
            m_selectedPoint = -1; // ریست هایلایت بعد از حرکت موفق
            // نوبت رو فقط وقتی نگه دار که mill تشکیل شده باشه (منتظر حذف مهره)
            move["keepTurn"] = m_game->isPendingRemoval();
            emit moveReadyToSend(move); // ارسال حرکت جابه‌جایی به شبکه
        }
    }

    update();
}