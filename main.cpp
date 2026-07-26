#include <QApplication>
#include <QDebug>

// ── پنجره‌های auth ──
#include "ui/loginwindow.h"
#include "ui/signupwindow.h"
#include "ui/recoverywindow.h"

// ── پنجره‌های اصلی ──
#include "ui/mainmenuwindow.h"
#include "ui/historywindow.h"
#include "ui/gamelobbywindow.h"
#include "ui/gameboardwindow.h"

// ── بازی‌ها ──
#include "games/ninemensmorrisgame.h"
#include "games/fanoronagame.h"
// #include "games/dotsandboxesgame.h"

// ── ویجت‌های بازی‌ها ──
#include "ui/ninemensmorriswidget.h"
#include "ui/fanoronagamewidget.h"
// #include "ui/dotsandboxeswidget.h"

// ── مدل‌ها ──
#include "models/user.h"
#include "network/gameclient.h"

static GameType gameTypeFromString(const QString& s)
{
    if (s == "nine_mens_morris") return GameType::NINE_MENS_MORRIS;
    if (s == "fanorona")         return GameType::FANORONA;
    return GameType::DOTS_AND_BOXES;
}

// ─────────────────────────────────────────────
// فقط Game و Widget می‌سازیم — اتصال شبکه داخل
// GameBoardWindow انجام می‌شه (نه اینجا)
// ─────────────────────────────────────────────
static GameBoardWindow* createBoardWindow(const QString& gameTypeStr,
                                          GameClient*    client,
                                          int            myPlayer,
                                          QWidget*       parent = nullptr)
{
    Game*    game        = nullptr;
    QWidget* boardWidget = nullptr;

    if (gameTypeStr == "nine_mens_morris") {
        auto* g = new NineMensMorrisGame(parent);
        auto* w = new NineMensMorrisWidget(g, myPlayer);
        game        = g;
        boardWidget = w;
    }
    else if (gameTypeStr == "fanorona") {
        auto* g = new FanoronaGame(parent);
        auto* w = new FanoronaWidget(g, myPlayer);
        game = g; boardWidget = w;
    }
    else {
        qDebug() << "این بازی هنوز پیاده‌سازی نشده:" << gameTypeStr;
        return nullptr;
    }

    // GameBoardWindow خودش شبکه رو وصل می‌کنه
    return new GameBoardWindow(game, boardWidget, client, myPlayer, parent);
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("PlaTwo");

    auto* login    = new LoginWindow();
    auto* signup   = new SignUpWindow();
    auto* recovery = new RecoveryWindow();

    QObject::connect(login,  &LoginWindow::switchToSignup, [=]() {
        login->hide(); signup->show();
    });
    QObject::connect(signup, &SignUpWindow::switchToLogin, [=]() {
        signup->hide(); login->show();
    });
    QObject::connect(login,    &LoginWindow::switchToRecovery, [=]() {
        login->hide(); recovery->show();
    });
    QObject::connect(recovery, &RecoveryWindow::switchToLogin, [=]() {
        recovery->hide(); login->show();
    });
    QObject::connect(signup, &SignUpWindow::signUpSuccess, [=]() {
        signup->hide(); login->show();
    });

    QObject::connect(login, &LoginWindow::loginSuccess, [=](User user) {
        login->hide();

        auto* menu = new MainMenuWindow(user);

        QObject::connect(menu, &MainMenuWindow::loggedOut, [=]() {
            menu->hide();
            menu->deleteLater();
            login->show();
        });

        auto openHistory = [=](const QString& gameTypeStr) {
            auto* history = new HistoryWindow(user, gameTypeStr);
            menu->hide();
            history->show();

            QObject::connect(history, &HistoryWindow::backToMenu, [=]() {
                history->hide();
                history->deleteLater();
                menu->show();
            });

            QObject::connect(history, &HistoryWindow::startNewGame, [=]() {
                history->hide();

                auto* lobby = new GameLobbyWindow(user, gameTypeFromString(gameTypeStr));
                lobby->show();

                QObject::connect(lobby, &GameLobbyWindow::backToMenu, [=]() {
                    lobby->hide();
                    lobby->deleteLater();
                    history->show();
                });

                QObject::connect(lobby, &GameLobbyWindow::gameReady,
                                 [=](GameClient* client, const QString& p1,
                                                                         const QString& p2, int boardSize,
                                                                         bool hasTimer, int timerSecs)
                                 {
                                     Q_UNUSED(p1) Q_UNUSED(p2)
                                     Q_UNUSED(boardSize) Q_UNUSED(hasTimer) Q_UNUSED(timerSecs)

                                     const int myPlayer = lobby->isHost() ? 0 : 1;

                                     auto* board = createBoardWindow(gameTypeStr, client, myPlayer);
                                     if (!board) return;

                                     lobby->hide();
                                     board->show();

                                     QObject::connect(board, &GameBoardWindow::backToMenuRequested,
                                                      [=]() {
                                                          board->hide();
                                                          board->deleteLater();
                                                          lobby->deleteLater();
                                                          history->show();
                                                      });
                                 });
            });
        };

        QObject::connect(menu, &MainMenuWindow::dotsAndBoxesSelected,
                         [=]() { openHistory("dots_and_boxes"); });
        QObject::connect(menu, &MainMenuWindow::nineMensMorrisSelected,
                         [=]() { openHistory("nine_mens_morris"); });
        QObject::connect(menu, &MainMenuWindow::fanoronaSelected,
                         [=]() { openHistory("fanorona"); });

        menu->show();
    });

    login->show();
    return a.exec();
}
