#ifndef GAMEBOARDWINDOW_H
#define GAMEBOARDWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QVariantMap>

#include "../models/user.h"
#include "../models/gamerecord.h"
#include "../core/storagemanager.h"
#include "games/game.h"
#include "../network/gameclient.h"
#include "chatwidget.h"

class GameBoardWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameBoardWindow(Game* game, QWidget* boardWidget, GameClient* client,
                             int myPlayer, const User& currentUser,
                             const QString& gameTypeStr,
                             const QString& opponentUsername,
                             QWidget* parent = nullptr);
    ~GameBoardWindow() override = default;

signals:
    void backToMenuRequested();

private slots:
    void onMoveReadyToSend(const QVariantMap& moveData);
    void onOpponentMoved(int row, int col, const QString& extra);
    void onOpponentDisconnected();
    void onGameOver(const QString& winner, const QString& reason);

private:
    Game*        m_game;
    QWidget*     m_boardWidget;
    GameClient*  m_client;
    int          m_myPlayer; // 0 = Host, 1 = Guest
    ChatWidget* m_chatWidget = nullptr;
    User    m_currentUser;
    QString m_gameTypeStr;
    QString m_opponentUsername;
};

#endif // GAMEBOARDWINDOW_H