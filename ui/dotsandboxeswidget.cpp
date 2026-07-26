#include "dotsandboxeswidget.h"
#include <QPainter>
#include <QMouseEvent>

DotsAndBoxesWidget::DotsAndBoxesWidget(DotsAndBoxesGame* game, int myPlayer, QWidget *parent)
    : QWidget(parent), m_game(game), m_myPlayer(myPlayer),
    m_cellSize(60), m_margin(50), m_dotRadius(6), m_lineThickness(8), m_hitRadius(15)
{
    if (m_game) {
        int w = 2 * m_margin + m_game->getCols() * m_cellSize;
        int h = 2 * m_margin + m_game->getRows() * m_cellSize;
        setMinimumSize(w, h);

        connect(m_game, &Game::boardChanged, this, [this](){
            update();
        });
    }
}

void DotsAndBoxesWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_game) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int rows = m_game->getRows();
    int cols = m_game->getCols();

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int owner = m_game->boxOwner(r, c);
            if (owner != -1) {
                QRect rect(m_margin + c * m_cellSize, m_margin + r * m_cellSize, m_cellSize, m_cellSize);
                if (owner == 0)
                    painter.fillRect(rect, QColor(100, 150, 255, 150));
                else
                    painter.fillRect(rect, QColor(255, 100, 100, 150));
            }
        }
    }

    painter.setPen(QPen(Qt::black, m_lineThickness, Qt::SolidLine, Qt::RoundCap));
    for (int r = 0; r <= rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (m_game->hasHLine(r, c)) {
                int x = m_margin + c * m_cellSize;
                int y = m_margin + r * m_cellSize;
                painter.drawLine(x, y, x + m_cellSize, y);
            }
        }
    }

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c <= cols; ++c) {
            if (m_game->hasVLine(r, c)) {
                int x = m_margin + c * m_cellSize;
                int y = m_margin + r * m_cellSize;
                painter.drawLine(x, y, x, y + m_cellSize);
            }
        }
    }

    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);
    for (int r = 0; r <= rows; ++r) {
        for (int c = 0; c <= cols; ++c) {
            painter.drawEllipse(QPoint(m_margin + c * m_cellSize, m_margin + r * m_cellSize), m_dotRadius, m_dotRadius);
        }
    }
}

void DotsAndBoxesWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_game || m_game->isGameOver()) return;

    int effectivePlayer = (m_myPlayer == -1) ? m_game->currentPlayer() : m_myPlayer;

    if (effectivePlayer != m_game->currentPlayer()) return;

    QPoint pos = event->pos();
    int rows = m_game->getRows();
    int cols = m_game->getCols();

    for (int r = 0; r <= rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (m_game->hasHLine(r, c)) continue;

            int x = m_margin + c * m_cellSize;
            int y = m_margin + r * m_cellSize;
            QRect hitBox(x + m_dotRadius, y - m_hitRadius, m_cellSize - 2 * m_dotRadius, 2 * m_hitRadius);

            if (hitBox.contains(pos)) {
                QVariantMap move;
                move["type"] = "H";
                move["row"] = r;
                move["col"] = c;

                if (m_game->makeMove(effectivePlayer, move)) {
                    emit moveReadyToSend(move);
                }
                return;
            }
        }
    }

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c <= cols; ++c) {
            if (m_game->hasVLine(r, c)) continue;

            int x = m_margin + c * m_cellSize;
            int y = m_margin + r * m_cellSize;
            QRect hitBox(x - m_hitRadius, y + m_dotRadius, 2 * m_hitRadius, m_cellSize - 2 * m_dotRadius);

            if (hitBox.contains(pos)) {
                QVariantMap move;
                move["type"] = "V";
                move["row"] = r;
                move["col"] = c;

                if (m_game->makeMove(effectivePlayer, move)) {
                    emit moveReadyToSend(move);
                }
                return;
            }
        }
    }
}
