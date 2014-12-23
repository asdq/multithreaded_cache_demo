#ifndef MAIN_H
#define MAIN_H

#include "chat_serial.h"
#include <QObject>
#include <QHash>

class QTcpServer;
class QAbstractSocket;

/*!
 * \brief Handles communication with clients.
 *
 * Handle connection with clients, dispatch messages,
 * implement server side of chat protocol. 
 */
class Main : public QObject {
    
    Q_OBJECT
    
    struct SocketHandler {
        QAbstractSocket *socket;
        QByteArray buffer;
        QString nickname;
    };
    
public:
    
    /*!
     * \brief constructor.
     *
     * It doesn't start listening.
     * \see start()
     */
    explicit
    Main(QObject *parent = 0);
    
signals:
    
public slots:
    
    /*!
     * \brief start listening for nev connections.
     */
    void start();
    
private:
    void dispatch(SocketHandler *h, QByteArray packet);
    void sendPacket(SocketHandler *h, const QByteArray &packet);
    void sendPacket(const QVector<QString> &list, const QByteArray &packet);
    void onLoginRequest(SocketHandler *h, QDataStream &in);
    void onTextMessage(SocketHandler *h, QDataStream &in);
    void addClient(SocketHandler *h);
    void removeClient(SocketHandler *h);
    QVector<QString> connectedClients();
    
    QTcpServer *m_server;
    QHash<QString, SocketHandler*> m_map;
};

#endif // MAIN_H
