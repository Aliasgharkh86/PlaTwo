#ifndef MAINMENUWINDOW_H
#define MAINMENUWINDOW_H

#include <QDialog>
#include "../models/user.h"

namespace Ui {
class MainMenuWindow;
}

class MainMenuWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainMenuWindow(const User& user, QWidget* parent = nullptr);
    ~MainMenuWindow();

signals:
    void dotsAndBoxesSelected(User user);
    void nineMensMorrisSelected(User user);
    void fanoronaSelected(User user);
    void editProfileSelected(User user);
    void historySelected();
    void loggedOut();

private slots:
    void onDotsClicked();
    void onMorrisClicked();
    void onFanoronaClicked();
    void onEditClicked();
    void onLogoutClicked();


private:
    Ui::MainMenuWindow* ui;
    User m_user;
};

#endif // MAINMENUWINDOW_H
