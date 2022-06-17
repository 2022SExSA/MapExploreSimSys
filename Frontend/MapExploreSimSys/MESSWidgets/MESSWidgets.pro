QT += network
#QT += charts
QT += widgets websockets

TEMPLATE = lib
DEFINES += MESSWIDGETS_LIBRARY

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BaseDisplayWidget.cpp \
    #StatisticsWidget.cpp \
    ExperimentConfigWidget.cpp \
    ExperimentDemandWidget.cpp \
    MESSDisplayWidget.cpp \
    MESSSimulationWidget.cpp \
    ObjectEditWidget.cpp \
    ObjectEntryEditWidget.cpp \
    $$PWD/../../External/QJsonModel/qjsonmodel.cpp \
    utils.cpp

HEADERS += \
    BaseDisplayWidget.h \
    ExperimentConfigWidget.h \
    ExperimentDemandWidget.h \
    JsonModel.h \
    MESSDisplayWidget.h \
    #StatisticsWidget.h \
    MESSSimulationWidget.h \
    ObjectEditWidget.h \
    ObjectEntryEditWidget.h \
    $$PWD/../../External/QJsonModel/qjsonmodel.h \
    RenderOrder.h \
    config.h \
    dll_export.h \
    utils.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    ExperimentDemandWidget.ui \
    StatisticsWidget.ui \
    ExperimentConfigWidget.ui \
    MESSSimulationWidget.ui

INCLUDEPATH += $$PWD/../../External/xpack/
INCLUDEPATH += $$PWD/../../External/QJsonModel/
