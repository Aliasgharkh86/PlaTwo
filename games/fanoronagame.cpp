#include "fanoronagame.h"
#include <QtGlobal>

FanoronaGame::FanoronaGame(QObject* parent)
    : Game(parent)
{
    resetGame();
}

// ─────────────────────────────────────────────
// چیدمان اولیه‌ی استاندارد Fanorona (۲۲ مهره برای هر بازیکن)
// row0: W W W W W W W W W
// row1: W W W W W W W W W
// row2: W B W B . W B W B   ← مرکز خالی
// row3: B B B B B B B B B
// row4: B B B B B B B B B
// ─────────────────────────────────────────────

void FanoronaGame::resetGame()
{
    for (int i = 0; i < CELLS; ++i)
        m_board[i] = 0;

    for (int c = 0; c < COLS; ++c) {
        m_board[0 * COLS + c] = 2; // ردیف‌های بالا -> خاکستری (بازیکن 1)
        m_board[1 * COLS + c] = 2;
        m_board[3 * COLS + c] = 1; // ردیف‌های پایین -> نارنجی (بازیکن 0)
        m_board[4 * COLS + c] = 1;
    }

    // سطر وسط: شروع با نارنجی (1) و سپس خاکستری (2)
    static const int midRow[COLS] = {1, 2, 1, 2, 0, 1, 2, 1, 2};
    for (int c = 0; c < COLS; ++c)
        m_board[2 * COLS + c] = midRow[c];

    m_currentPlayer    = 0; // 👈 نوبت به 0 برگشت تا با GameServer هماهنگ شود
    m_piecesOnBoard[0] = 22;
    m_piecesOnBoard[1] = 22;
    m_winner           = -2;

    m_chainInProgress  = false;
    m_chainPosition    = -1;
    m_visitedThisTurn.clear();
    m_lastDirRow       = 0;
    m_lastDirCol       = 0;

    m_halfMoveClock    = 0;
    m_positionHistory.clear();
    recordBoardState();

    emit boardChanged();
}

int FanoronaGame::currentPlayer() const { return m_currentPlayer; }
QString FanoronaGame::gameName() const { return "Fanorona"; }
bool FanoronaGame::isGameOver() const { return m_winner != -2; }
int  FanoronaGame::getWinner() const { return m_winner; }

QVariant FanoronaGame::getBoardState() const
{
    QVariantList boardList;
    for (int i = 0; i < CELLS; ++i)
        boardList.append(m_board[i]);

    QVariantMap state;
    state["board"]           = boardList;
    state["currentPlayer"]   = m_currentPlayer;
    state["piecesOnBoard0"]  = m_piecesOnBoard[0];
    state["piecesOnBoard1"]  = m_piecesOnBoard[1];
    state["chainInProgress"] = m_chainInProgress;
    state["chainPosition"]   = m_chainPosition;
    state["winner"]          = m_winner;
    return state;
}

// ── هندسه‌ی تخته ──

bool FanoronaGame::inBounds(int row, int col)
{
    return row >= 0 && row < ROWS && col >= 0 && col < COLS;
}

bool FanoronaGame::hasDiagonal(int row, int col)
{
    return (row + col) % 2 == 0;
}

bool FanoronaGame::isValidDirection(int fromRow, int fromCol, int dRow, int dCol)
{
    if (dRow == 0 && dCol == 0)
        return false;
    if (dRow != 0 && dCol != 0)
        return hasDiagonal(fromRow, fromCol);
    return true;
}

// ── منطق capture ──

QVector<int> FanoronaGame::findCaptureLine(int row, int col, int dRow, int dCol, int opponentVal) const
{
    QVector<int> result;
    int r = row, c = col;

    while (isValidDirection(r, c, dRow, dCol)) {
        r += dRow;
        c += dCol;
        if (!inBounds(r, c))
            break;
        const int idx = r * COLS + c;
        if (m_board[idx] != opponentVal)
            break;
        result.append(idx);
    }
    return result;
}

bool FanoronaGame::moveHasCapture(int from, int to) const
{
    const int fromRow = from / COLS, fromCol = from % COLS;
    const int toRow   = to / COLS,   toCol   = to % COLS;
    const int dRow = toRow - fromRow, dCol = toCol - fromCol;

    const int mover    = m_board[from];
    const int opponent = (mover == 1) ? 2 : 1;

    if (!findCaptureLine(toRow, toCol, dRow, dCol, opponent).isEmpty())
        return true;

    if (!findCaptureLine(fromRow, fromCol, -dRow, -dCol, opponent).isEmpty())
        return true;

    return false;
}

bool FanoronaGame::hasAnyCaptureAvailable(int player) const
{
    const int val = player + 1;
    static const int dirs[8][2] = {
        {-1,-1}, {-1,0}, {-1,1},
        { 0,-1},         { 0,1},
        { 1,-1}, { 1,0}, { 1,1}
    };

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (m_board[row * COLS + col] != val)
                continue;
            for (const auto& d : dirs) {
                if (!isValidDirection(row, col, d[0], d[1]))
                    continue;
                const int toRow = row + d[0], toCol = col + d[1];
                if (!inBounds(toRow, toCol))
                    continue;
                const int toIdx = toRow * COLS + toCol;
                if (m_board[toIdx] != 0)
                    continue;
                if (moveHasCapture(row * COLS + col, toIdx))
                    return true;
            }
        }
    }
    return false;
}

// ── بررسی دقیق امکان ادامه زنجیره برای یک مهره خاص ──
bool FanoronaGame::hasChainCaptureAvailable(int fromIdx) const
{
    if (fromIdx < 0 || fromIdx >= CELLS)
        return false;

    const int val = m_board[fromIdx];
    if (val == 0)
        return false;

    const int opponentVal = (val == 1) ? 2 : 1;
    const int fromRow = fromIdx / COLS;
    const int fromCol = fromIdx % COLS;

    static const int dirs[8][2] = {
        {-1,-1}, {-1,0}, {-1,1},
        { 0,-1},         { 0,1},
        { 1,-1}, { 1,0}, { 1,1}
    };

    for (const auto& d : dirs) {
        int dRow = d[0];
        int dCol = d[1];

        // ۱. نباید دقیقاً در همون جهت حرکت قبلی در این زنجیره باشه
        if (dRow == m_lastDirRow && dCol == m_lastDirCol)
            continue;

        // ۲. جهت باید روی خطوط تخته مجاز باشه
        if (!isValidDirection(fromRow, fromCol, dRow, dCol))
            continue;

        int toRow = fromRow + dRow;
        int toCol = fromCol + dCol;

        if (!inBounds(toRow, toCol))
            continue;

        int toIdx = toRow * COLS + toCol;

        // ۳. خانه مقصد باید خالی باشه
        if (m_board[toIdx] != 0)
            continue;

        // ۴. خانه مقصد نباید قبلاً در این نوبت ملاقات شده باشه
        if (m_visitedThisTurn.contains(toIdx))
            continue;

        // ۵. حرکت باید حتماً منجر به زدن مهره (Approach یا Withdrawal) بشه
        bool hasApproach   = !findCaptureLine(toRow, toCol, dRow, dCol, opponentVal).isEmpty();
        bool hasWithdrawal = !findCaptureLine(fromRow, fromCol, -dRow, -dCol, opponentVal).isEmpty();

        if (hasApproach || hasWithdrawal)
            return true;
    }

    return false;
}

bool FanoronaGame::hasAnyLegalMove(int player) const
{
    const int val = player + 1;
    static const int dirs[8][2] = {
        {-1,-1}, {-1,0}, {-1,1},
        { 0,-1},         { 0,1},
        { 1,-1}, { 1,0}, { 1,1}
    };

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (m_board[row * COLS + col] != val)
                continue;
            for (const auto& d : dirs) {
                if (!isValidDirection(row, col, d[0], d[1]))
                    continue;
                const int toRow = row + d[0], toCol = col + d[1];
                if (inBounds(toRow, toCol) && m_board[toRow * COLS + toCol] == 0)
                    return true;
            }
        }
    }
    return false;
}

void FanoronaGame::switchPlayer() { m_currentPlayer = 1 - m_currentPlayer; }

void FanoronaGame::recordBoardState()
{
    QString stateStr;
    for (int i = 0; i < CELLS; ++i)
        stateStr.append(QString::number(m_board[i]));
    stateStr.append(QString("|%1").arg(m_currentPlayer));

    m_positionHistory[stateStr]++;
}

void FanoronaGame::endTurn()
{
    m_chainInProgress = false;
    m_chainPosition   = -1;
    m_visitedThisTurn.clear();
    m_lastDirRow = 0;
    m_lastDirCol = 0;

    switchPlayer();
    recordBoardState();
    checkWinCondition();
}

void FanoronaGame::checkWinCondition()
{
    // ۱. تمام شدن مهره‌های یکی از بازیکنان
    if (m_piecesOnBoard[0] == 0) { m_winner = 1; emit gameEnded(m_winner); return; }
    if (m_piecesOnBoard[1] == 0) { m_winner = 0; emit gameEnded(m_winner); return; }

    // ۲. عدم وجود حرکت قانونی (بن‌بست)
    if (!hasAnyLegalMove(m_currentPlayer)) {
        m_winner = 1 - m_currentPlayer;
        emit gameEnded(m_winner);
        return;
    }

    // ۳. قانون ۵۰ حرکت بدون capture (معادل ۱۰۰ نیم‌حرکت)
    if (m_halfMoveClock >= 100) {
        m_winner = -1; // مساوی
        emit gameEnded(m_winner);
        return;
    }

    // ۴. قانون تکرار ۳ باره یک وضعیت مشخص تخته
    QString currentStateStr;
    for (int i = 0; i < CELLS; ++i)
        currentStateStr.append(QString::number(m_board[i]));
    currentStateStr.append(QString("|%1").arg(m_currentPlayer));

    if (m_positionHistory.value(currentStateStr, 0) >= 3) {
        m_winner = -1; // مساوی
        emit gameEnded(m_winner);
        return;
    }
}

bool FanoronaGame::moveNeedsCaptureTypeChoice(int from, int to) const
{
    if (from < 0 || from >= CELLS || to < 0 || to >= CELLS)
        return false;
    if (m_board[from] == 0 || m_board[to] != 0)
        return false;

    const int fromRow = from / COLS, fromCol = from % COLS;
    const int toRow   = to / COLS,   toCol   = to % COLS;
    const int dRow = toRow - fromRow, dCol = toCol - fromCol;

    if (qAbs(dRow) > 1 || qAbs(dCol) > 1)
        return false;
    if (!isValidDirection(fromRow, fromCol, dRow, dCol))
        return false;

    const int mover       = m_board[from];
    const int opponentVal = (mover == 1) ? 2 : 1;

    const bool hasApproach   = !findCaptureLine(toRow, toCol, dRow, dCol, opponentVal).isEmpty();
    const bool hasWithdrawal = !findCaptureLine(fromRow, fromCol, -dRow, -dCol, opponentVal).isEmpty();

    return hasApproach && hasWithdrawal;
}

bool FanoronaGame::makeMove(int player, const QVariant& moveData)
{
    if (isGameOver())
        return false;
    if (player != m_currentPlayer)
        return false;

    const QVariantMap data = moveData.toMap();

    // پایان داوطلبانه زنجیره زدن توسط بازیکن
    if (m_chainInProgress && data.value("endTurn").toBool()) {
        endTurn();
        emit boardChanged();
        return true;
    }

    if (!data.contains("from") || !data.contains("to"))
        return false;

    const int from = data.value("from").toInt();
    const int to   = data.value("to").toInt();

    if (from < 0 || from >= CELLS || to < 0 || to >= CELLS)
        return false;

    // در نوبت زنجیره‌ای، فقط همان مهره‌ی قبلی اجازه حرکت دارد
    if (m_chainInProgress && from != m_chainPosition)
        return false;

    const int val = player + 1;
    if (m_board[from] != val)
        return false;
    if (m_board[to] != 0)
        return false;

    const int fromRow = from / COLS, fromCol = from % COLS;
    const int toRow   = to / COLS,   toCol   = to % COLS;
    const int dRow = toRow - fromRow, dCol = toCol - fromCol;

    if (qAbs(dRow) > 1 || qAbs(dCol) > 1)
        return false;
    if (!isValidDirection(fromRow, fromCol, dRow, dCol))
        return false;

    // اعمال محدودیت‌های زنجیره
    if (m_chainInProgress && dRow == m_lastDirRow && dCol == m_lastDirCol)
        return false;
    if (m_chainInProgress && m_visitedThisTurn.contains(to))
        return false;

    const int opponent = 1 - player;
    const int opponentVal = (val == 1) ? 2 : 1;

    QVector<int> approachCaptures   = findCaptureLine(toRow, toCol, dRow, dCol, opponentVal);
    QVector<int> withdrawalCaptures = findCaptureLine(fromRow, fromCol, -dRow, -dCol, opponentVal);

    const bool hasApproach   = !approachCaptures.isEmpty();
    const bool hasWithdrawal = !withdrawalCaptures.isEmpty();

    // قانون زدن اجباری: اگر زدن مهره ممکن باشد، حرکت عادی مجاز نیست
    const bool captureAvailableNow = hasAnyCaptureAvailable(player);
    if (!m_chainInProgress && captureAvailableNow && !hasApproach && !hasWithdrawal)
        return false;

    // ادامه زنجیره فقط با حرکات زدن مهره مجاز است
    if (m_chainInProgress && !hasApproach && !hasWithdrawal)
        return false;

    QVector<int> capturedCells;
    if (hasApproach && hasWithdrawal) {
        const QString type = data.value("captureType").toString();
        if (type == "approach")
            capturedCells = approachCaptures;
        else if (type == "withdrawal")
            capturedCells = withdrawalCaptures;
        else
            return false; // باید انتخاب مشخص باشد
    } else if (hasApproach) {
        capturedCells = approachCaptures;
    } else if (hasWithdrawal) {
        capturedCells = withdrawalCaptures;
    }

    // اعمال تغییرات روی تخته
    m_board[from] = 0;
    m_board[to]   = val;

    for (int cell : capturedCells) {
        m_board[cell] = 0;
        m_piecesOnBoard[opponent]--;
    }

    if (!capturedCells.isEmpty()) {
        m_halfMoveClock = 0; // ریست شمارنده حرکات بدون زدن

        m_visitedThisTurn.append(from);
        m_lastDirRow = dRow;
        m_lastDirCol = dCol;
        m_chainPosition = to;

        // بررسی اینکه آیا مهره منتقل شده امکان ادامه زنجیره را دارد یا خیر
        if (hasChainCaptureAvailable(to)) {
            m_chainInProgress = true;
            emit boardChanged();
            return true;
        }
    } else {
        m_halfMoveClock++; // افزایش شمارنده در حرکت عادی
    }

    endTurn();
    emit boardChanged();
    return true;
}