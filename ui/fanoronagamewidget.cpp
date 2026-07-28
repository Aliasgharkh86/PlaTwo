#include "fanoronagamewidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QVariantMap>
#include <cmath>

FanoronaWidget::FanoronaWidget(QWidget* parent)
    : QWidget(parent)
    , m_game(nullptr)
    , m_myPlayer(-1)
    , m_selectedPoint(-1)
{
    setMinimumSize(520, 340);

    m_endTurnBtn = new QPushButton("پایان نوبت (توقف زنجیره)", this);
    m_endTurnBtn->hide();
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

    // اعمال حرکت پایان نوبت و ارسال سیگنال به سرور
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
    const int btnW = 200, btnH = 28;
    m_endTurnBtn->setGeometry(width() - btnW - 10, height() - btnH - 34, btnW, btnH);
}

bool FanoronaWidget::isMyTurn() const
{
    if (!m_game || m_game->isGameOver())
        return false;
    if (m_myPlayer == -1)
        return true; // حالت تست محلی (hotseat)
    return m_game->currentPlayer() == m_myPlayer;
}

QPoint FanoronaWidget::pointPosition(int index) const
{
    const int row = index / COLS;
    const int col = index % COLS;

    const int margin = 30;
    const int usableW = width()  - 2 * margin;
    const int usableH = height() - 2 * margin - 30; // فضای متن وضعیت پایین

    const int x = margin + (col * usableW) / (COLS - 1);
    const int y = margin + (row * usableH) / (ROWS - 1);
    return QPoint(x, y);
}

int FanoronaWidget::pointAt(const QPoint& pos) const
{
    const int threshold = 15;
    for (int i = 0; i < ROWS * COLS; ++i) {
        const QPoint p = pointPosition(i);
        const int dx = p.x() - pos.x();
        const int dy = p.y() - pos.y();
        if (std::sqrt(double(dx * dx + dy * dy)) <= threshold)
            return i;
    }
    return -1;
}

void FanoronaWidget::drawBoardLines(QPainter& p) const
{
    p.setPen(QPen(Qt::black, 1));
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            const int idx = row * COLS + col;
            // خط به سمت راست
            if (col + 1 < COLS)
                p.drawLine(pointPosition(idx), pointPosition(idx + 1));
            // خط به سمت پایین
            if (row + 1 < ROWS)
                p.drawLine(pointPosition(idx), pointPosition(idx + COLS));
            // خطوط قطر فقط روی خونه‌هایی که row+col زوجه
            if ((row + col) % 2 == 0) {
                if (row + 1 < ROWS && col + 1 < COLS)
                    p.drawLine(pointPosition(idx), pointPosition(idx + COLS + 1));
                if (row + 1 < ROWS && col - 1 >= 0)
                    p.drawLine(pointPosition(idx), pointPosition(idx + COLS - 1));
            }}
    }
}

void FanoronaWidget::drawPoints(QPainter& p) const
{
    p.setPen(QPen(Qt::black, 1));
    p.setBrush(Qt::white);
    for (int i = 0; i < ROWS * COLS; ++i) {
        const QPoint pt = pointPosition(i);
        if (i == m_selectedPoint) {
            p.setBrush(QColor(255, 230, 120));
            p.drawEllipse(pt, 7, 7);
            p.setBrush(Qt::white);
        } else {
            p.drawEllipse(pt, 4, 4);
        }
    }
}
void FanoronaWidget::drawPieces(QPainter& p) const
{
    const QVariantMap state = m_game->getBoardState().toMap();
    const QVariantList board = state.value("board").toList();

    for (int i = 0; i < ROWS * COLS; ++i) {
        const int val = board.at(i).toInt();
        if (val == 0)
            continue;
        const QPoint pt = pointPosition(i);
        p.setPen(QPen(Qt::black, 2));

        // 👈 مقدار 1 حالا نارنجی است، مقدار 2 خاکستری
        p.setBrush(val == 1 ? QColor(255, 140, 0) : QColor(128, 128, 128));
        p.drawEllipse(pt, 10, 10);
    }
}

void FanoronaWidget::drawStatusText(QPainter& p) const
{
    QString text;
    bool isError = false;
    QVariantMap state = m_game->getBoardState().toMap();

    if (!m_feedbackMessage.isEmpty()) {
        text = m_feedbackMessage;
        isError = true;
    } else if (m_game->isGameOver()) {
        const int winner = m_game->getWinner();

        // بررسی حالت تساوی که در لاجیک اضافه شده بود
        if (winner == -1) {
            text = "🤝  بازی مساوی شد!";
        } else {
            text = (m_myPlayer == -1)
            ? QString("🎉  بازیکن %1 برد!").arg(winner + 1)
            : (winner == m_myPlayer ? "🎉  شما بردید!" : "😔  حریف برد.");
        }
    } else if (state.value("chainInProgress").toBool()) {
        text = "می‌تونید با همون مهره capture دیگه‌ای بزنید، یا نوبت رو تموم کنید.";
    } else if (isMyTurn()) {
        text = (m_myPlayer == -1)
        ? QString("نوبت بازیکن %1").arg(m_game->currentPlayer() + 1)
        : "نوبت شماست.";
    } else {
        text = "در انتظار حرکت حریف...";
    }

    p.setPen(isError ? QColor(200, 30, 30) : Qt::black);
    QFont f = p.font();
    f.setPointSize(10);
    p.setFont(f);
    p.drawText(QRect(0, height() - 26, width(), 26), Qt::AlignCenter, text);
}

void FanoronaWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBoardLines(p);
    drawPoints(p);

    if (!m_game)
        return;

    drawPieces(p);
    drawStatusText(p);
}

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

        // اعمال حرکت، بررسی زنجیره و ارسال به سرور
        if (m_game->makeMove(effectivePlayer, move)) {
            QVariantMap state = m_game->getBoardState().toMap();
            if (state.value("chainInProgress").toBool()) {
                move["keepTurn"] = true; // در صورت ادامه زنجیره، نوبت را از بازیکن نگیر
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

    // اعمال حرکت، بررسی زنجیره و ارسال به سرور (برای حرکات عادی)
    if (m_game->makeMove(effectivePlayer, move)) {
        QVariantMap state = m_game->getBoardState().toMap();
        if (state.value("chainInProgress").toBool()) {
            move["keepTurn"] = true; // در صورت ادامه زنجیره، نوبت را از بازیکن نگیر
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

    // اگه وسط زنجیره‌ایم، فقط اجازه‌ی حرکت با همون مهره (استخراج شده از state) رو داریم
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