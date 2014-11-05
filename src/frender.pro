if(!equals(QT_MAJOR_VERSION, 5)|!equals(QT_MINOR_VERSION, 3)) {
    error("This program can only be compiled with Qt 5.3.x.")
}

QT  += network webkitwidgets
QT -= gui

TEMPLATE = app
TARGET = frender
CONFIG   += console

DESTDIR = ../bin

SOURCES += main.cpp \
    webpage.cpp \
    custompage.cpp

HEADERS += \
    webpage.h
