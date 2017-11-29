#-------------------------------------------------
#
# Project created by QtCreator 2015-02-02T17:12:13
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Projet_Config
TEMPLATE = app

SOURCES += main.cpp\
    bdd.cpp \
    configurer.cpp \
    login.cpp

HEADERS  +=  \
    configurer.h \
    bdd.h \
    login.h

FORMS    += configurer.ui \
    login.ui
