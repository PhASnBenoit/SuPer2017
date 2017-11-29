#-------------------------------------------------
#
# Project created by QtCreator 2014-02-11T09:34:19
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Projet_SuPer
TEMPLATE = app


SOURCES += main.cpp\
    affichealarme.cpp \
    server.cpp \
    reader.cpp \
    thread.cpp \
    tcpsocket.cpp \
    tcpserver.cpp \
    ihm.cpp \
    dynamique.cpp \
    contenuonglet.cpp \
    clientconnection.cpp \
    bdd.cpp

HEADERS  += server.h \
    reader.h \
    thread.h \
    tcpsocket.h \
    tcpserver.h \
    ihm.h \
    dynamique.h \
    contenuonglet.h \
    clientconnection.h \
    bdd.h \
    affichealarme.h \

FORMS    += ihm.ui

OTHER_FILES += \
    ../../../bin/SuPer.ini
