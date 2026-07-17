#include "mainmenuwindow.h"
#include "ui_mainmenuwindow.h"
#include <QMessageBox>
#include <QPushButton>
#include "editprofilewindow.h"


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
    // ۱. ساختن شیء از پنجره ویرایش پروفایل
    EditProfileWindow *editWin = new EditProfileWindow(this);

    // ۲. ارسال اطلاعات کاربر فعلی (m_user) به پنجره ویرایش
    editWin->setCurrentUser(m_user);

    // ۳. اتصال سیگنال آپدیت شدن پروفایل برای بروزرسانی رابط کاربری منوی اصلی
    connect(editWin, &EditProfileWindow::profileUpdated, this, [this](const User& updatedUser){
        // آپدیت کردن اطلاعات متغیر کلاس
        this->m_user = updatedUser;

        // آپدیت کردن لیبل خوش‌آمدگویی با نام جدید
        ui->welcomeLabel->setText("خوش آمدید، " + m_user.name + " 👋");
    });

    // ۴. نمایش پنجره (رفع خطای exec)
    // چون کلاس از نوع QWidget است، آن را به عنوان یک پنجره مستقل (Window) تعریف می‌کنیم
    editWin->setWindowFlags(Qt::Window);

    // حالت Modal به آن می‌دهیم تا بقیه برنامه‌ قفل شود تا زمانی که این پنجره بسته شود
    editWin->setWindowModality(Qt::ApplicationModal);

    // برای جلوگیری از نشت حافظه (Memory Leak) وقتی پنجره بسته شد، خودبه‌خود پاک شود
    editWin->setAttribute(Qt::WA_DeleteOnClose);

    // نمایش پنجره
    editWin->show();
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


