#-------------------------------------------------
#
# Project created by QtCreator 2016-04-02T22:08:03
#
#-------------------------------------------------

QT       += core gui

CC = g++

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    ../test_leader_election/broadcast_message.cpp \
    ../test_leader_election/client_chat_ack.cpp \
    ../test_leader_election/client_heartbeat.cpp \
    ../test_leader_election/leader_election.cpp \
    ../test_leader_election/msg_listener.cpp \
    ../test_leader_election/user_listener.cpp

HEADERS  += mainwindow.h \
    ui_mainwindow.h \
    ../test_leader_election/chat_system.h

QMAKE_CXXFLAGS += -std=c++11

FORMS    += mainwindow.ui
