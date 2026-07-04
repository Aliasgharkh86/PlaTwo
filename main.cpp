#include "mainwindow.h"
#include "ui/loginwindow.h"
#include "ui/signupwindow.h"
#include "ui/recoverywindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWindow login;
    login.show();

    // SignUpWindow signup;
    // signup.show();

    // RecoveryWindow recover;
    // recover.show();

    // MainWindow w;
    // w.show();

    return a.exec();
}


