#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QTimer>

class GameServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit GameServer(QObject *parent = nullptr);
    void startServer(quint16 port);
    void startGameTimer(int seconds);
    void stopGameTimer();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void gameReady();
    void gameAborted();

private slots:
    void onReadyRead();
    void onClientDisconnected();
    void onTimerTick();

private:
    QList<QTcpSocket*> clients;
    QTimer *gameTimer;
    int remainingTime;

    void broadcastMessage(const QByteArray &message);
};

#endif
