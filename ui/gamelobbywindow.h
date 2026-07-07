#ifndef GAMELOBBYWINDOW_H
#define GAMELOBBYWINDOW_H

#include <QWidget>

namespace Ui {
class GameLobbyWindow;
}

class GameLobbyWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameLobbyWindow(QWidget *parent = nullptr);
    ~GameLobbyWindow();

private:
    Ui::GameLobbyWindow *ui;
};

#endif // GAMELOBBYWINDOW_H
