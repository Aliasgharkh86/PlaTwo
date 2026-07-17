#ifndef EDITPROFILEWINDOW_H
#define EDITPROFILEWINDOW_H

#include <QWidget>
#include "models/user.h"

namespace Ui {
class EditProfileWindow;
}

class EditProfileWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EditProfileWindow(QWidget *parent = nullptr);
    ~EditProfileWindow();

    void setCurrentUser(const User &user);
signals:
    // ------ این دو خط را اضافه کنید ------
    void backRequested();
    void profileUpdated(const User &user);
private slots:
    // ------ این دو خط را اضافه کنید ------
    void on_backBtn_clicked();
    void on_saveBtn_clicked();
private:
    Ui::EditProfileWindow *ui;

    User m_currentUser;
};

#endif // EDITPROFILEWINDOW_H
