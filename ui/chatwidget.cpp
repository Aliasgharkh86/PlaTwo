#include "chatwidget.h"
#include "ui_chatwidget.h"
#include <QScrollBar>

ChatWidget::ChatWidget(const QString& myUsername, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ChatWidget)
    , m_myUsername(myUsername)
{
    ui->setupUi(this);

    // ── connects دکمه‌ی ارسال و Enter ──────────
    connect(ui->sendBtn,   &QPushButton::clicked,
            this, &ChatWidget::onSendClicked);
    connect(ui->chatInput, &QLineEdit::returnPressed,
            this, &ChatWidget::onSendClicked);

    // ── connects استیکرها ───────────────────────
    const QStringList stickers = {
        "😂","👍","🔥","😎","😢","🤝","💪","🎉"
    };
    const QList<QPushButton*> btns = {
        ui->stickerBtn1, ui->stickerBtn2,
        ui->stickerBtn3, ui->stickerBtn4,
        ui->stickerBtn5, ui->stickerBtn6,
        ui->stickerBtn7, ui->stickerBtn8,
    };
    for (int i = 0; i < btns.size(); ++i) {
        const QString s = stickers[i];
        connect(btns[i], &QPushButton::clicked,
                [=]() { sendText(s); });
    }
}

ChatWidget::~ChatWidget()
{
    delete ui;
}

void ChatWidget::setClient(GameClient* client)
{
    if (m_client)
        m_client->disconnect(this);

    m_client = client;

    if (m_client)
        connect(m_client, &GameClient::chatReceived,
                this, &ChatWidget::onChatReceived);
}

void ChatWidget::sendText(const QString& text)
{
    if (!m_client || !m_client->isConnected() || text.isEmpty())
        return;

    m_client->sendChat(text);

    ui->chatDisplay->append(
        QString("<div style='color:#89b4fa;'>"
                "<b>شما:</b> %1</div>").arg(text.toHtmlEscaped()));

    ui->chatDisplay->verticalScrollBar()->setValue(
        ui->chatDisplay->verticalScrollBar()->maximum());
}

void ChatWidget::onSendClicked()
{
    const QString text = ui->chatInput->text().trimmed();
    if (text.isEmpty()) return;
    sendText(text);
    ui->chatInput->clear();
}

void ChatWidget::onChatReceived(const QString& sender, const QString& text)
{
    ui->chatDisplay->append(
        QString("<div style='color:#a6e3a1;'>"
                "<b>%1:</b> %2</div>")
            .arg(sender.toHtmlEscaped(), text.toHtmlEscaped()));

    ui->chatDisplay->verticalScrollBar()->setValue(
        ui->chatDisplay->verticalScrollBar()->maximum());
}
