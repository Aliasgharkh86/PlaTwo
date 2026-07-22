#ifndef GAMELOBBYWINDOW_H
#define GAMELOBBYWINDOW_H

#include <QWidget>
#include "../network/gameclient.h"
// #include "../network/gameserver.h"
#include "../network/gameroom.h"
#include "../models/user.h"

namespace Ui {
class GameLobbyWindow;
}

class GameLobbyWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameLobbyWindow(const User& currentUser,
                             GameType gameType,
                             QWidget* parent = nullptr);
    ~GameLobbyWindow();

signals:
    void gameReady(GameClient* client,
                   const QString& player1,
                   const QString& player2,
                   int boardSize,
                   bool hasTimer,
                   int timerSeconds);
    void backToMenu();

private slots:
    // صفحه 0
    void onHostClicked();
    void onGuestClicked();
    // صفحه 1
    void onTimerCheckToggled(bool checked);
    void onStartHostClicked();
    void onBackFromHostClicked();
    // صفحه 2
    void onStopHostClicked();
    void onPlayerConnected(int slot, const QString& username);
    void onServerGameStarted(const QString& p1, const QString& p2);
    // صفحه 3
    void onConnectClicked();
    void onBackFromGuestClicked();
    // صفحه 4
    void onCancelConnectClicked();
    // GameClient
    void onClientConnected();
    void onClientConnectionError(const QString& error);
    void onGameStarted(const QString& p1, const QString& p2,
                       int boardSize, bool hasTimer, int timerSecs);

private:
    QString gameTypeName() const;

    Ui::GameLobbyWindow* ui;
    User        m_currentUser;
    GameType    m_gameType;
    // Gameserver* m_server = nullptr;
    GameClient* m_client;
};

#endif // GAMELOBBYWINDOW_H
