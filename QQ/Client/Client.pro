QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    Login.cpp \
    chat.cpp \
    main.cpp \
    register.cpp

HEADERS += \
    Login.h \
    chat.h \
    register.h \
    userinfo.h

FORMS += \
    Login.ui \
    chat.ui \
    register.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# 添加CSS文件到资源系统
RESOURCES += \
    styles.qrc
