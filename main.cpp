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
#include "models/user.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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

        // خروج از منو → برگشت به login
        QObject::connect(menu, &MainMenuWindow::loggedOut, [=]() {
            menu->hide();
            menu->deleteLater();
            login->show();
        });

        // TODO: وصل کردن دکمه‌های بازی به GameLobbyWindow
        // QObject::connect(menu, &MainMenuWindow::dotsAndBoxesSelected, ...);
        // QObject::connect(menu, &MainMenuWindow::nineMensMorrisSelected, ...);
        // QObject::connect(menu, &MainMenuWindow::fanoronaSelected, ...);

        menu->show();
    });

    login->show();
    return a.exec();
}
