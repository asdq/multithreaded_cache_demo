#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ClientListModel;
class Main;

namespace Ui {
    class MainWindow;
}

/*!
 * \brief Handles the interface.
 *
 * Accept input from the user, show logged clients, display messages
 * from clients.
 */
class MainWindow : public QMainWindow {
    
    Q_OBJECT
    
public:
    
    /*!
     * \brief MainWindow constructor.
     * \param m controller
     * \param parent
     *
     * Connects with the controller.
     * Id does not take the ownership of m.
     */
    explicit
    MainWindow(Main *m, QWidget *parent = 0);
    
    ~MainWindow();
    
signals:
    
    /*!
     * \brief nickname inserted
     * \param n name
     *
     * Emitted when a nickname was inserted in the login window.
     */
    void nickname(QString n);
    
    /*!
     * \brief message inserted
     * \param m
     *
     * Emitted when a message is inserted in the chat window.
     */
    void message(QVector<QString> receivers, QString text);
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
