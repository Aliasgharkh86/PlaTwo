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
    explicit HistoryWindow(const User& user,
                           const QString& gameType = "",
                           QWidget* parent = nullptr);
signals:
    void backToMenu();

private slots:
    void onFilterChanged(int index);
    void onBackClicked();

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

    // دکمه بازگشت
    QPushButton*  m_backBtn;

    // ── State ──────────────────────────────────
    User m_currentUser;
};

#endif // HISTORYWINDOW_H
