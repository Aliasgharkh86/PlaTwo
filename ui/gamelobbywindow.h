#ifndef GAMELOBBYWINDOW_H
#define GAMELOBBYWINDOW_H

#include <QWidget>
#include "network/gameserver.h"

namespace Ui {
class GameLobbyWindow;
}

class GameLobbyWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameLobbyWindow(QWidget *parent = nullptr);
    ~GameLobbyWindow();

private slots:
    void on_hostButton_clicked();
    void onGameReady();
    void onGameAborted();

private:
    Ui::GameLobbyWindow *ui;
    GameServer *server;
};

#endif
