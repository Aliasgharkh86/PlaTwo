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
    explicit RecoveryWindow(QWidget *parent = nullptr);
    ~RecoveryWindow();

private:
    Ui::RecoveryWindow *ui;
};

#endif // RECOVERYWINDOW_H
