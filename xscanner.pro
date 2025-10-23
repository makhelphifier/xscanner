QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += gui app # 添加 gui 和 app 目录，以便互相查找头文件
INCLUDEPATH += $$PWD/../sdk/opencv/include

LIBS += -L$$PWD/../sdk/opencv/lib/ -lopencv_world470

SOURCES += \
    app/main.cpp \
    service/imageprocessor.cpp \
    # GUI - Views
    gui/views/imageviewer.cpp \
    gui/views/mainwindow.cpp \
    # GUI - Widgets
    gui/widgets/toprightinfowidget.cpp \
    # GUI - Items
    gui/items/annotationellipseitem.cpp \
    gui/items/annotationhorizontallineitem.cpp \
    gui/items/annotationlineitem.cpp \
    gui/items/annotationpointitem.cpp \
    gui/items/annotationrectitem.cpp \
    gui/items/annotationverticallineitem.cpp \
    # GUI - States
    gui/states/drawingstate.cpp \
    gui/states/rectdrawingstate.cpp \
    gui/states/drawingstatemachine.cpp

HEADERS += \
    service/imageprocessor.h \
    # GUI - Views
    gui/views/imageviewer.h \
    gui/views/mainwindow.h \
    # GUI - Widgets
    gui/widgets/toprightinfowidget.h \
    # GUI - Items
    gui/items/annotationellipseitem.h \
    gui/items/annotationhorizontallineitem.h \
    gui/items/annotationlineitem.h \
    gui/items/annotationpointitem.h \
    gui/items/annotationrectitem.h \
    gui/items/annotationverticallineitem.h \
    # GUI - States
    gui/states/drawingstate.h \
    gui/states/rectdrawingstate.h \
    gui/states/drawingstatemachine.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    gui/Img.qrc
