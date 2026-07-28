#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include "../network/gameclient.h"

namespace Ui { class ChatWidget; }

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(const QString& myUsername, QWidget* parent = nullptr);
    ~ChatWidget();

    void setClient(GameClient* client);

private slots:
    void onSendClicked();
    void onChatReceived(const QString& sender, const QString& text);

private:
    Ui::ChatWidget* ui;
    GameClient*     m_client = nullptr;
    QString         m_myUsername;

    void sendText(const QString& text);
};

#endif // CHATWIDGET_H
