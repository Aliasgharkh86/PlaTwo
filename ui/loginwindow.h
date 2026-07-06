#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H
#include "models/user.h"
#include <QWidget>
#include <QDialog>
namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QDialog *parent = nullptr);
    ~LoginWindow();

private slots:
    void on_signUpButton_clicked();

    void on_recoveryButton_clicked();

    void on_loginButton_clicked();
signals :
    void switchToSignup();
    void loginSuccess(User user);   // ← User رو پاس میده به MainMenu
    void switchToRecovery();   // ← این رو اضافه کن
private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
