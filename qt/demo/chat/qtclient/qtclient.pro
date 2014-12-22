#-------------------------------------------------
#
# Project created by QtCreator 2014-08-17T23:47:24
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = qtclient
TEMPLATE = app
VERSION = 0.1

SOURCES += \
	main.cpp \
	mainwindow.cpp \
	clientlistmodel.cpp

HEADERS  += \
	mainwindow.h \
	chat_serial.h \
	main.h \
	clientlistmodel.h

FORMS += mainwindow.ui
