#ifndef FANORONAGAMEWIDGET_H
#define FANORONAGAMEWIDGET_H

#include <QWidget>
#include <QString>
#include "games/fanoronagame.h"

class QPushButton;
class QPainter;
class QMouseEvent;
class QResizeEvent;
class QPaintEvent;

class FanoronaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FanoronaWidget(QWidget* parent = nullptr);
    FanoronaWidget(FanoronaGame* game, int myPlayer, QWidget* parent = nullptr);

    void setGame(FanoronaGame* game, int myPlayer);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onBoardChanged();
    void onGameEnded(int winner);
    void onEndTurnClicked();

private:
    static const int ROWS = 5;
    static const int COLS = 9;

    FanoronaGame* m_game;
    int m_myPlayer;
    int m_selectedPoint;

    QPushButton* m_endTurnBtn;
    QString m_feedbackMessage;

    void updateEndTurnButtonVisibility();
    bool isMyTurn() const;
    QPoint pointPosition(int index) const;
    int pointAt(const QPoint& pos) const;
    void attemptMove(int from, int to);

    void drawBoardLines(QPainter& p) const;
    void drawPoints(QPainter& p) const;
    void drawPieces(QPainter& p) const;
    void drawStatusText(QPainter& p) const;


signals:
    void moveReadyToSend(const QVariantMap& moveData); // 👈 این خط باید اضافه شود
};

#endif // FANORONAGAMEWIDGET_H