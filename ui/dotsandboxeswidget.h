#ifndef DOTSANDBOXESWIDGET_H
#define DOTSANDBOXESWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QVariantMap>
#include "games/dotsandboxesgame.h"

class DotsAndBoxesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DotsAndBoxesWidget(DotsAndBoxesGame* game,
                                int myPlayer = -1,
                                const QString& player1Name = "Player 1",
                                const QString& player2Name = "Player 2",
                                QWidget* parent = nullptr);

    void setPlayerNames(const QString& p1Name, const QString& p2Name);

signals:
    void moveReadyToSend(const QVariantMap& moveData);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    struct LineHoverInfo {
        QString type; // "h" یا "v"
        int row = -1;
        int col = -1;
        bool valid = false;
    };

    bool isMyTurn() const;
    LineHoverInfo hitTestLine(const QPoint& pos) const;

    DotsAndBoxesGame* m_game;
    int m_myPlayer; // -1: بازی محلی | 0: کلاینت اول | 1: کلاینت دوم

    QString m_player1Name;
    QString m_player2Name;

    LineHoverInfo m_hoveredLine;
};

#endif // DOTSANDBOXESWIDGET_H