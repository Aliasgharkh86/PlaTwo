#include "ninemensmorrisgame.h"

// ─────────────────────────────────────────────
// شماره‌گذاری استاندارد تخته‌ی Nine Men's Morris (24 نقطه):
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

NineMensMorrisGame::NineMensMorrisGame(QObject* parent)
    : Game(parent)
{
    resetGame();
}

const QVector<QVector<int>>& NineMensMorrisGame::adjacency()
{
    static const QVector<QVector<int>> adj = {
                                              /*0*/  {1, 9},
                                              /*1*/  {0, 2, 4},
                                              /*2*/  {1, 14},
                                              /*3*/  {4, 10},
                                              /*4*/  {1, 3, 5, 7},
                                              /*5*/  {4, 13},
                                              /*6*/  {7, 11},
                                              /*7*/  {4, 6, 8},
                                              /*8*/  {7, 12},
                                              /*9*/  {0, 10, 21},
                                              /*10*/ {3, 9, 11, 18},
                                              /*11*/ {6, 10, 15},
                                              /*12*/ {8, 13, 17},
                                              /*13*/ {5, 12, 14, 20},
                                              /*14*/ {2, 13, 23},
                                              /*15*/ {11, 16},
                                              /*16*/ {15, 17, 19},
                                              /*17*/ {12, 16},
                                              /*18*/ {10, 19},
                                              /*19*/ {16, 18, 20, 22},
                                              /*20*/ {13, 19},
                                              /*21*/ {9, 22},
                                              /*22*/ {19, 21, 23},
                                              /*23*/ {14, 22},
                                              };
    return adj;
}

const QVector<QVector<int>>& NineMensMorrisGame::mills()
{
    static const QVector<QVector<int>> m = {
                                            // ردیف‌های افقی
                                            {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
                                            {9, 10, 11}, {12, 13, 14}, {15, 16, 17},
                                            {18, 19, 20}, {21, 22, 23},
                                            // ستون‌های عمودی
                                            {0, 9, 21}, {3, 10, 18}, {6, 11, 15},
                                            {1, 4, 7}, {16, 19, 22}, {8, 12, 17},
                                            {5, 13, 20}, {2, 14, 23},
                                            };
    return m;
}

int NineMensMorrisGame::currentPlayer() const
{
    return m_currentPlayer;
}

QString NineMensMorrisGame::gameName() const
{
    return "Nine Men's Morris";
}

bool NineMensMorrisGame::isGameOver() const
{
    return m_winner != -2;
}

int NineMensMorrisGame::getWinner() const
{
    return m_winner;
}

void NineMensMorrisGame::resetGame()
{
    for (int i = 0; i < 24; ++i)
        m_board[i] = 0;

    m_currentPlayer     = 0;
    m_piecesToPlace[0]  = 9;
    m_piecesToPlace[1]  = 9;
    m_piecesOnBoard[0]  = 0;
    m_piecesOnBoard[1]  = 0;
    m_pendingRemoval    = false;
    m_winner            = -2;

    emit boardChanged();
}

QVariant NineMensMorrisGame::getBoardState() const
{
    QVariantList boardList;
    for (int i = 0; i < 24; ++i)
        boardList.append(m_board[i]);

    QVariantMap state;
    state["board"]          = boardList;
    state["currentPlayer"]  = m_currentPlayer;
    state["piecesToPlace0"] = m_piecesToPlace[0];
    state["piecesToPlace1"] = m_piecesToPlace[1];
    state["piecesOnBoard0"] = m_piecesOnBoard[0];
    state["piecesOnBoard1"] = m_piecesOnBoard[1];
    state["pendingRemoval"] = m_pendingRemoval;
    return state;
}

bool NineMensMorrisGame::isMill(int position, int player) const
{
    const int val = player + 1;
    for (const QVector<int>& mill : mills()) {
        if (!mill.contains(position))
            continue;
        if (m_board[mill[0]] == val && m_board[mill[1]] == val && m_board[mill[2]] == val)
            return true;
    }
    return false;
}

bool NineMensMorrisGame::allPiecesInMills(int player) const
{
    const int val = player + 1;
    for (int i = 0; i < 24; ++i) {
        if (m_board[i] == val && !isMill(i, player))
            return false;
    }
    return true;
}

bool NineMensMorrisGame::canFly(int player) const
{
    return m_piecesToPlace[player] == 0 && m_piecesOnBoard[player] == 3;
}

bool NineMensMorrisGame::hasAnyLegalMove(int player) const
{
    if (m_piecesToPlace[player] > 0)
        return true; // همیشه جای خالی برای گذاشتن هست (فاز placing)

    if (canFly(player)) {
        for (int i = 0; i < 24; ++i)
            if (m_board[i] == 0)
                return true;
        return false;
    }

    const int val = player + 1;
    for (int i = 0; i < 24; ++i) {
        if (m_board[i] != val)
            continue;
        for (int adj : adjacency()[i]) {
            if (m_board[adj] == 0)
                return true;
        }
    }
    return false;
}

void NineMensMorrisGame::switchPlayer()
{
    m_currentPlayer = 1 - m_currentPlayer;
}

void NineMensMorrisGame::checkForStalemateLoss()
{
    if (!hasAnyLegalMove(m_currentPlayer)) {
        m_winner = 1 - m_currentPlayer;
        emit gameEnded(m_winner);
    }
}

bool NineMensMorrisGame::makeMove(int player, const QVariant& moveData)
{
    if (isGameOver())
        return false;
    if (player != m_currentPlayer)
        return false;

    const QVariantMap data = moveData.toMap();

    // ── حالت ۱: منتظر حذف مهره‌ی حریف بعد از تشکیل mill ──
    if (m_pendingRemoval) {
        if (!data.contains("remove"))
            return false;

        const int removePos = data.value("remove").toInt();
        const int opponent  = 1 - player;

        if (removePos < 0 || removePos > 23)
            return false;
        if (m_board[removePos] != opponent + 1)
            return false;
        // نمی‌شه مهره‌ای که توی mill هست رو حذف کرد،
        // مگر این‌که همه‌ی مهره‌های حریف توی mill باشن
        if (isMill(removePos, opponent) && !allPiecesInMills(opponent))
            return false;

        m_board[removePos] = 0;
        m_piecesOnBoard[opponent]--;
        m_pendingRemoval = false;

        // اگه حریف کمتر از ۳ مهره داشته باشه (و دیگه چیزی برای گذاشتن نداره) می‌بازه
        if (m_piecesOnBoard[opponent] < 3 && m_piecesToPlace[opponent] == 0) {
            m_winner = player;
            emit gameEnded(m_winner);
            emit boardChanged();
            return true;
        }

        switchPlayer();
        checkForStalemateLoss();
        emit boardChanged();
        return true;
    }

    // ── حالت ۲: حرکت عادی (گذاشتن یا جابه‌جایی) ──
    bool formedMill = false;

    if (m_piecesToPlace[player] > 0) {
        // فاز Placing
        if (!data.contains("place"))
            return false;
        const int pos = data.value("place").toInt();
        if (pos < 0 || pos > 23 || m_board[pos] != 0)
            return false;

        m_board[pos] = player + 1;
        m_piecesToPlace[player]--;
        m_piecesOnBoard[player]++;
        formedMill = isMill(pos, player);
    } else {
        // فاز Moving یا Flying
        if (!data.contains("from") || !data.contains("to"))
            return false;
        const int from = data.value("from").toInt();
        const int to   = data.value("to").toInt();

        if (from < 0 || from > 23 || to < 0 || to > 23)
            return false;
        if (m_board[from] != player + 1 || m_board[to] != 0)
            return false;

        const bool flying = canFly(player);
        if (!flying && !adjacency()[from].contains(to))
            return false;

        m_board[from] = 0;
        m_board[to]   = player + 1;
        formedMill = isMill(to, player);
    }

    // اگه mill تشکیل شد و حریف مهره‌ای برای حذف داره، منتظر حذف می‌مونیم
    if (formedMill) {
        const int opponent = 1 - player;
        if (m_piecesOnBoard[opponent] > 0)
            m_pendingRemoval = true;
    }

    if (!m_pendingRemoval) {
        switchPlayer();
        checkForStalemateLoss();
    }

    emit boardChanged();
    return true;
}
