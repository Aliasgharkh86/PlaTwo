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

signals:
    void signUpSuccess();
    void switchToLogin();

private slots:
    void onSignupClicked();

private:
    Ui::SignUpWindow* ui;
};

#endif