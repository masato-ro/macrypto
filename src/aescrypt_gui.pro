QT += core gui widgets
CONFIG += c++17
TEMPLATE = app
TARGET = aescrypt_gui

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    aescrypt.cpp \
    hashutil.cpp

HEADERS += \
    ../include/mainwindow.h \
    ../include/aescrypt.h \

FORMS += \
    mainwindow.ui

INCLUDEPATH += ../include
INCLUDEPATH += C:/OpenSSL-MSVC/include

LIBS += -LC:/OpenSSL-MSVC/lib -llibssl -llibcrypto

RC_FILE = app.rc
RESOURCES += resources.qrc