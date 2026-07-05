#ifndef MAINMENUWINDOW_H
#define MAINMENUWINDOW_H

#include <QWidget>

namespace Ui {
class mainmenuwindow;
}

class mainmenuwindow : public QWidget
{
    Q_OBJECT

public:
    explicit mainmenuwindow(QWidget *parent = nullptr);
    ~mainmenuwindow();

private:
    Ui::mainmenuwindow *ui;
};

#endif // MAINMENUWINDOW_H
