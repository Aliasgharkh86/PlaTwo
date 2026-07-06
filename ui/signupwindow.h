#ifndef SIGNUPWINDOW_H
#define SIGNUPWINDOW_H
#include <QDialog>
#include <QWidget>
#include <QPushButton>

namespace Ui {
class SignUpWindow;
}

class SignUpWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SignUpWindow(QWidget* parent = nullptr);
    ~SignUpWindow();

signals:
    void switchToLogin();
        void signUpSuccess();   // ← اضافه کن
private slots:
    void on_pushButton_clicked();       // دکمه ثبت‌نام
    void on_backToLoginButton_clicked(); // دکمه بازگشت

private:
    Ui::SignUpWindow* ui;
};

#endif