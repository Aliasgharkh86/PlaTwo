#include "mainmenuwindow.h"
#include "ui_mainmenuwindow.h"
#include <QMessageBox>
#include <QPushButton>

MainMenuWindow::MainMenuWindow(const User& user, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::MainMenuWindow)
    , m_user(user)
{
    ui->setupUi(this);
    setFixedSize(420, 520);
    setWindowTitle("PlaTwo - منوی اصلی");

    ui->welcomeLabel->setText("خوش آمدید، " + m_user.name + " 👋");

    connect(ui->dotsBtn,     &QPushButton::clicked,
            this,             &MainMenuWindow::onDotsClicked);
    connect(ui->morrisBtn,   &QPushButton::clicked,
            this,             &MainMenuWindow::onMorrisClicked);
    connect(ui->fanoronaBtn, &QPushButton::clicked,
            this,             &MainMenuWindow::onFanoronaClicked);
    connect(ui->editBtn,     &QPushButton::clicked,
            this,             &MainMenuWindow::onEditClicked);
    connect(ui->exitBtn,     &QPushButton::clicked,
            this,             &MainMenuWindow::onLogoutClicked);
}

MainMenuWindow::~MainMenuWindow()
{
    delete ui;
}

void MainMenuWindow::onDotsClicked()
{
    emit dotsAndBoxesSelected(m_user);
}

void MainMenuWindow::onMorrisClicked()
{
    emit nineMensMorrisSelected(m_user);
}

void MainMenuWindow::onFanoronaClicked()
{
    emit fanoronaSelected(m_user);
}

void MainMenuWindow::onEditClicked()
{
    emit editProfileSelected(m_user);
}

void MainMenuWindow::onLogoutClicked()
{
    auto reply = QMessageBox::question(
        this, "خروج",
        "آیا می‌خواهید از حساب کاربری خارج شوید؟",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
        emit loggedOut();
}
