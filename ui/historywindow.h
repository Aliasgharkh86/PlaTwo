#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "../models/user.h"
#include "../models/gamerecord.h"

// ─────────────────────────────────────────────
//  HistoryWindow
//  نمایش تاریخچه بازی‌های کاربر + آمار برد/باخت
//  بدون .ui — همه چیز با کد ساخته می‌شه
// ─────────────────────────────────────────────

class HistoryWindow : public QWidget
{
    Q_OBJECT

public:
    // تغییر ۱: پارامتر gameType اضافه شد
    // تا بدونیم از کدوم بازی وارد شدیم و لابی رو
    // با همون gameType باز کنیم
    explicit HistoryWindow(const User& user,
                           const QString& gameType,
                           QWidget* parent = nullptr);

signals:
    void backToMenu();
    void startNewGame(); // تغییر ۲: این signal اضافه شد

private slots:
    void onFilterChanged(int index);
    void onBackClicked();
    void onStartGameClicked(); // تغییر ۳: اسلات دکمه‌ی شروع بازی

private:
    // ── Setup ──────────────────────────────────
    void setupUi();
    void applyStyles();

    // ── Data ───────────────────────────────────
    void loadHistory(const QString& gameType = "");
    void updateStats(const QString& gameType = "");
    void populateTable(const QList<GameRecord>& records);

    // ── Helper ─────────────────────────────────
    QString resultEmoji(GameResult result) const;

    // ── Widgets ────────────────────────────────
    QLabel*       m_titleLabel;
    QLabel*       m_subtitleLabel;

    // کارت‌های آمار
    QLabel*       m_winLabel;
    QLabel*       m_loseLabel;
    QLabel*       m_drawLabel;
    QLabel*       m_totalLabel;

    // فیلتر و جدول
    QComboBox*    m_filterCombo;
    QTableWidget* m_table;

    // دکمه‌ها
    QPushButton*  m_backBtn;
    QPushButton*  m_startGameBtn; // تغییر ۳: دکمه‌ی جدید

    // ── State ──────────────────────────────────
    User    m_currentUser;
    QString m_gameType; // تغییر ۱: ذخیره‌ی نوع بازی
};

#endif // HISTORYWINDOW_H
