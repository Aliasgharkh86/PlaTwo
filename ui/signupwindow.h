#ifndef SIGNUPWINDOW_H
#define SIGNUPWINDOW_H

#include <QDialog>

namespace Ui {
class SignUpWindow;
}

class SignUpWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SignUpWindow(QDialog* parent = nullptr);
    ~SignUpWindow();

<<<<<<< HEAD
signals:
    void signUpSuccess();
    void switchToLogin();

private slots:
    void onSignupClicked();
=======
private slots:
    void on_backToLoginButton_clicked();

    void on_pushButton_clicked();
>>>>>>> 2e07b7b58785751398bd5f4b321672e593d92e14

private:
    Ui::SignUpWindow* ui;
};

#endif