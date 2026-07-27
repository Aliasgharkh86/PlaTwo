#include "dotsandboxeswidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QVariantMap>
#include <cmath>

// ── رنگ‌های تم ────────────────────────────────
namespace {
const QColor BG_DARK   {0x1e, 0x1e, 0x2e};
const QColor BG_BOARD  {0x28, 0x28, 0x3a};
const QColor BG_STATUS {0x18, 0x18, 0x25};
const QColor DOT_COLOR {0xcd, 0xd6, 0xf4};
const QColor LINE_DRAWN{0x58, 0x5b, 0x70};
const QColor LINE_EMPTY{0x31, 0x32, 0x44};
const QColor P0_COLOR  {0x89, 0xb4, 0xfa}; // آبی
const QColor P1_COLOR  {0xf3, 0x8b, 0xa8}; // قرمز
const QColor BOX_P0    {0x89, 0xb4, 0xfa, 60};
const QColor BOX_P1    {0xf3, 0x8b, 0xa8, 60};
const QColor HOVER_P0  {0x89, 0xb4, 0xfa, 180};
const QColor HOVER_P1  {0xf3, 0x8b, 0xa8, 180};
const QColor TEXT_MAIN {0xcd, 0xd6, 0xf4};
const QColor TEXT_DIM  {0x6c, 0x70, 0x86};
}

// ── Constructors ──────────────────────────────

DotsAndBoxesWidget::DotsAndBoxesWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(400, 440);
    setMouseTracking(true);
}

DotsAndBoxesWidget::DotsAndBoxesWidget(DotsAndBoxesGame* game,
                                       int myPlayer,
                                       QWidget* parent)
    : DotsAndBoxesWidget(parent)
{
    setGame(game, myPlayer);
}

void DotsAndBoxesWidget::setGame(DotsAndBoxesGame* game, int myPlayer)
{
    if (m_game) m_game->disconnect(this);

    m_game     = game;
    m_myPlayer = myPlayer;
    m_hoverRow = -1;
    m_hoverCol = -1;

    connect(m_game, &Game::boardChanged, this, &DotsAndBoxesWidget::onBoardChanged);
    connect(m_game, &Game::gameEnded,    this, &DotsAndBoxesWidget::onGameEnded);
    update();
}

void DotsAndBoxesWidget::onBoardChanged() { update(); }
void DotsAndBoxesWidget::onGameEnded(int) { m_hoverRow = -1; update(); }

bool DotsAndBoxesWidget::isMyTurn() const
{
    if (!m_game || m_game->isGameOver()) return false;
    if (m_myPlayer == -1) return true;
    return m_game->currentPlayer() == m_myPlayer;
}

// ── هندسه ─────────────────────────────────────

int DotsAndBoxesWidget::cellSize() const
{
    if (!m_game) return 50;
    const int N    = m_game->boardSize();
    const int statusH = 80;
    const int margin  = 40;
    const int available = qMin(width(), height() - statusH) - 2 * margin;
    return qMax(20, available / (N - 1));
}

QPoint DotsAndBoxesWidget::dotPosition(int row, int col) const
{
    const int N    = m_game ? m_game->boardSize() : 6;
    const int cs   = cellSize();
    const int totalW = cs * (N - 1);
    const int totalH = cs * (N - 1);
    const int offX = (width()  - totalW) / 2;
    const int topBarH = 36;
    const int offY = (height() - totalH - 70) / 2 + topBarH / 2;
    return QPoint(offX + col * cs, offY + row * cs);
}

// ── تشخیص نزدیک‌ترین خط به موس ───────────────

bool DotsAndBoxesWidget::nearestLine(const QPoint& pos,
                                     bool& outIsH,
                                     int&  outRow,
                                     int&  outCol) const
{
    if (!m_game) return false;
    const int N         = m_game->boardSize();
    const int threshold = cellSize() / 2;

    double bestDist = threshold + 1;
    bool   found    = false;

    // خطوط افقی
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N - 1; ++c) {
            const QPoint a = dotPosition(r, c);
            const QPoint b = dotPosition(r, c + 1);
            const QPoint mid((a.x() + b.x()) / 2, (a.y() + b.y()) / 2);
            const double d = std::hypot(pos.x() - mid.x(), pos.y() - mid.y());
            if (d < bestDist) {
                bestDist = d; outIsH = true;
                outRow = r; outCol = c; found = true;
            }
        }
    }

    // خطوط عمودی
    for (int r = 0; r < N - 1; ++r) {
        for (int c = 0; c < N; ++c) {
            const QPoint a = dotPosition(r, c);
            const QPoint b = dotPosition(r + 1, c);
            const QPoint mid((a.x() + b.x()) / 2, (a.y() + b.y()) / 2);
            const double d = std::hypot(pos.x() - mid.x(), pos.y() - mid.y());
            if (d < bestDist) {
                bestDist = d; outIsH = false;
                outRow = r; outCol = c; found = true;
            }
        }
    }
    return found;
}

// ── رویدادهای ماوس ────────────────────────────

void DotsAndBoxesWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!isMyTurn()) { m_hoverRow = -1; update(); return; }

    bool isH; int r, c;
    if (nearestLine(event->pos(), isH, r, c)) {
        // فقط خط‌های کشیده‌نشده رو هایلایت کن
        const QVariantMap state = m_game->getBoardState().toMap();
        const int N = m_game->boardSize();
        bool already = false;
        if (isH) {
            int idx = r * (N - 1) + c;
            already = state.value("hLines").toList().at(idx).toBool();
        } else {
            int idx = r * N + c;
            already = state.value("vLines").toList().at(idx).toBool();
        }
        if (!already) {
            m_hoverIsHorizontal = isH;
            m_hoverRow = r; m_hoverCol = c;
            update(); return;
        }
    }
    m_hoverRow = -1;
    update();
}

void DotsAndBoxesWidget::mousePressEvent(QMouseEvent* event)
{
    if (!isMyTurn() || m_hoverRow == -1) return;

    const int effectivePlayer = (m_myPlayer == -1)
                                    ? m_game->currentPlayer()
                                    : m_myPlayer;

    QVariantMap move;
    move["isHorizontal"] = m_hoverIsHorizontal;
    move["row"]          = m_hoverRow;
    move["col"]          = m_hoverCol;

    if (m_game->makeMove(effectivePlayer, move))
        emit moveReadyToSend(move);

    m_hoverRow = -1;
    update();
}

void DotsAndBoxesWidget::leaveEvent(QEvent*)
{
    m_hoverRow = -1;
    update();
}

// ── رسم ──────────────────────────────────────

void DotsAndBoxesWidget::drawBackground(QPainter& p) const
{
    p.fillRect(rect(), BG_DARK);

    const int statusH = 70;
    const int topBarH = 36;
    QRect boardRect(20, 20 + topBarH, width() - 40, height() - 40 - statusH - topBarH);
    p.setPen(QPen(QColor(0x45, 0x47, 0x5a), 1));
    p.setBrush(BG_BOARD);
    p.drawRoundedRect(boardRect, 12, 12);
}

void DotsAndBoxesWidget::drawBoxes(QPainter& p) const
{
    if (!m_game) return;
    const QVariantMap state = m_game->getBoardState().toMap();
    const QVariantList boxes = state.value("boxes").toList();
    const int N = m_game->boardSize();

    for (int r = 0; r < N - 1; ++r) {
        for (int c = 0; c < N - 1; ++c) {
            const int owner = boxes.at(r * (N - 1) + c).toInt();
            if (owner == 0) continue;

            const QPoint tl = dotPosition(r, c);
            const QPoint br = dotPosition(r + 1, c + 1);
            const QRect boxRect(tl, br);

            // رنگ جعبه
            p.setPen(Qt::NoPen);
            p.setBrush(owner == 1 ? BOX_P0 : BOX_P1);
            p.drawRect(boxRect);

            // حرف بازیکن وسط جعبه
            p.setPen(owner == 1 ? P0_COLOR : P1_COLOR);
            QFont f = p.font();
            f.setPointSize(10);
            f.setBold(true);
            p.setFont(f);
            p.drawText(boxRect, Qt::AlignCenter,
                       owner == 1 ? "A" : "B");
        }
    }
}

void DotsAndBoxesWidget::drawLines(QPainter& p) const
{
    if (!m_game) return;
    const QVariantMap state  = m_game->getBoardState().toMap();
    const QVariantList hList = state.value("hLines").toList();
    const QVariantList vList = state.value("vLines").toList();
    const int N = m_game->boardSize();

    // خطوط افقی
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N - 1; ++c) {
            const bool drawn = hList.at(r * (N - 1) + c).toBool();
            p.setPen(QPen(drawn ? LINE_DRAWN : LINE_EMPTY,
                          drawn ? 3.0 : 1.5,
                          drawn ? Qt::SolidLine : Qt::DotLine,
                          Qt::RoundCap));
            p.drawLine(dotPosition(r, c), dotPosition(r, c + 1));
        }
    }

    // خطوط عمودی
    for (int r = 0; r < N - 1; ++r) {
        for (int c = 0; c < N; ++c) {
            const bool drawn = vList.at(r * N + c).toBool();
            p.setPen(QPen(drawn ? LINE_DRAWN : LINE_EMPTY,
                          drawn ? 3.0 : 1.5,
                          drawn ? Qt::SolidLine : Qt::DotLine,
                          Qt::RoundCap));
            p.drawLine(dotPosition(r, c), dotPosition(r + 1, c));
        }
    }
}

void DotsAndBoxesWidget::drawHover(QPainter& p) const
{
    if (m_hoverRow < 0 || !m_game) return;

    const QColor hoverColor = (m_game->currentPlayer() == 0) ? HOVER_P0 : HOVER_P1;
    p.setPen(QPen(hoverColor, 3.5, Qt::SolidLine, Qt::RoundCap));

    if (m_hoverIsHorizontal)
        p.drawLine(dotPosition(m_hoverRow, m_hoverCol),
                   dotPosition(m_hoverRow, m_hoverCol + 1));
    else
        p.drawLine(dotPosition(m_hoverRow,     m_hoverCol),
                   dotPosition(m_hoverRow + 1, m_hoverCol));
}

void DotsAndBoxesWidget::drawDots(QPainter& p) const
{
    if (!m_game) return;
    const int N = m_game->boardSize();

    p.setPen(Qt::NoPen);
    p.setBrush(DOT_COLOR);
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            p.drawEllipse(dotPosition(r, c), 5, 5);
}

void DotsAndBoxesWidget::drawScoreBar(QPainter& p) const
{
    if (!m_game) return;
    const QVariantMap state = m_game->getBoardState().toMap();
    const int s0 = state.value("score0").toInt();
    const int s1 = state.value("score1").toInt();

    // نوار بالای صفحه — بالای تخته
    const int topBarH = 36;
    QRect topBar(0, 0, width(), topBarH);
    p.fillRect(topBar, BG_STATUS);
    p.setPen(QPen(QColor(0x31, 0x32, 0x44), 1));
    p.drawLine(0, topBarH, width(), topBarH);

    QFont f = p.font();
    f.setPointSize(10);
    f.setBold(true);
    p.setFont(f);

    // بازیکن ۰ (آبی) — چپ
    p.setPen(P0_COLOR);
    p.drawText(QRect(30, 0, 200, topBarH), Qt::AlignVCenter | Qt::AlignLeft,
               QString("● بازیکن ۱   %1 جعبه").arg(s0));

    // بازیکن ۱ (قرمز) — راست
    p.setPen(P1_COLOR);
    p.drawText(QRect(width() - 230, 0, 200, topBarH), Qt::AlignVCenter | Qt::AlignRight,
               QString("● بازیکن ۲   %1 جعبه").arg(s1));
}

void DotsAndBoxesWidget::drawStatusText(QPainter& p) const
{
    if (!m_game) return;

    QString text;
    QColor  color = TEXT_MAIN;

    if (m_game->isGameOver()) {
        const int w = m_game->getWinner();
        if (w == -1) {
            text  = "🤝  مساوی!";
        } else if (w == m_myPlayer || m_myPlayer == -1) {
            text  = (m_myPlayer == -1)
            ? QString("🎉  بازیکن %1 برد!").arg(w + 1)
            : "🎉  شما بردید!";
            color = QColor(0xa6, 0xe3, 0xa1);
        } else {
            text  = "😔  حریف برد.";
            color = P1_COLOR;
        }
    } else if (isMyTurn()) {
        text  = "✨  نوبت شماست — روی یک خط کلیک کنید.";
        color = P0_COLOR;
    } else {
        text  = "⏳  در انتظار حرکت حریف...";
        color = TEXT_DIM;
    }

    const QRect statusRect(0, height() - 46, width(), 46);
    p.fillRect(statusRect, BG_STATUS);
    p.setPen(QPen(QColor(0x31, 0x32, 0x44), 1));
    p.drawLine(0, height() - 46, width(), height() - 46);

    p.setPen(color);
    QFont f = p.font();
    f.setPointSize(11);
    f.setBold(isMyTurn());
    p.setFont(f);
    p.drawText(statusRect, Qt::AlignCenter, text);
}

void DotsAndBoxesWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    if (!m_game) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    drawBackground(p);
    drawBoxes(p);
    drawLines(p);
    drawHover(p);
    drawDots(p);
    drawScoreBar(p);
    drawStatusText(p);
}
