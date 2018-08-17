SOURCES += $$PWD/framelesswidget.cpp  

HEADERS += $$PWD/framelesswidget.h 

win32 {
QT += winextras
LIBS += -luser32 -ldwmapi
}

INCLUDEPATH += $$PWD

