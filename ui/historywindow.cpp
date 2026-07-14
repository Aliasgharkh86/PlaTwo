#include "historywindow.h"
#include "../core/storagemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QHeaderView>
#include <QFont>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────

HistoryWindow::HistoryWindow(const User& user,
                             const QString& gameType,
                             QWidget* parent)
    : QWidget(parent)
    , m_currentUser(user)
{
    setupUi();
    applyStyles();

    // اگه gameType مشخص بود، فیلتر رو روی اون بذار
    if (!gameType.isEmpty()) {
        int idx = m_filterCombo->findData(gameType);
        if (idx >= 0)
            m_filterCombo->setCurrentIndex(idx);  // این خودش loadHistory رو صدا می‌زنه
    } else {
        loadHistory();
    }
}
// ─────────────────────────────────────────────
//  setupUi
// ─────────────────────────────────────────────

void HistoryWindow::setupUi()
{
    setMinimumSize(700, 600);
    setWindowTitle("PlaTwo - تاریخچه بازی‌ها");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 24, 32, 24);
    mainLayout->setSpacing(16);

    // ── عنوان ─────────────────────────────────
    m_titleLabel = new QLabel("📋  تاریخچه بازی‌ها", this);
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel(
        "کاربر: " + m_currentUser.username, this);
    m_subtitleLabel->setObjectName("subtitleLabel");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    // ── خط جداکننده ───────────────────────────
    auto* line1 = new QFrame(this);
    line1->setFrameShape(QFrame::HLine);
    line1->setObjectName("separator");

    // ── کارت‌های آمار ─────────────────────────
    m_winLabel   = new QLabel("🏆\n0\nبرد",    this);
    m_loseLabel  = new QLabel("💔\n0\nباخت",   this);
    m_drawLabel  = new QLabel("🤝\n0\nمساوی",  this);
    m_totalLabel = new QLabel("🎮\n0\nکل بازی", this);

    m_winLabel->setObjectName("winCard");
    m_loseLabel->setObjectName("loseCard");
    m_drawLabel->setObjectName("drawCard");
    m_totalLabel->setObjectName("totalCard");

    for (auto* card : {m_winLabel, m_loseLabel,
                       m_drawLabel, m_totalLabel}) {
        card->setAlignment(Qt::AlignCenter);
        card->setMinimumHeight(80);
    }

    auto* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(12);
    statsLayout->addWidget(m_winLabel);
    statsLayout->addWidget(m_loseLabel);
    statsLayout->addWidget(m_drawLabel);
    statsLayout->addWidget(m_totalLabel);

    // ── فیلتر ─────────────────────────────────
    auto* filterLayout = new QHBoxLayout();
    auto* filterLabel  = new QLabel("فیلتر بازی:", this);
    filterLabel->setObjectName("fieldLabel");

    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem("همه بازی‌ها",       "");
    m_filterCombo->addItem("Dots and Boxes",    "dots_and_boxes");
    m_filterCombo->addItem("Nine Men's Morris", "nine_mens_morris");
    m_filterCombo->addItem("Fanorona",          "fanorona");
    m_filterCombo->setMinimumWidth(200);

    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(m_filterCombo);
    filterLayout->addStretch();

    // ── جدول ──────────────────────────────────
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({
        "بازی", "حریف", "نقش", "نتیجه", "امتیاز", "تاریخ"
    });

    m_table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);

    // ── خط و دکمه بازگشت ──────────────────────
    auto* line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine);
    line2->setObjectName("separator");

    m_backBtn = new QPushButton("← بازگشت به منو", this);
    m_backBtn->setObjectName("linkBtn");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setFlat(true);

    // ── چیدمان نهایی ──────────────────────────
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_subtitleLabel);
    mainLayout->addWidget(line1);
    mainLayout->addLayout(statsLayout);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(m_table);
    mainLayout->addWidget(line2);
    mainLayout->addWidget(m_backBtn, 0, Qt::AlignCenter);

    // ── اتصالات ───────────────────────────────
    connect(m_filterCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HistoryWindow::onFilterChanged);

    connect(m_backBtn, &QPushButton::clicked,
            this,       &HistoryWindow::onBackClicked);
}

// ─────────────────────────────────────────────
//  applyStyles
// ─────────────────────────────────────────────

void HistoryWindow::applyStyles()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #1e1e2e;
            color: #cdd6f4;
            font-family: 'Segoe UI';
        }

        QLabel#titleLabel    { color: #cba6f7; }
        QLabel#subtitleLabel { color: #6c7086; font-size: 12px; }
        QLabel#fieldLabel    { color: #a6adc8; font-size: 12px; }

        QFrame#separator { color: #313244; }

        QLabel#winCard {
            background-color: #1e3a2f;
            border: 1px solid #a6e3a1;
            border-radius: 10px;
            color: #a6e3a1;
            font-size: 13px;
            font-weight: bold;
            padding: 6px;
        }
        QLabel#loseCard {
            background-color: #3a1e24;
            border: 1px solid #f38ba8;
            border-radius: 10px;
            color: #f38ba8;
            font-size: 13px;
            font-weight: bold;
            padding: 6px;
        }
        QLabel#drawCard {
            background-color: #2a2a1e;
            border: 1px solid #f9e2af;
            border-radius: 10px;
            color: #f9e2af;
            font-size: 13px;
            font-weight: bold;
            padding: 6px;
        }
        QLabel#totalCard {
            background-color: #1e2a3a;
            border: 1px solid #89b4fa;
            border-radius: 10px;
            color: #89b4fa;
            font-size: 13px;
            font-weight: bold;
            padding: 6px;
        }

        QComboBox {
            background-color: #313244;
            border: 1px solid #45475a;
            border-radius: 8px;
            padding: 6px 12px;
            color: #cdd6f4;
            font-size: 12px;
        }
        QComboBox:focus      { border-color: #cba6f7; }
        QComboBox::drop-down { border: none; }

        QTableWidget {
            background-color: #181825;
            border: 1px solid #313244;
            border-radius: 8px;
            gridline-color: transparent;
            font-size: 12px;
        }
        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #313244;
        }
        QTableWidget::item:selected {
            background-color: #313244;
            color: #cdd6f4;
        }
        QTableWidget::item:alternate {
            background-color: #1e1e2e;
        }
        QHeaderView::section {
            background-color: #313244;
            color: #a6adc8;
            padding: 8px;
            border: none;
            font-weight: bold;
            font-size: 12px;
        }

        QPushButton#linkBtn {
            color: #89b4fa;
            border: none;
            font-size: 12px;
            text-decoration: underline;
            background: transparent;
        }
        QPushButton#linkBtn:hover { color: #cba6f7; }
    )");
}

// ─────────────────────────────────────────────
//  Load & Populate
// ─────────────────────────────────────────────

void HistoryWindow::loadHistory(const QString& gameType)
{
    auto records = StorageManager::instance()
    .getGameHistory(m_currentUser.id, gameType);
    updateStats(gameType);
    populateTable(records);
}

void HistoryWindow::updateStats(const QString& gameType)
{
    auto& db  = StorageManager::instance();
    int wins  = db.getWinCount (m_currentUser.id, gameType);
    int losses= db.getLoseCount(m_currentUser.id, gameType);
    int draws = db.getDrawCount(m_currentUser.id, gameType);
    int total = wins + losses + draws;

    m_winLabel->setText  (QString("🏆\n%1\nبرد")    .arg(wins));
    m_loseLabel->setText (QString("💔\n%1\nباخت")   .arg(losses));
    m_drawLabel->setText (QString("🤝\n%1\nمساوی")  .arg(draws));
    m_totalLabel->setText(QString("🎮\n%1\nکل بازی").arg(total));
}

void HistoryWindow::populateTable(const QList<GameRecord>& records)
{
    m_table->setRowCount(0);

    if (records.isEmpty()) {
        m_table->setRowCount(1);
        auto* emptyItem = new QTableWidgetItem("هیچ بازی‌ای یافت نشد.");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setForeground(QColor("#6c7086"));
        m_table->setItem(0, 0, emptyItem);
        m_table->setSpan(0, 0, 1, 6);
        return;
    }

    m_table->setRowCount(records.size());

    for (int row = 0; row < records.size(); ++row) {
        const GameRecord& r = records[row];

        auto* gameItem = new QTableWidgetItem(r.gameTypeText());
        gameItem->setTextAlignment(Qt::AlignCenter);

        auto* oppItem = new QTableWidgetItem(r.opponentUsername);
        oppItem->setTextAlignment(Qt::AlignCenter);

        auto* roleItem = new QTableWidgetItem(
            r.role == "host" ? "میزبان 🏠" : "مهمان 🚪");
        roleItem->setTextAlignment(Qt::AlignCenter);

        auto* resultItem = new QTableWidgetItem(
            resultEmoji(r.result) + " " + r.resultText());
        resultItem->setTextAlignment(Qt::AlignCenter);
        switch (r.result) {
        case GameResult::WIN:
            resultItem->setForeground(QColor("#a6e3a1")); break;
        case GameResult::LOSE:
            resultItem->setForeground(QColor("#f38ba8")); break;
        case GameResult::DRAW:
            resultItem->setForeground(QColor("#f9e2af")); break;
        }

        auto* scoreItem = new QTableWidgetItem(
            QString("%1 - %2").arg(r.myScore).arg(r.opponentScore));
        scoreItem->setTextAlignment(Qt::AlignCenter);

        auto* dateItem = new QTableWidgetItem(
            r.playedAt.toString("yyyy/MM/dd  hh:mm"));
        dateItem->setTextAlignment(Qt::AlignCenter);
        dateItem->setForeground(QColor("#6c7086"));

        m_table->setItem(row, 0, gameItem);
        m_table->setItem(row, 1, oppItem);
        m_table->setItem(row, 2, roleItem);
        m_table->setItem(row, 3, resultItem);
        m_table->setItem(row, 4, scoreItem);
        m_table->setItem(row, 5, dateItem);

        m_table->setRowHeight(row, 44);
    }
}

QString HistoryWindow::resultEmoji(GameResult result) const
{
    switch (result) {
    case GameResult::WIN:  return "🏆";
    case GameResult::LOSE: return "💔";
    case GameResult::DRAW: return "🤝";
    }
    return "";
}

// ─────────────────────────────────────────────
//  Slots
// ─────────────────────────────────────────────

void HistoryWindow::onFilterChanged(int index)
{
    QString gameType = m_filterCombo->itemData(index).toString();
    loadHistory(gameType);
}

void HistoryWindow::onBackClicked()
{
    emit backToMenu();
}
