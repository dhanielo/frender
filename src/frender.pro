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
    filesystem.cpp \
    system.cpp \
    encoding.cpp \
    terminal.cpp \
    env.cpp \
    callback.cpp \
    frender.cpp

HEADERS += \
    webpage.h \
    system.h \
    filesystem.h \
    encoding.h \
    terminal.h \
    env.h \
    callback.h \
    frender.h
