QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../MESSWidgets/release/ -lMESSWidgets
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../MESSWidgets/debug/ -lMESSWidgets
else:unix: LIBS += -L$$OUT_PWD/../../MESSWidgets/ -lMESSWidgets

INCLUDEPATH += $$PWD/../../MESSWidgets
DEPENDPATH += $$PWD/../../MESSWidgets

INCLUDEPATH += $$PWD/../../../External/QJsonModel/
INCLUDEPATH += $$PWD/../../../External/xpack/
