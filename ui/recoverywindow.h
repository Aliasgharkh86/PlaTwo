#ifndef RECOVERYWINDOW_H
#define RECOVERYWINDOW_H

#include <QDialog>

namespace Ui {
class RecoveryWindow;
}

class RecoveryWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RecoveryWindow(QWidget* parent = nullptr);
    ~RecoveryWindow();

signals:
    // وقتی کاربر رمز رو عوض کرد و خواست بره به Login
    void switchToLogin();

private slots:
    void on_confirmBtn_clicked();      // دکمه "تغییر رمز عبور"
    void on_backBtn_clicked();         // دکمه "بازگشت به ورود"

private:
    Ui::RecoveryWindow* ui;
};

#endif // RECOVERYWINDOW_H
