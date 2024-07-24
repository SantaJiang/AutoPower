QT       += core gui

win32:{
QT       += winextras
LIBS     += -lUser32 -lPowrProf
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    powerop.cpp \
    schedule/customdateedit.cpp \
    schedule/lunarcalendarinfo.cpp \
    schedule/minicalendaritem.cpp \
    schedule/minicalendarwidget.cpp \
    trayicon.cpp \
    worker.cpp

HEADERS += \
    mainwindow.h \
    powerop.h \
    schedule/customdateedit.h \
    schedule/lunarcalendarinfo.h \
    schedule/minicalendaritem.h \
    schedule/minicalendarwidget.h \
    trayicon.h \
    worker.h

FORMS += \
    mainwindow.ui

RC_ICONS = logo.ico

TRANSLATIONS += AutoPower_zh_CN.ts

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src.qrc
