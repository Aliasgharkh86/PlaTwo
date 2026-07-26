#ifndef DOTSANDBOXESWIDGET_H
#define DOTSANDBOXESWIDGET_H

#include <QWidget>
#include <QVariantMap>
#include "games/dotsandboxesgame.h"

class DotsAndBoxesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DotsAndBoxesWidget(DotsAndBoxesGame* game, int myPlayer = -1, QWidget *parent = nullptr);
    ~DotsAndBoxesWidget() override = default;

signals:
    void moveReadyToSend(const QVariantMap& move);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    DotsAndBoxesGame* m_game;
    int m_myPlayer;
    int m_cellSize;
    int m_margin;
    int m_dotRadius;
    int m_lineThickness;
    int m_hitRadius;
};

#endif
