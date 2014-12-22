#ifndef CLIENTLISTMODEL_H
#define CLIENTLISTMODEL_H

#include <QAbstractListModel>
#include <QVector>

/*!
 * \brief model for the list of clients.
 * 
 * Container for client's nicknames. Keeps nicknames ordered and unique,
 * they can be added with \ref merge() and removed with \ref remove() .
 */
class ClientListModel : public QAbstractListModel {
	
	Q_OBJECT
	
public:
	
	/*!
	 * \brief constructor.
	 *
	 * Initialize an empty model.
	 */
	explicit
	ClientListModel(QObject *parent = 0);
	
	/*!
	 * \brief rowCount \see QAbstractListModel
	 *
	 * Implementation for the viewer.
	 */
	virtual
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	
	/*!
	 * \brief data \see QAbstractListModel
	 *
	 * Implementation for the viewer.
	 */
	virtual
	QVariant data(const QModelIndex &index, int role) const;
	
	/*!
	 * \brief check if a item is in the model.
	 * \param client client's nickname
	 * \return true if client is in the model, othewise false
	 */
	bool contains(const QString &client) const;
	
	/*!
	 * \brief underlying data.
	 * \return a unmodifiable reference to the data
	 */
	const QVector<QString>& data() const { return m_data; }
	
public slots:
	
	/*!
	 * \brief merge new clients.
	 * \param clients list of clients
	 *
	 * Add new clients, keep ordering, remove duplicated elements.
	 */
	void merge(const QVector<QString> &clients);
	
	/*!
	 * \brief remove clients.
	 * \param clients list of clients
	 */
	void remove(const QVector<QString> &clients);
	
	/*!
	 * \brief empty the model.
	 */
	void clear();
	
private:
	QVector<QString> m_data;
};

#endif // CLIENTLISTMODEL_H
