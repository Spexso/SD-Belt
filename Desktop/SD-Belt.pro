QT       += core gui websockets
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CameraReceiver.cpp \
    Logs.cpp \
    SystemInfoRetriever.cpp \
    SystemLogRetriever.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CameraReceiver.h \
    Globals.h \
    Logs.h \
    SystemInfoRetriever.h \
    SystemLogRetriever.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

DISTFILES += \
    styles/verticalWidget.qss

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
