#include "clientlistmodel.h"
#include <algorithm>
#include <QtDebug>

using std::begin; using std::end;
using std::back_inserter;

ClientListModel::ClientListModel(QObject *parent)
    : QAbstractListModel(parent) {}

int ClientListModel::rowCount(const QModelIndex &parent) const {
	return parent.isValid() ? 0 : m_data.size();
}

QVariant ClientListModel::data(const QModelIndex &index, int role) const {
	if (index.column() != 0) return QVariant();
	
	int r = index.row();
	int sz = m_data.size();
	
	switch (role) {
	case Qt::DisplayRole: return (r < sz) ? m_data[r] : QVariant();
	default: return QVariant();
	}
}

bool ClientListModel::contains(const QString &client) const {
	return std::binary_search(begin(m_data), end(m_data), client);
}

void ClientListModel::merge(const QVector<QString> &clients) {
	Q_ASSERT(std::is_sorted(begin(m_data), end(m_data)));
	Q_ASSERT(std::is_sorted(begin(clients), end(clients)));
	qDebug() << "ClientListModel::merge";
	qDebug() << "data" << m_data;
	qDebug() << "clients" << clients;
	
	QVector<QString>::iterator i;
	
	beginResetModel();
	m_data.reserve(m_data.size() +  clients.size());
	i = end(m_data);
	std::copy(begin(clients), end(clients), back_inserter(m_data));
	std::inplace_merge(begin(m_data), i, end(m_data));
	i = std::unique(begin(m_data), end(m_data));
	m_data.erase(i, end(m_data));
	endResetModel();
	
	qDebug() << "data" << m_data;
}

void ClientListModel::remove(const QVector<QString> &clients) {
	Q_ASSERT(std::is_sorted(begin(m_data), end(m_data)));
	Q_ASSERT(std::is_sorted(begin(clients), end(clients)));
	qDebug() << "ClientListModel::remove";
	qDebug() << "data" << m_data;
	qDebug() << "clients" << clients;
	
	QVector<QString>::iterator a, b, e;
	QVector<QString>::const_iterator i = begin(clients);
	QVector<QString>::const_iterator ec = end(clients);
	
	auto equate = [&i, &ec] (const QString &c) -> bool {
		while (i != ec && *i < c) ++i;
		if (i == ec) return false;
		if (*i == c) return ++i, true;
		return false;
	};
	
	beginResetModel();
	e = end(m_data);
	for (a = b = begin(m_data); b != e; ++b) {
		if ( ! equate(*b)) *a++ = *b;
	}
	m_data.erase(a, b);
	endResetModel();
	
	qDebug() << "data" << m_data;
}

void ClientListModel::clear() {
	beginResetModel();
	m_data.clear();
	endResetModel();
}
