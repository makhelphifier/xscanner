QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += gui app # 添加 gui 和 app 目录，以便互相查找头文件
INCLUDEPATH += $$PWD/../sdk/opencv/include

LIBS += -L$$PWD/../sdk/opencv/lib/ -lopencv_world470

SOURCES += \
    app/main.cpp \
    gui/annotationellipseitem.cpp \
    gui/annotationlineitem.cpp \
    gui/annotationpointitem.cpp \
    gui/annotationrectitem.cpp \
    service/imageprocessor.cpp \
    gui/imageviewer.cpp \
    gui/mainwindow.cpp \
    gui/toprightinfowidget.cpp

HEADERS += \
    gui/annotationellipseitem.h \
    gui/annotationlineitem.h \
    gui/annotationpointitem.h \
    gui/annotationrectitem.h \
    service/imageprocessor.h \
    gui/imageviewer.h \
    gui/mainwindow.h \
    gui/toprightinfowidget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    gui/Img.qrc
