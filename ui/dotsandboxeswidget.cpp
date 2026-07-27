#include "dotsandboxeswidget.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

DotsAndBoxesWidget::DotsAndBoxesWidget(DotsAndBoxesGame* game, int myPlayer, const QString& player1Name, const QString& player2Name, QWidget* parent)
    : QWidget(parent)
    , m_game(game)
    , m_myPlayer(myPlayer)
    , m_player1Name(player1Name)
    , m_player2Name(player2Name)
{
    // فعال‌سازی تعقیب موش برای افکت Hover بدون نیاز به فشردن کلیک
    setMouseTracking(true);

    if (m_game) {
        // بازپاشی صفحه به محض تغییر وضعیت بازی
        connect(m_game, &Game::boardChanged, this, [this]() {
            update();
        });
    }
}

void DotsAndBoxesWidget::setPlayerNames(const QString& p1Name, const QString& p2Name)
{
    m_player1Name = p1Name;
    m_player2Name = p2Name;
    update();
}

bool DotsAndBoxesWidget::isMyTurn() const
{
    if (!m_game) return false;
    if (m_myPlayer == -1) return true; // حالت تست محلی
    return m_game->currentPlayer() == m_myPlayer;
}

void DotsAndBoxesWidget::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    if (m_hoveredLine.valid) {
        m_hoveredLine.valid = false;
        update();
    }
}

void DotsAndBoxesWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!isMyTurn() || !m_game || m_game->isGameOver()) {
        if (m_hoveredLine.valid) {
            m_hoveredLine.valid = false;
            update();
        }
        return;
    }

    LineHoverInfo currentHover = hitTestLine(event->pos());
    if (currentHover.valid != m_hoveredLine.valid ||
        currentHover.type != m_hoveredLine.type ||
        currentHover.row != m_hoveredLine.row ||
        currentHover.col != m_hoveredLine.col)
    {
        m_hoveredLine = currentHover;
        update();
    }
}

void DotsAndBoxesWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_game || !isMyTurn() || m_game->isGameOver())
        return;

    const int effectivePlayer = (m_myPlayer == -1) ? m_game->currentPlayer() : m_myPlayer;
    LineHoverInfo line = hitTestLine(event->pos());

    if (line.valid) {
        QVariantMap moveData;
        moveData["type"] = line.type;
        moveData["row"]  = line.row;
        moveData["col"]  = line.col;

        if (m_game->makeMove(effectivePlayer, moveData)) {
            m_hoveredLine.valid = false;
            emit moveReadyToSend(moveData);
        }
        update();
    }
}

DotsAndBoxesWidget::LineHoverInfo DotsAndBoxesWidget::hitTestLine(const QPoint& pos) const
{
    LineHoverInfo info;
    info.valid = false;

    if (!m_game) return info;

    int pRows = m_game->pointRows();
    int pCols = m_game->pointCols();
    QVariantMap state = m_game->getBoardState().toMap();
    QVariantList hLines = state["hLines"].toList();
    QVariantList vLines = state["vLines"].toList();

    double margin = 50.0;
    double boardW = width() - 2 * margin;
    double boardH = height() - 2 * margin;

    if (boardW <= 0 || boardH <= 0) return info;

    double cellW = boardW / (pCols - 1);
    double cellH = boardH / (pRows - 1);

    double minDistance = 18.0; // شعاع تشخیص کلیک/هاور حول خط به پیکسل

    // ۱. بررسی خطوط افقی
    for (int r = 0; r < pRows; ++r) {
        QVariantList rowList = hLines[r].toList();
        for (int c = 0; c < pCols - 1; ++c) {
            if (rowList[c].toBool()) continue; // قبلاً رسم شده است

            double x1 = margin + c * cellW;
            double x2 = margin + (c + 1) * cellW;
            double y  = margin + r * cellH;

            if (pos.x() >= x1 - 5 && pos.x() <= x2 + 5) {
                double dist = std::abs(pos.y() - y);
                if (dist < minDistance) {
                    minDistance = dist;
                    info.type = "h";
                    info.row = r;
                    info.col = c;
                    info.valid = true;
                }
            }
        }
    }

    // ۲. بررسی خطوط عمودی
    for (int r = 0; r < pRows - 1; ++r) {
        QVariantList rowList = vLines[r].toList();
        for (int c = 0; c < pCols; ++c) {
            if (rowList[c].toBool()) continue; // قبلاً رسم شده است
            double x  = margin + c * cellW;
            double y1 = margin + r * cellH;
            double y2 = margin + (r + 1) * cellH;

            if (pos.y() >= y1 - 5 && pos.y() <= y2 + 5) {
                double dist = std::abs(pos.x() - x);
                if (dist < minDistance) {
                    minDistance = dist;
                    info.type = "v";
                    info.row = r;
                    info.col = c;
                    info.valid = true;
                }
            }
        }
    }

    return info;
}

void DotsAndBoxesWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if (!m_game) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QVariantMap state = m_game->getBoardState().toMap();
    int pRows = state["pointRows"].toInt();
    int pCols = state["pointCols"].toInt();
    int bRows = pRows - 1;
    int bCols = pCols - 1;

    QVariantList hLines = state["hLines"].toList();
    QVariantList vLines = state["vLines"].toList();
    QVariantList boxes  = state["boxes"].toList();

    double margin = 50.0;
    double boardW = width() - 2 * margin;
    double boardH = height() - 2 * margin;

    if (boardW <= 0 || boardH <= 0) return;

    double cellW = boardW / (pCols - 1);
    double cellH = boardH / (pRows - 1);

    // رنگ‌بندی دو بازیکن
    QColor p1Color(41, 128, 185); // آبی
    QColor p2Color(231, 76, 60);  // قرمز
    QColor neutralColor(127, 140, 141);

    // ── ۱. رسم مربع‌های تکمیل‌شده و حروف اول یوزرنیم ──────────────────────
    QString p1Initial = m_player1Name.isEmpty() ? "1" : QString(m_player1Name.at(0)).toUpper();
    QString p2Initial = m_player2Name.isEmpty() ? "2" : QString(m_player2Name.at(0)).toUpper();

    QFont font("Segoe UI", qMin(cellW, cellH) * 0.45, QFont::Bold);
    painter.setFont(font);

    for (int r = 0; r < bRows; ++r) {
        QVariantList rowList = boxes[r].toList();
        for (int c = 0; c < bCols; ++c) {
            int owner = rowList[c].toInt();
            if (owner != -1) {
                QRectF boxRect(margin + c * cellW, margin + r * cellH, cellW, cellH);

                QColor fillColor = (owner == 0) ? p1Color : p2Color;
                fillColor.setAlpha(45); // پس‌زمینه شفاف
                painter.fillRect(boxRect, fillColor);

                painter.setPen((owner == 0) ? p1Color : p2Color);
                QString initial = (owner == 0) ? p1Initial : p2Initial;
                painter.drawText(boxRect, Qt::AlignCenter, initial);
            }
        }
    }

    // ── ۲. رسم خط Hover (هایلایت پیش‌نمایش کلیک) ───────────────────────────
    if (m_hoveredLine.valid && isMyTurn() && !m_game->isGameOver()) {
        int currPlayer = m_game->currentPlayer();
        QColor hoverColor = (currPlayer == 0) ? p1Color : p2Color;
        hoverColor.setAlpha(120);

        QPen hoverPen(hoverColor, 5, Qt::DashLine, Qt::RoundCap);
        painter.setPen(hoverPen);

        if (m_hoveredLine.type == "h") {
            double x1 = margin + m_hoveredLine.col * cellW;
            double x2 = margin + (m_hoveredLine.col + 1) * cellW;
            double y  = margin + m_hoveredLine.row * cellH;
            painter.drawLine(QPointF(x1, y), QPointF(x2, y));
        } else if (m_hoveredLine.type == "v") {
            double x  = margin + m_hoveredLine.col * cellW;
            double y1 = margin + m_hoveredLine.row * cellH;
            double y2 = margin + (m_hoveredLine.row + 1) * cellH;
            painter.drawLine(QPointF(x, y1), QPointF(x, y2));
        }
    }

    // ── ۳. رسم خطوط افقی ثبت‌شده ──────────────────────────────────────────
    QPen linePen(QColor(44, 62, 80), 5, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(linePen);

    for (int r = 0; r < pRows; ++r) {
        QVariantList rowList = hLines[r].toList();
        for (int c = 0; c < pCols - 1; ++c) {
            if (rowList[c].toBool()) {
                double x1 = margin + c * cellW;
                double x2 = margin + (c + 1) * cellW;
                double y  = margin + r * cellH;
            painter.drawLine(QPointF(x1, y), QPointF(x2, y));
            }
        }
    }

    // ── ۴. رسم خطوط عمودی ثبت‌شده ──────────────────────────────────────────
    for (int r = 0; r < pRows - 1; ++r) {
        QVariantList rowList = vLines[r].toList();
        for (int c = 0; c < pCols; ++c) {
            if (rowList[c].toBool()) {
                double x  = margin + c * cellW;
                double y1 = margin + r * cellH;
                double y2 = margin + (r + 1) * cellH;
                painter.drawLine(QPointF(x, y1), QPointF(x, y2));
            }
        }
    }

    // ── ۵. رسم نقاط شبکه (Dots) ───────────────────────────────────────────
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(44, 62, 80)); // رنگ نقاط
    double dotRadius = 6.0;

    for (int r = 0; r < pRows; ++r) {
        for (int c = 0; c < pCols; ++c) {
            double x = margin + c * cellW;
            double y = margin + r * cellH;
            painter.drawEllipse(QPointF(x, y), dotRadius, dotRadius);
        }
    }
}