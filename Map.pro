QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT += axcontainer #读写office文件
QT += sql         #连接数据库

SOURCES += \
    changepassword.cpp \
    floyddemonstrate.cpp \
    login.cpp \
    main.cpp \
    chinamap.cpp \
    showlog.cpp \
    showsuggest.cpp \
    showuser.cpp \
    signin.cpp

HEADERS += \
    Graph.h \
    SetWindowCompositionAttribute.h \
    changepassword.h \
    chinamap.h \
    floyddemonstrate.h \
    login.h \
    queue_Sq.h \
    showlog.h \
    showsuggest.h \
    showuser.h \
    signin.h

FORMS += \
    changepassword.ui \
    chinamap.ui \
    floyddemonstrate.ui \
    login.ui \
    showlog.ui \
    showsuggest.ui \
    showuser.ui \
    signin.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rsc.qrc

RC_ICONS = map.ico
