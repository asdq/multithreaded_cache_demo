#include "main.h"
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDateTime>
#include <QtDebug>

Main::Main(QObject *parent) : QObject(parent) {
    m_server = new QTcpServer(this);
    
    connect(m_server, &QTcpServer::newConnection, [this] {
    	while (m_server -> hasPendingConnections()) {
    		auto h = new SocketHandler;
    		
    		h -> socket = m_server -> nextPendingConnection();
    		
    		connect(h -> socket, &QAbstractSocket::disconnected, [this, h] {
    			qDebug() << "QAbstractSocket::disconnected"
    			         << h -> nickname
    			         << "address"
    			         << h -> socket -> peerAddress().toString();
    			
    			h -> socket -> close();
    			h -> socket -> deleteLater();
    			removeClient(h);
    		});
    		
    		connect(h -> socket, &QAbstractSocket::destroyed, [h] {
    			qDebug() << "QAbstractSocket::destroyed delete handler";
    			delete h;
    		});
    		
    		connect(h -> socket, &QAbstractSocket::readyRead, [this, h] {
    			qDebug() << "QAbstractSocket::readyRead"
    			         << h -> nickname
    			         << "address"
    			         << h -> socket -> peerAddress().toString();
    			
    			QByteArray data = h -> socket -> readAll();
    			int a, b;
    			
    			a = 0;
    			do {
    				b = data.indexOf(Protocol::terminator(), a);
    				h -> buffer.append(data.mid(a, b != -1 ? b - a : -1));
    				if (b != -1) {
    					dispatch(h, QByteArray::fromBase64(h -> buffer));
    					h -> buffer.clear();
    				}
    				a = b  + 1;
    			} while (b != -1);
    		});
    	}
    });
}

void Main::start() {
    m_server -> listen(QHostAddress::Any, 54321);
    qDebug("Server is listening at port 54321");
}

void Main::dispatch(SocketHandler *h, QByteArray packet) {
    qDebug() << "Main::dispatch"
             << "from" << h -> nickname
             << "address" << h -> socket -> peerAddress().toString()
             << packet.toBase64();
    
    QDataStream in(packet);
    qint64 mask, version;
    
    in.setVersion(QDataStream::Qt_5_2);
    
    // check mask, drop silently if different
    in >> mask;
    if (mask != Protocol::mask()) {
    	qDebug() << mask << Protocol::mask();
    	return;
    }
    
    // check version, if it is different, send an empty message
    // this should cause the client to disconnect
    in >> version;
    if (version != Protocol::version()) {
    	QByteArray buffer;
    	QDataStream out(&buffer, QIODevice::WriteOnly);
    	
    	setHeader(out) << EmptyMessage;
    	return sendPacket(h, buffer);
    }
    
    MessageType mt;
    
    in >> mt;
    switch (mt) {
    case LoginRequest: return onLoginRequest(h, in);
    case TextMessage: return onTextMessage(h, in);
    default: return;
    }
}

void Main::onLoginRequest(SocketHandler *h, QDataStream &in) {
    qDebug() << "Main::onLoginRequest"
             << "from" << h -> nickname
             << "address" << h -> socket -> peerAddress().toString();
    
    auto sendResponse = [this, h] (const QString &name) {
    	qDebug() << "sendResponse" << name
    	         << "to" << h -> nickname
    	         << "address" << h -> socket -> peerAddress().toString();
    	
    	QByteArray buffer;
    	QDataStream out(&buffer, QIODevice::WriteOnly);
    	
    	setHeader(out) << LoginResponse << name;
    	sendPacket(h, buffer);
    };
    
    QString name;
    
    in >> name;
    
    SocketHandler *x = m_map.value(name);
    
    if (x == nullptr) {
    	h -> nickname = name;
    	sendResponse(h -> nickname);
    	addClient(h);
    	
    // a client already holds the nickname
    } else if (x -> socket -> socketDescriptor() !=
               h -> socket -> socketDescriptor()) {
    	sendResponse(QString());
    }
}

void Main::onTextMessage(SocketHandler *h, QDataStream &in) {
    qDebug() << "Main::onTextMessage"
             << "from" << h -> nickname
             << "address" << h -> socket -> peerAddress().toString();
    
    if (m_map.value(h -> nickname) == nullptr) return; // not logged
    Q_ASSERT(m_map.value(h -> nickname) == h);
    
    QString sender, text;
    QVector<QString> receivers;
    
    in >> sender >> receivers >> text;
    
    if (sender != h -> nickname) {
    	qDebug() << "in text message" << text
    	         << "source" << sender
    	         << "differ from sender"
    	         << h -> nickname
    	         << h -> socket -> peerAddress().toString();
    	h -> socket -> close();
    	return;
    }
    
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    
    setHeader(out) << TextMessage << QDateTime::currentDateTime()
                   << sender << text;
    sendPacket(receivers, buffer);
}

void Main::addClient(SocketHandler *h) {
    qDebug() << "Main::addClient"
             << "client" << h -> nickname
             << "address" << h -> socket -> peerAddress().toString();
    Q_ASSERT(m_map.value(h -> nickname) == nullptr);
    
    m_map[h -> nickname] = h;
    
    // send to the client information of connected clients
    {
    	auto list = connectedClients();
    	
    	qDebug() << "send client list" << list
    	         << "to" << h -> nickname
    	         << "address" << h -> socket -> peerAddress().toString();
    	
    	QByteArray buffer;
    	QDataStream out(&buffer, QIODevice::WriteOnly);
    	
    	setHeader(out) << ClientEntered << list;
    	sendPacket(h, buffer);
    }
    
    // send to other clients information of the client
    {
    	qDebug() << "send client list" << h -> nickname
    	         << "address" << h -> socket -> peerAddress().toString()
    	         << "to" << connectedClients();
    	
    	QByteArray buffer;
    	QDataStream out(&buffer, QIODevice::WriteOnly);
    	QVector<QString> list;
    	
    	list.push_back(h -> nickname);
    	setHeader(out) << ClientEntered << list;
    	sendPacket(connectedClients(), buffer);
    }
}

void Main::removeClient(SocketHandler *h) {
    qDebug() << "Main::removeClient"
             << "client" << h -> nickname
             << "address" << h -> socket -> peerAddress().toString();
    if (m_map.value(h -> nickname) == nullptr) return; // not logged
    
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    QVector<QString> list;
    
    m_map[h -> nickname] = nullptr;
    list.push_back(h -> nickname);
    setHeader(out) << ClientExited << list;
    sendPacket(connectedClients(), buffer);
}

void Main::sendPacket(SocketHandler *h, const QByteArray &packet) {
    qDebug() << "Main::sendPacket"
             << "to" << h -> nickname
             << "address" << h -> socket -> peerAddress().toString();
    
    h -> socket -> write(packet.toBase64());
    h -> socket -> putChar(Protocol::terminator());
    h -> socket -> flush();
}

void Main::sendPacket(const QVector<QString> &list, const QByteArray &packet) {
    qDebug() << "Main::sendPacket"
             << "to" << list;
    
    for (auto name : list) {
    	SocketHandler *h = m_map.value(name);
    	
    	if (h == nullptr) continue;
    	h -> socket -> write(packet.toBase64());
    	h -> socket -> putChar(Protocol::terminator());
    	h -> socket -> flush();
    }
}

QVector<QString> Main::connectedClients() {
    QVector<QString> list;
    
    auto e =  m_map.constEnd();
    for (auto i = m_map.constBegin(); i != e; ++i) {
    	if (i.value() == nullptr) continue;
    	list.push_back(i.key());
    }
    return list;
}

int main(int argc, char **argv) {
    registerSerialMetatypes();
    
    QCoreApplication app(argc, argv);
    Main m;
    
    m.start();
    return app.exec();
}
