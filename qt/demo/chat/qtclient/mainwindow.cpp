#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientlistmodel.h"
#include "main.h"
#include <QListView>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QMessageBox>
#include <QTextEdit>
#include <QStringBuilder>
#include <QStatusBar>

MainWindow::MainWindow(Main *m, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui -> setupUi(this);
    
    auto popup = new QMessageBox(this);
    auto listModel = new ClientListModel(this);
    
    ui -> clientList -> setModel(listModel);
    ui -> statusBar -> showMessage(tr("Disconnected"));
    ui -> nickEdit -> setEnabled(false);
    connect(this, &MainWindow::nickname, m, &Main::sendLoginRequest);
    connect(this, &MainWindow::message, m, &Main::sendMessage);
    connect(m, &Main::nicknameChanged, ui -> inputLabel, &QLabel::setText);
    connect(m, &Main::newClients, listModel, &ClientListModel::merge);
    connect(m, &Main::deletedClients, listModel, &ClientListModel::remove);
    
    connect(m, &Main::stateChanged, [this, listModel] (Main::ClientState s) {
        switch (s) {
        case Main::Idle:
            ui -> statusBar -> showMessage(tr("Idle"));
            ui -> nickEdit -> setEnabled(false);
            return;
            
        case Main::Disconnected:
            ui -> statusBar -> showMessage(tr("Disconnected"));
            ui -> nickEdit -> setEnabled(false);
            listModel -> clear();
            return;
            
        case Main::Connected:
            ui -> statusBar -> showMessage(tr("Connected"));
            ui -> nickEdit -> setEnabled(true);
            ui -> stackedWidget -> setCurrentWidget(ui -> loginPanel);
            return;
            
        case Main::Logged:
            ui -> statusBar -> showMessage(tr("Logged"));
            ui -> nickEdit -> setEnabled(false);
            ui -> stackedWidget -> setCurrentWidget(ui -> chatPanel);
            ui -> lineEdit -> setFocus();
            return;
            
        default: return;
        }
    });
    
    connect(m, &Main::errorMessage, [this, popup] (const QString &text){
        popup -> setText(text);
        popup -> setVisible(true);
    });
    
    connect(m, &Main::chatMessage, [this, listModel] (
            const QDateTime &timestamp, const QString &source,
            const QString &text) {
        if (listModel -> contains(source)) {
            ui -> chatText -> append(timestamp.toString("yyyy-MM-dd hh:mm") %
                                     " " % source % ": " % text);
        }
    });
    
    connect(ui -> nickEdit, &QLineEdit::returnPressed, [this] {
        emit nickname(ui -> nickEdit -> text());
    });
    
    connect(ui -> lineEdit, &QLineEdit::returnPressed, [this, listModel] {
        emit message(listModel -> data(), ui -> lineEdit -> text());
        ui -> lineEdit -> clear();
    });
}

MainWindow::~MainWindow() {
    delete ui;
}
