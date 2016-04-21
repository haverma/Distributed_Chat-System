#-------------------------------------------------
#
# Project created by QtCreator 2016-04-02T22:08:03
#
#-------------------------------------------------

QT       += core gui

CC = g++

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dchat
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    broadcast_message.cpp \
    client_chat_ack.cpp \
    heartbeat.cpp \
    leader_election.cpp \
    msg_listener.cpp \
    user_listener.cpp

HEADERS  += mainwindow.h \
    ui_mainwindow.h \
    globals.h \
    chat_system.h

QMAKE_CXXFLAGS += -std=c++11

FORMS    += mainwindow.ui
