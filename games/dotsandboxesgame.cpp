#include "dotsandboxesgame.h"

DotsAndBoxesGame::DotsAndBoxesGame(int boxRows, int boxCols, QObject *parent)
    : Game(parent), m_boxRows(boxRows), m_boxCols(boxCols)
{
    resetGame();
}

void DotsAndBoxesGame::resetGame()
{
    m_currentPlayer = 0;
    m_winner = -2;
    m_score[0] = 0;
    m_score[1] = 0;

    m_hLines.assign(m_boxRows + 1, QVector<bool>(m_boxCols, false));
    m_vLines.assign(m_boxRows, QVector<bool>(m_boxCols + 1, false));
    m_boxes.assign(m_boxRows, QVector<int>(m_boxCols, -1));

    emit boardChanged();
}

int DotsAndBoxesGame::currentPlayer() const
{
    return m_currentPlayer;
}

bool DotsAndBoxesGame::isGameOver() const
{
    return m_winner != -2;
}

int DotsAndBoxesGame::getWinner() const
{
    return m_winner;
}

QString DotsAndBoxesGame::gameName() const
{
    return "Dots and Boxes";
}

QVariant DotsAndBoxesGame::getBoardState() const
{
    // ارسال وضعیت کلی برای همگام‌سازی (در صورت نیاز)
    QVariantMap state;
    state["currentPlayer"] = m_currentPlayer;
    state["score0"] = m_score[0];
    state["score1"] = m_score[1];
    return state;
}

bool DotsAndBoxesGame::makeMove(int player, const QVariant& moveData)
{
    if (isGameOver() || player != m_currentPlayer) return false;

    QVariantMap map = moveData.toMap();
    QString type = map["type"].toString(); // "H" برای افقی, "V" برای عمودی
    int r = map["row"].toInt();
    int c = map["col"].toInt();

    bool isValidMove = false;

    if (type == "H" && r >= 0 && r <= m_boxRows && c >= 0 && c < m_boxCols) {
        if (!m_hLines[r][c]) {
            m_hLines[r][c] = true;
            isValidMove = true;
        }
    } else if (type == "V" && r >= 0 && r < m_boxRows && c >= 0 && c <= m_boxCols) {
        if (!m_vLines[r][c]) {
            m_vLines[r][c] = true;
            isValidMove = true;
        }
    }

    if (!isValidMove) return false;

    // بررسی اینکه آیا خانه‌ای گرفته شده است؟
    bool boxClaimed = checkAndClaimBoxes(r, c, type == "H", player);

    // اگر خانه‌ای نگرفت، نوبت به حریف می‌رسد
    if (!boxClaimed) {
        m_currentPlayer = (m_currentPlayer == 0) ? 1 : 0;
    }

    // بررسی پایان بازی
    if (m_score[0] + m_score[1] == m_boxRows * m_boxCols) {
        if (m_score[0] > m_score[1]) m_winner = 0;
        else if (m_score[1] > m_score[0]) m_winner = 1;
        else m_winner = -1; // مساوی

        emit boardChanged();
        emit gameEnded(m_winner);
    } else {
        emit boardChanged();
    }

    return true;
}

bool DotsAndBoxesGame::checkAndClaimBoxes(int r, int c, bool isHorizontal, int player)
{
    bool claimed = false;

    auto isBoxComplete = [&](int br, int bc) {
        if (br < 0 || br >= m_boxRows || bc < 0 || bc >= m_boxCols) return false;
        return m_hLines[br][bc] && m_hLines[br+1][bc] && m_vLines[br][bc] && m_vLines[br][bc+1];
    };

    // اگر خط افقی کشیده شده، حداکثر دو خانه بالا و پایین آن ممکن است کامل شوند
    if (isHorizontal) {
        if (isBoxComplete(r - 1, c) && m_boxes[r - 1][c] == -1) {
            m_boxes[r - 1][c] = player;
            m_score[player]++;
            claimed = true;
        }
        if (isBoxComplete(r, c) && m_boxes[r][c] == -1) {
            m_boxes[r][c] = player;
            m_score[player]++;
            claimed = true;
        }
    }
    // اگر خط عمودی کشیده شده، حداکثر دو خانه چپ و راست آن ممکن است کامل شوند
    else {
        if (isBoxComplete(r, c - 1) && m_boxes[r][c - 1] == -1) {
            m_boxes[r][c - 1] = player;
            m_score[player]++;
            claimed = true;
        }
        if (isBoxComplete(r, c) && m_boxes[r][c] == -1) {
            m_boxes[r][c] = player;
            m_score[player]++;
            claimed = true;
        }
    }

    return claimed;
}
