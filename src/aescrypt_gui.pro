QT += core gui widgets
CONFIG += c++17
TEMPLATE = app
TARGET = aescrypt_gui

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    aescrypt.cpp

HEADERS += \
    ../include/mainwindow.h \
    ../include/aescrypt.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += ../include
LIBS += -L../lib64 -lssl -lcrypto
QMAKE_LFLAGS += -Wl,-rpath,'$$ORIGIN/../lib64'
