#include "dotsandboxesgame.h"

DotsAndBoxesGame::DotsAndBoxesGame(int boardSize, QObject* parent)
    : Game(parent)
    , m_boardSize(boardSize)
    , m_N(boardSize)
{
    resetGame();
}

void DotsAndBoxesGame::resetGame()
{
    // مقداردهی اولیه‌ی خطوط — همه false (کشیده‌نشده)
    m_hLines.assign(m_N,     QVector<bool>(m_N - 1, false));
    m_vLines.assign(m_N - 1, QVector<bool>(m_N,     false));
    m_boxes .assign(m_N - 1, QVector<int> (m_N - 1, 0));

    m_scores[0]   = 0;
    m_scores[1]   = 0;
    m_currentPlayer = 0;
    m_winner        = -2;

    emit boardChanged();
}

int DotsAndBoxesGame::currentPlayer() const { return m_currentPlayer; }
QString DotsAndBoxesGame::gameName() const  { return "Dots and Boxes"; }
bool DotsAndBoxesGame::isGameOver() const   { return m_winner != -2; }
int DotsAndBoxesGame::getWinner() const     { return m_winner; }

QVariant DotsAndBoxesGame::getBoardState() const
{
    // خطوط افقی
    QVariantList hList;
    for (int r = 0; r < m_N; ++r)
        for (int c = 0; c < m_N - 1; ++c)
            hList.append(m_hLines[r][c]);

    // خطوط عمودی
    QVariantList vList;
    for (int r = 0; r < m_N - 1; ++r)
        for (int c = 0; c < m_N; ++c)
            vList.append(m_vLines[r][c]);

    // جعبه‌ها
    QVariantList boxList;
    for (int r = 0; r < m_N - 1; ++r)
        for (int c = 0; c < m_N - 1; ++c)
            boxList.append(m_boxes[r][c]);

    QVariantMap state;
    state["boardSize"]     = m_boardSize;
    state["hLines"]        = hList;
    state["vLines"]        = vList;
    state["boxes"]         = boxList;
    state["score0"]        = m_scores[0];
    state["score1"]        = m_scores[1];
    state["currentPlayer"] = m_currentPlayer;
    return state;
}

// ── اعتبارسنجی ────────────────────────────────

bool DotsAndBoxesGame::isValidHLine(int row, int col) const
{
    return row >= 0 && row < m_N && col >= 0 && col < m_N - 1;
}

bool DotsAndBoxesGame::isValidVLine(int row, int col) const
{
    return row >= 0 && row < m_N - 1 && col >= 0 && col < m_N;
}

bool DotsAndBoxesGame::isBoxComplete(int boxRow, int boxCol) const
{
    return m_hLines[boxRow    ][boxCol] &&   // بالا
           m_hLines[boxRow + 1][boxCol] &&   // پایین
           m_vLines[boxRow    ][boxCol] &&   // چپ
           m_vLines[boxRow    ][boxCol + 1]; // راست
}

// ── کامل کردن جعبه‌های مجاور ─────────────────

int DotsAndBoxesGame::claimAdjacentBoxes(bool isHorizontal,
                                         int row, int col, int player)
{
    int claimed = 0;

    if (isHorizontal) {
        // یه خط افقی در (row, col) می‌تونه دو جعبه رو کامل کنه:
        //   جعبه‌ی بالا: (row-1, col) — اگه row > 0
        //   جعبه‌ی پایین: (row, col) — اگه row < N-1
        if (row > 0 && isBoxComplete(row - 1, col)) {
            m_boxes[row - 1][col] = player + 1;
            m_scores[player]++;
            claimed++;
        }
        if (row < m_N - 1 && isBoxComplete(row, col)) {
            m_boxes[row][col] = player + 1;
            m_scores[player]++;
            claimed++;
        }
    } else {
        // یه خط عمودی در (row, col) می‌تونه دو جعبه رو کامل کنه:
        //   جعبه‌ی چپ:  (row, col-1) — اگه col > 0
        //   جعبه‌ی راست: (row, col) — اگه col < N-1
        if (col > 0 && isBoxComplete(row, col - 1)) {
            m_boxes[row][col - 1] = player + 1;
            m_scores[player]++;
            claimed++;
        }
        if (col < m_N - 1 && isBoxComplete(row, col)) {
            m_boxes[row][col] = player + 1;
            m_scores[player]++;
            claimed++;
        }
    }

    return claimed;
}

void DotsAndBoxesGame::checkGameOver()
{
    // بازی تموم می‌شه وقتی همه‌ی جعبه‌ها صاحب داشته باشن
    const int totalBoxes = (m_N - 1) * (m_N - 1);
    if (m_scores[0] + m_scores[1] < totalBoxes)
        return;

    if (m_scores[0] > m_scores[1])
        m_winner = 0;
    else if (m_scores[1] > m_scores[0])
        m_winner = 1;
    else
        m_winner = -1; // مساوی

    emit gameEnded(m_winner);
}

// ── makeMove ──────────────────────────────────

bool DotsAndBoxesGame::makeMove(int player, const QVariant& moveData)
{
    if (isGameOver())         return false;
    if (player != m_currentPlayer) return false;

    const QVariantMap data = moveData.toMap();
    if (!data.contains("isHorizontal") ||
        !data.contains("row") ||
        !data.contains("col"))
        return false;

    const bool isHorizontal = data.value("isHorizontal").toBool();
    const int  row          = data.value("row").toInt();
    const int  col          = data.value("col").toInt();

    if (isHorizontal) {
        if (!isValidHLine(row, col))        return false;
        if (m_hLines[row][col])             return false; // قبلاً کشیده شده
        m_hLines[row][col] = true;
    } else {
        if (!isValidVLine(row, col))        return false;
        if (m_vLines[row][col])             return false;
        m_vLines[row][col] = true;
    }

    // چک کن آیا جعبه‌ای کامل شد
    const int boxesClaimed = claimAdjacentBoxes(isHorizontal, row, col, player);

    checkGameOver();

    if (!isGameOver()) {
        // اگه جعبه‌ای کامل نشد → نوبت عوض می‌شه
        // اگه جعبه‌ای کامل شد → همون بازیکن دوباره حرکت می‌کنه
        if (boxesClaimed == 0)
            m_currentPlayer = 1 - m_currentPlayer;
    }

    emit boardChanged();
    return true;
}
