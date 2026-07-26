#include "gameserver.h"
#include <QDebug>

GameServer::GameServer(QObject *parent) : QTcpServer(parent), remainingTime(0) {
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameServer::onTimerTick);
}

void GameServer::startServer(quint16 port) {
    if (!this->listen(QHostAddress::Any, port)) {
        qDebug() << "Server failed to start:" << this->errorString();
    } else {
        qDebug() << "Server started on port:" << port;
    }
}

void GameServer::incomingConnection(qintptr socketDescriptor) {
    if (clients.size() >= 2) {
        QTcpSocket tempSocket;
        tempSocket.setSocketDescriptor(socketDescriptor);
        tempSocket.disconnectFromHost();
        return;
    }

    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);

    connect(client, &QTcpSocket::readyRead, this, &GameServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &GameServer::onClientDisconnected);

    clients.append(client);
    qDebug() << "Client connected. Total clients:" << clients.size();

    // وقتی هر دو بازیکن متصل شدند
    if (clients.size() == 2) {
        emit gameReady();
        broadcastMessage("GAME_READY");
        startGameTimer(60);
    }
}

void GameServer::onReadyRead() {
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    QByteArray data = senderSocket->readAll();

    for (QTcpSocket *client : clients) {
        if (client != senderSocket) {
            client->write(data);
            client->flush();
        }
    }
}

void GameServer::onClientDisconnected() {
    QTcpSocket *disconnectedClient = qobject_cast<QTcpSocket*>(sender());
    if (!disconnectedClient) return;

    clients.removeOne(disconnectedClient);
    disconnectedClient->deleteLater();
    qDebug() << "Client disconnected. Total clients:" << clients.size();

    stopGameTimer();
    broadcastMessage("OPPONENT_DISCONNECTED");

    emit gameAborted();
}

void GameServer::broadcastMessage(const QByteArray &message) {
    for (QTcpSocket *client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(message);
            client->flush();
        }
    }
}

void GameServer::startGameTimer(int seconds) {
    remainingTime = seconds;
    gameTimer->start(1000);
    broadcastMessage(QByteArray("TIMER_START:") + QByteArray::number(seconds));
}

void GameServer::stopGameTimer() {
    if (gameTimer->isActive()) {
        gameTimer->stop();
    }
}

void GameServer::onTimerTick() {
    remainingTime--;
    if (remainingTime > 0) {
        broadcastMessage(QByteArray("TIME:") + QByteArray::number(remainingTime));
    } else {
        stopGameTimer();
        broadcastMessage("TIME_OUT");
    }
}
