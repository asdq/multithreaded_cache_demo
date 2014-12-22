#include "main.h"
#include "mainwindow.h"
#include <QApplication>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QtDebug>

Main::Main(QObject *parent) : QObject(parent),
    m_socket(new QTcpSocket(this)), m_state(Disconnected) {
	qApp -> setQuitOnLastWindowClosed(false);
	connect(qApp, &QApplication::lastWindowClosed, this, &Main::stop);
	
	auto *buffer = new QByteArray;
	
	connect(m_socket, &QAbstractSocket::destroyed, [buffer] {
		qDebug() << "delete buffer";
		delete buffer;
	});
	
	connect(m_socket, &QAbstractSocket::readyRead, [this, buffer] {
		qDebug() << "QAbstractSocket::readyRead";
		
		if (m_state == Idle) return;
		
		QByteArray newData = m_socket -> readAll();
		int a, b;
		
		a = 0;
		do {
			b = newData.indexOf(Protocol::terminator(), a);
			buffer -> append(newData.mid(a, b != -1 ? b - a : -1));
			if (b != -1) {
				dispatch(QByteArray::fromBase64(*buffer));
				buffer -> clear();
			}
			a = b  + 1;
		} while (b != -1);
	});
	
	// try to reconnect on connection lost
	auto *timer = new QTimer(this);
	
	timer -> setSingleShot(true);
	timer -> setInterval(3000);
	connect(timer, &QTimer::timeout, this, &Main::start);
	
	connect(m_socket, &QAbstractSocket::stateChanged, [this, timer] {
		qDebug() << "QAbstractSocket::stateChanged" << m_socket -> state();
		if (m_state == Idle) return;
		
		switch (m_socket -> state()) {
		case QAbstractSocket::UnconnectedState:
			setState(Disconnected);
			if ( ! timer -> isActive()) timer -> start();
			return;
			
		case QAbstractSocket::ConnectedState:
			setState(Connected);
			return;
			
		default: return;
		}
	});
}

void Main::start() {
	qDebug() << "Main::start";
	if (m_state != Disconnected) return;
	
	if (m_socket -> state() == QAbstractSocket::UnconnectedState) {
		qDebug() << "m_socket -> connectToHost 127.0.0.1 on port 54321";
		m_socket -> connectToHost("127.0.0.1", 54321);
	}
}

void Main::stop() {
	qDebug() << "Main::stop";
	setState(Idle);
	m_socket -> close();
	qApp -> quit();
}

void Main::dispatch(const QByteArray &packet) {
	qDebug() << "Main::dispatch";
	
	QDataStream in(packet);
	qint64 mask, version;
	
	in.setVersion(QDataStream::Qt_5_2);
	
	// check mask, drop silently if different
	in >> mask;
	if (mask != Protocol::mask()) return;
	
	// wrong protocol
	in >> version;
	if (version != Protocol::version()) {
		setState(Idle);
		
		emit errorMessage(tr("Wrong protocol version: client's version %1, "
		                     "server's version %2")
		                  .arg(Protocol::version(), version));
		
		return m_socket -> disconnectFromHost();
	}
	
	MessageType mt;
	
	in >> mt;
	switch (mt) {
	case LoginResponse: return onLoginResponse(in);
	case TextMessage: return onTextMessage(in);
	case ClientEntered: return onClientEntered(in);
	case ClientExited: return onClientExited(in);
	default: return;
	}
}

void Main::onLoginResponse(QDataStream &in) {
	qDebug() << "Main::onLoginResponse";
	if (m_state != Connected) return;
	QString name;
	
	in >> name;
	if ( ! name.isEmpty()) {
		setNickname(name);
		setState(Logged);
	} else emit errorMessage(tr("NickName %1 is already in use.").arg(name));
}

void Main::onTextMessage(QDataStream &in) {
	qDebug() << "Main::onTextMessage";
	if (m_state != Logged) return;
	
	QString sender, text;
	QDateTime timestamp;
	
	in >> timestamp >> sender >> text;
	
	emit chatMessage(timestamp, sender, text);
}

void Main::onClientEntered(QDataStream &in) {
	qDebug() << "Main::onClientEntered";
	if (m_state != Logged) return;
	
	QVector<QString> clients;
	
	in >> clients;
	std::sort(clients.begin(), clients.end());
	emit newClients(clients);
}

void Main::onClientExited(QDataStream &in) {
	qDebug() << "Main::onClientExited";
	if (m_state != Logged) return;
	
	QVector<QString> clients;
	
	in >> clients;
	emit deletedClients(clients);
}

void Main::sendMessage(QVector<QString> receivers, QString text) {
	qDebug() << "Main::sendMessage" << text;
	if (m_state != Logged) return;
	
	QByteArray buffer;
	QDataStream out(&buffer, QIODevice::WriteOnly);
	
	setHeader(out) << TextMessage << m_nickname << receivers << text;
	sendPacket(buffer);
}

void Main::sendLoginRequest(QString text) {
	qDebug() << "Main::sendLoginRequest" << text;
	if (m_state != Connected) return;
	
	QByteArray buffer;
	QDataStream out(&buffer, QIODevice::WriteOnly);
	
	setHeader(out) << LoginRequest << text;
	sendPacket(buffer);
}

void Main::sendPacket(const QByteArray &packet) {
	qDebug() << "Main::sendPacket"
	         << "to" << m_socket -> peerAddress().toString();
	
	qDebug() << packet.toBase64();
	
	m_socket -> write(packet.toBase64());
	m_socket -> putChar(Protocol::terminator());
	m_socket -> flush();
}

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	Main m;
	MainWindow w(&m);
	
	w.show();
	m.start();
	return a.exec();
}
