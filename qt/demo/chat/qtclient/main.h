#ifndef MAIN_H
#define MAIN_H

#include "chat_serial.h"
#include <QObject>
#include <QVector>
#include <QDateTime>

class QTcpSocket;

/*!
 * \brief Handles communication with a server.
 *
 * Handle connection with server, accept and send packets,
 * implement client side of chat protocol. 
 */
class Main : public QObject {
    
    Q_OBJECT
    Q_PROPERTY(ClientState state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString nickname READ nickname NOTIFY nicknameChanged)
    Q_ENUMS(ClientState)
    
public:
    
    /*!
     * \brief States of client.
     */
    enum ClientState {
        Idle, //!< Dead state.
        Disconnected, //!< Client is disconnected.
        Connected, //!< Client is connected with a server, but is not logged.
        Logged //!< Client is connected with a server and entered the chat.
    };
    
    /*!
     * \brief constructor.
     *
     * Constructor does not start the connection.
     * \see start()
     */
    explicit
    Main(QObject *parent = 0);
    
    /*!
     * \brief state property.
     * \return client's state
     */
    ClientState state() const { return m_state; }
    
    /*!
     * \brief nickname property.
     * \return nickname if logged, otherwise an empty string.
     */
    QString nickname() const { return m_nickname; }
    
signals:
    
    /*!
     * \brief emitted when client's state changed.
     * \param s the new state
     */
    void stateChanged(ClientState s);
    
    /*!
     * \brief emitted when nickname changed.
     * \param name the new name
     */
    void nicknameChanged(QString name);
    
    /*!
     * \brief emitted when an error occoured.
     * \param text error's description
     */
    void errorMessage(QString text);
    
    /*!
     * \brief emitted when client receives a message.
     * \param timestamp date and time of the message
     * \param sender nickname
     * \param text message
     */
    void chatMessage(QDateTime timestamp, QString sender, QString text);
    
    /*!
     * \brief notifies new clients.
     * \param list client nicknames
     */
    void newClients(QVector<QString> list);
    
    /*!
     * \brief notifies clients that left the chat
     * \param list client nicknames
     */
    void deletedClients(QVector<QString> list);
    
public slots:
    
    /*!
     * \brief connect to the server.
     */
    void start();
    
    /*!
     * \brief close the connection and quit the program.
     */
    void stop();
    
    /*!
     * \brief send a message.
     * \param receivers list of clients
     * \param text message
     *
     * If the client itself is not in the list, it wouldn't display
     * the message.
     */
    void sendMessage(QVector<QString> receivers, QString text);
    
    /*!
     * \brief send a request to enter the chat.
     * \param text a nickname
     */
    void sendLoginRequest(QString text);
    
private:
    QTcpSocket *m_socket;
    QString m_nickname;
    ClientState m_state;
    
    void setState(ClientState s) {
        if (m_state == s) return;
        
        m_state = s;
        emit stateChanged(s);
    }
    
    void setNickname(const QString &name) {
        if (m_nickname == name) return;
        
        m_nickname = name;
        emit nicknameChanged(name);
    }
    
    void dispatch(const QByteArray &packet);
    void sendPacket(const QByteArray &packet);
    void onLoginResponse(QDataStream &in);
    void onTextMessage(QDataStream &in);
    void onClientEntered(QDataStream &in);
    void onClientExited(QDataStream &in);
};

#endif // MAIN_H
