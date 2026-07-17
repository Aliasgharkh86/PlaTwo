// #include "mainwindow.h"
// #include "ui/loginwindow.h"
// #include "ui/signupwindow.h"
// #include "ui/recoverywindow.h"

// #include <QApplication>

// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);

//     LoginWindow login;
//     login.show();

//     // SignUpWindow signup;
//     // signup.show();

//     // RecoveryWindow recover;
//     // recover.show();

//     // MainWindow w;
//     // w.show();

//     return a.exec();
// }

// #include "ui/loginwindow.h"
// #include "ui/signupwindow.h"
// #include "ui/recoverywindow.h"
// #include <QApplication>
// #include "ui/mainmenuwindow.h"
// #include "models/user.h"
// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);

//     LoginWindow*   login   = new LoginWindow();
//     SignUpWindow*  signup  = new SignUpWindow();

//     // از signup برو به login
//     QObject::connect(signup, &SignUpWindow::switchToLogin, [=]() {
//         signup->hide();
//         login->show();
//     });

//     // از login برو به signup
//     QObject::connect(login, &LoginWindow::switchToSignup, [=]() {
//         login->hide();
//         signup->show();
//     });

//     // بعد از login موفق
//     QObject::connect(login, &LoginWindow::loginSuccess, [=](User user ) {
//         login->hide();
//         MainMenuWindow* menu = new MainMenuWindow(user);
//         menu->show();
//     });

//     RecoveryWindow* recovery = new RecoveryWindow();

//     // از login برو به recovery
//     QObject::connect(login, &LoginWindow::switchToRecovery, [=]() {
//         login->hide();
//         recovery->show();
//     });

//     // از recovery برگرد به login
//     QObject::connect(recovery, &RecoveryWindow::switchToLogin, [=]() {
//         recovery->hide();
//         login->show();
//     });

//     login->show();
//     return a.exec();
// }

#include <QApplication>
#include "ui/loginwindow.h"
#include "ui/signupwindow.h"
#include "ui/recoverywindow.h"
#include "ui/mainmenuwindow.h"
#include "ui/historywindow.h"
#include "models/user.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("PlaTwo");

    LoginWindow*    login    = new LoginWindow();
    SignUpWindow*   signup   = new SignUpWindow();
    RecoveryWindow* recovery = new RecoveryWindow();

    // ── login ↔ signup ────────────────────────
    QObject::connect(login, &LoginWindow::switchToSignup, [=]() {
        login->hide();
        signup->show();
    });
    QObject::connect(signup, &SignUpWindow::switchToLogin, [=]() {
        signup->hide();
        login->show();
    });

    // ── login ↔ recovery ──────────────────────
    QObject::connect(login, &LoginWindow::switchToRecovery, [=]() {
        login->hide();
        recovery->show();
    });
    QObject::connect(recovery, &RecoveryWindow::switchToLogin, [=]() {
        recovery->hide();
        login->show();
    });

    // ── signup موفق → login ───────────────────
    QObject::connect(signup, &SignUpWindow::signUpSuccess, [=]() {
        signup->hide();
        login->show();
    });

    // ── login موفق → منوی اصلی ───────────────
    QObject::connect(login, &LoginWindow::loginSuccess, [=](User user) {
        login->hide();

        MainMenuWindow* menu = new MainMenuWindow(user);

        QObject::connect(menu, &MainMenuWindow::loggedOut, [=]() {
            menu->hide();
            menu->deleteLater();
            login->show();
        });

        // هر دکمه بازی → صفحه اختصاصی اون بازی
        // که شامل تاریخچه + شروع بازی جدید هست
        auto openHistory = [=](const QString& gameType) {
            HistoryWindow* history = new HistoryWindow(user, gameType);

            QObject::connect(history, &HistoryWindow::backToMenu, [history, menu]() {
                history->hide();
                history->deleteLater();
                menu->show();
            });

            menu->hide();
            history->show();
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

