#include "fanoronagamewidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QVariantMap>
#include <cmath>

// ── رنگ‌های تم Catppuccin Mocha ─────────────
namespace {
const QColor BG_DARK    {0x1e, 0x1e, 0x2e};       // پس‌زمینه اصلی
const QColor BG_BOARD   {0x28, 0x28, 0x3a};       // پس‌زمینه‌ی تخته
const QColor BG_STATUS  {0x18, 0x18, 0x25};       // نوار وضعیت
const QColor LINE_COLOR {0x58, 0x5b, 0x70};       // خطوط تخته
const QColor POINT_IDLE {0x45, 0x47, 0x5a};       // نقاط خالی
const QColor POINT_SEL  {0xf9, 0xe2, 0xaf};       // نقطه‌ی انتخاب‌شده (زرد)

// رنگ مهره بازیکن ۱ (نارنجی گرم)
const QColor PIECE_P0_A {0xfb, 0xb1, 0x6d};
const QColor PIECE_P0_B {0xe6, 0x7e, 0x22};

// رنگ مهره بازیکن ۲ (خاکستری مایل به بنفش)
const QColor PIECE_P1_A {0xba, 0xc2, 0xde};
const QColor PIECE_P1_B {0x6c, 0x70, 0x86};

const QColor CHAIN_GLOW  {0xa6, 0xe3, 0xa1, 160};  // هاله‌ی زنجیره حرکت (سبز)
const QColor TEXT_MAIN  {0xcd, 0xd6, 0xf4};       // متن اصلی
const QColor TEXT_DIM   {0x6c, 0x70, 0x86};       // متن کم‌رنگ
} // namespace

FanoronaWidget::FanoronaWidget(QWidget* parent)
    : QWidget(parent)
    , m_game(nullptr)
    , m_myPlayer(-1)
    , m_selectedPoint(-1)
{
    setMinimumSize(540, 380);

    m_endTurnBtn = new QPushButton("پایان نوبت (توقف زنجیره)", this);
    m_endTurnBtn->hide();

    // استایل‌دهی مدرن دکمه پایان نوبت متناسب با تم
    m_endTurnBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #f9e2af;"
        "  color: #11111b;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "  padding: 4px 10px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #fae3b0;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #df8e1d;"
        "}"
        );

    connect(m_endTurnBtn, &QPushButton::clicked, this, &FanoronaWidget::onEndTurnClicked);
}

FanoronaWidget::FanoronaWidget(FanoronaGame* game, int myPlayer, QWidget* parent)
    : FanoronaWidget(parent)
{
    setGame(game, myPlayer);
}

void FanoronaWidget::setGame(FanoronaGame* game, int myPlayer)
{
    if (m_game)
        m_game->disconnect(this);

    m_game = game;
    m_myPlayer = myPlayer;
    m_selectedPoint = -1;

    connect(m_game, &Game::boardChanged, this, &FanoronaWidget::onBoardChanged);
    connect(m_game, &Game::gameEnded,    this, &FanoronaWidget::onGameEnded);
    updateEndTurnButtonVisibility();
    update();
}

void FanoronaWidget::onBoardChanged()
{
    m_selectedPoint = -1;
    m_feedbackMessage.clear();
    updateEndTurnButtonVisibility();
    update();
}

void FanoronaWidget::onGameEnded(int winner)
{
    Q_UNUSED(winner)
    m_endTurnBtn->hide();
    update();
}

void FanoronaWidget::onEndTurnClicked()
{
    if (!m_game) return;

    QVariantMap state = m_game->getBoardState().toMap();
    if (!state.value("chainInProgress").toBool())
        return;

    QVariantMap move;
    move["endTurn"] = true;

    if (m_game->makeMove(m_game->currentPlayer(), move)) {
        emit moveReadyToSend(move);
    }
}

void FanoronaWidget::updateEndTurnButtonVisibility()
{
    bool show = false;
    if (m_game) {
        QVariantMap state = m_game->getBoardState().toMap();
        show = state.value("chainInProgress").toBool() && isMyTurn();
    }
    m_endTurnBtn->setVisible(show);
}

void FanoronaWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    const int btnW = 180, btnH = 30;
    const int statusH = 46;
    m_endTurnBtn->setGeometry(width() - btnW - 20, height() - statusH - btnH - 10, btnW, btnH);
}

bool FanoronaWidget::isMyTurn() const
{
    if (!m_game || m_game->isGameOver())
        return false;
    if (m_myPlayer == -1)
        return true;
    return m_game->currentPlayer() == m_myPlayer;
}

QPoint FanoronaWidget::pointPosition(int index) const
{
    const int row = index / COLS;
    const int col = index % COLS;

    const int marginX = 45;
    const int topMargin = 38;const int statusH = 46;
    const int bottomMargin = statusH + 15;

    const int usableW = width()  - 2 * marginX;
    const int usableH = height() - topMargin - bottomMargin;

    const int x = marginX + (col * usableW) / (COLS - 1);
    const int y = topMargin + (row * usableH) / (ROWS - 1);
    return QPoint(x, y);
}

int FanoronaWidget::pointAt(const QPoint& pos) const
{
    const int threshold = 18;
    for (int i = 0; i < ROWS * COLS; ++i) {
        const QPoint p = pointPosition(i);
        const int dx = p.x() - pos.x();
        const int dy = p.y() - pos.y();
        if (std::sqrt(double(dx * dx + dy * dy)) <= threshold)
            return i;
    }
    return -1;
}

// ── رسم پس‌زمینه ──────────────────────────────
void FanoronaWidget::drawBackground(QPainter& p) const
{
    p.fillRect(rect(), BG_DARK);

    const int margin = 20;
    const int statusH = 46;
    QRect boardRect(margin, margin,
                    width() - 2 * margin,
                    height() - 2 * margin - statusH);

    p.setPen(QPen(QColor(0x45, 0x47, 0x5a), 1));
    p.setBrush(BG_BOARD);
    p.drawRoundedRect(boardRect, 12, 12);
}

// ── رسم خطوط تخته ─────────────────────────────
void FanoronaWidget::drawBoardLines(QPainter& p) const
{
    p.setPen(QPen(LINE_COLOR, 1.8, Qt::SolidLine, Qt::RoundCap));
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            const int idx = row * COLS + col;

            if (col + 1 < COLS)
                p.drawLine(pointPosition(idx), pointPosition(idx + 1));

            if (row + 1 < ROWS)
                p.drawLine(pointPosition(idx), pointPosition(idx + COLS));

            if ((row + col) % 2 == 0) {
                if (row + 1 < ROWS && col + 1 < COLS)
                    p.drawLine(pointPosition(idx), pointPosition(idx + COLS + 1));
                if (row + 1 < ROWS && col - 1 >= 0)
                    p.drawLine(pointPosition(idx), pointPosition(idx + COLS - 1));
            }
        }
    }
}

// ── رسم تقاطع‌ها و هایلایت‌ها ──────────────────
void FanoronaWidget::drawPoints(QPainter& p) const
{
    for (int i = 0; i < ROWS * COLS; ++i) {
        const QPoint pt = pointPosition(i);

        if (i == m_selectedPoint) {
            // هاله‌ی دور نقطه انتخاب‌شده
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0xf9, 0xe2, 0xaf, 60));
            p.drawEllipse(pt, 14, 14);

            p.setPen(QPen(POINT_SEL, 2));
            p.setBrush(POINT_SEL);
            p.drawEllipse(pt, 6, 6);
        } else {
            p.setPen(QPen(LINE_COLOR, 1));
            p.setBrush(POINT_IDLE);
            p.drawEllipse(pt, 4, 4);
        }
    }
}

// ── رسم مهره‌ها ───────────────────────────────
void FanoronaWidget::drawPieces(QPainter& p) const
{
    if (!m_game) return;

    const QVariantMap state = m_game->getBoardState().toMap();
    const QVariantList board = state.value("board").toList();
    const bool chaining = state.value("chainInProgress").toBool();
    const int chainPos = state.value("chainPosition").toInt();

    for (int i = 0; i < ROWS * COLS; ++i) {
        const int val = board.at(i).toInt();
        if (val == 0)
            continue;

        const QPoint pt = pointPosition(i);
        const bool isP0 = (val == 1);

        // اگر مهره در حال ادامه زنجیره حمله باشد، هاله‌ی سبز دور آن رسم می‌شود
        if (chaining && i == chainPos) {
            p.setPen(Qt::NoPen);
            p.setBrush(CHAIN_GLOW);
            p.drawEllipse(pt, 18, 18);
        }

        // سایه‌ی مهره
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 70));
        p.drawEllipse(pt + QPoint(2, 3), 12, 12);

        // گرادیان سه بعدی مهره
        QRadialGradient grad(pt - QPoint(3, 4), 13);
        grad.setColorAt(0.0, isP0 ? PIECE_P0_A : PIECE_P1_A);
        grad.setColorAt(1.0, isP0 ? PIECE_P0_B : PIECE_P1_B);

        p.setPen(QPen(isP0 ? PIECE_P0_B : PIECE_P1_B, 1.5));
        p.setBrush(grad);
        p.drawEllipse(pt, 12, 12);

        // نقطه لایت/براق روی مهره
        p.setPen(Qt::NoPen);p.setBrush(QColor(255, 255, 255, 80));
        p.drawEllipse(pt - QPoint(4, 4), 4, 4);
    }
}

// ── شمارش مهره‌ها ─────────────────────────────
void FanoronaWidget::drawPieceCount(QPainter& p) const
{
    if (!m_game) return;

    const QVariantMap state = m_game->getBoardState().toMap();
    const QVariantList board = state.value("board").toList();

    int countP1 = 0, countP2 = 0;
    for (const auto& item : board) {
        if (item.toInt() == 1) countP1++;
        else if (item.toInt() == 2) countP2++;
    }

    QFont f = p.font();
    f.setPointSize(9);
    f.setBold(true);
    p.setFont(f);

    // بازیکن ۱ (سمت چپ)
    p.setPen(PIECE_P0_A);
    p.drawText(QRect(28, 8, 160, 20), Qt::AlignLeft,
               QString("⬤ بازیکن ۱: %1 مهره").arg(countP1));

    // بازیکن ۲ (سمت راست)
    p.setPen(PIECE_P1_A);
    p.drawText(QRect(width() - 188, 8, 160, 20), Qt::AlignRight,
               QString("⬤ بازیکن ۲: %1 مهره").arg(countP2));
}

// ── نوار وضعیت ────────────────────────────────
void FanoronaWidget::drawStatusText(QPainter& p) const
{
    QString text;
    QColor textColor = TEXT_MAIN;
    QVariantMap state = m_game ? m_game->getBoardState().toMap() : QVariantMap();

    if (!m_feedbackMessage.isEmpty()) {
        text = m_feedbackMessage;
        textColor = QColor(0xf3, 0x8b, 0xa8); // قرمز
    } else if (m_game && m_game->isGameOver()) {
        const int winner = m_game->getWinner();
        if (winner == -1) {
            text = "🤝  بازی مساوی شد!";
            textColor = QColor(0xf9, 0xe2, 0xaf);
        } else {
            if (m_myPlayer == -1) {
                text = QString("🎉  بازیکن %1 برنده شد!").arg(winner + 1);
            } else {
                text = (winner == m_myPlayer) ? "🎉  شما بردید!" : "😔  حریف برد.";
            }
            textColor = (winner == m_myPlayer || m_myPlayer == -1)
                            ? QColor(0xa6, 0xe3, 0xa1)
                            : QColor(0xf3, 0x8b, 0xa8);
        }
    } else if (state.value("chainInProgress").toBool()) {
        text = "⚡ زنجیره حرکت: می‌توانید capture دیگری بزنید یا نوبت را تمام کنید.";
        textColor = QColor(0xa6, 0xe3, 0xa1);
    } else if (isMyTurn()) {
        text = (m_myPlayer == -1)
        ? QString("✨ نوبت بازیکن %1").arg(m_game->currentPlayer() + 1)
        : "✨ نوبت شماست.";
        textColor = QColor(0x89, 0xb4, 0xfa);
    } else {
        text = "⏳ در انتظار حرکت حریف...";
        textColor = TEXT_DIM;
    }

    // پنل نوار وضعیت
    const QRect statusRect(0, height() - 46, width(), 46);
    p.fillRect(statusRect, BG_STATUS);

    // خط جداکننده بالا
    p.setPen(QPen(QColor(0x31, 0x32, 0x44), 1));
    p.drawLine(0, height() - 46, width(), height() - 46);

    // رسم متن
    p.setPen(textColor);
    QFont f = p.font();
    f.setPointSize(10);
    f.setBold(isMyTurn() && m_game && !m_game->isGameOver());
    p.setFont(f);
    p.drawText(statusRect, Qt::AlignCenter, text);
}

// ── paintEvent ────────────────────────────────
void FanoronaWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    drawBackground(p);
    drawBoardLines(p);
    drawPoints(p);

    if (!m_game)
        return;

    drawPieces(p);
    drawPieceCount(p);
    drawStatusText(p);
}

// ── منطق حرکت و کلیک (کاملاً بدون تغییر) ──────
void FanoronaWidget::attemptMove(int from, int to)
{
    const int effectivePlayer = (m_myPlayer == -1) ? m_game->currentPlayer() : m_myPlayer;

    if (m_game->moveNeedsCaptureTypeChoice(from, to)) {
        QMessageBox box(this);
        box.setWindowTitle("انتخاب نوع capture");
        box.setText("این حرکت هم به‌صورت Approach هم Withdrawal قابل انجامه. کدوم رو می‌خواید؟");
        QPushButton* approachBtn   = box.addButton("Approach (پیشروی)", QMessageBox::AcceptRole);
        QPushButton* withdrawalBtn = box.addButton("Withdrawal (عقب‌نشینی)", QMessageBox::RejectRole);
        box.exec();

        QVariantMap move;
        move["from"] = from;
        move["to"]   = to;
    if (box.clickedButton() == approachBtn)
            move["captureType"] = "approach";
        else if (box.clickedButton() == withdrawalBtn)
            move["captureType"] = "withdrawal";
        else
            return;

        if (m_game->makeMove(effectivePlayer, move)) {
            QVariantMap state = m_game->getBoardState().toMap();
            if (state.value("chainInProgress").toBool()) {
                move["keepTurn"] = true;
            }
            emit moveReadyToSend(move);
        } else {
            m_feedbackMessage = "⚠️ این حرکت مجاز نیست.";
            update();
        }
        return;
    }

    QVariantMap move;
    move["from"] = from;
    move["to"]   = to;

    if (m_game->makeMove(effectivePlayer, move)) {
        QVariantMap state = m_game->getBoardState().toMap();
        if (state.value("chainInProgress").toBool()) {
            move["keepTurn"] = true;
        }
        emit moveReadyToSend(move);
    } else {
        m_feedbackMessage = "⚠️ این حرکت مجاز نیست — احتمالاً باید با مهره‌ی دیگه‌ای capture اجباری بزنید.";
        update();
    }
}

void FanoronaWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_game || !isMyTurn())
        return;

    const int clicked = pointAt(event->pos());
    if (clicked == -1)
        return;

    QVariantMap state = m_game->getBoardState().toMap();
    bool chaining = state.value("chainInProgress").toBool();
    int chainPos = state.value("chainPosition").toInt();

    if (chaining) {
        attemptMove(chainPos, clicked);
        return;
    }

    const int effectivePlayer = (m_myPlayer == -1) ? m_game->currentPlayer() : m_myPlayer;
    const QVariantList board = state.value("board").toList();
    const int clickedVal = board.at(clicked).toInt();

    if (m_selectedPoint == -1) {
        if (clickedVal == effectivePlayer + 1)
            m_selectedPoint = clicked;
        m_feedbackMessage.clear();
        update();
        return;
    }

    if (clicked == m_selectedPoint) {
        m_selectedPoint = -1;
        m_feedbackMessage.clear();
        update();
        return;
    }

    if (clickedVal == effectivePlayer + 1) {
        m_selectedPoint = clicked;
        m_feedbackMessage.clear();
        update();
        return;
    }

    attemptMove(m_selectedPoint, clicked);
    m_selectedPoint = -1;
    update();
}