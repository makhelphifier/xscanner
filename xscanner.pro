QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += gui app # 添加 gui 和 app 目录，以便互相查找头文件
INCLUDEPATH += device
INCLUDEPATH += $$PWD/../sdk/opencv/include
INCLUDEPATH += $$PWD/../sdk/log4qt/include
LIBS += -L$$PWD/../sdk/opencv/lib/ -lopencv_world470
LIBS += -L$$PWD/../sdk/log4qt/lib/ -llog4qt
SOURCES += \
    app/main.cpp \
    device/motioncommand.cpp \
    device/motionconfig.cpp \
    device/motioncontroller.cpp \
    device/motionhelper.cpp \
    device/motionmanager.cpp \
    gui/items/ROI.cpp \
    gui/items/handle.cpp \
    gui/items/rectroi.cpp \
    gui/states/dragginghandlestate.cpp \
    gui/states/drawingrectstate.cpp \
    gui/states/idlestate.cpp \
    gui/states/panningstate.cpp \
    gui/widgets/logwidget.cpp \
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
    gui/states/drawingstatemachine.cpp \
    util/logger/logger.cpp \
    util/logger/qtwidgetappender.cpp \
    # device/gongyu.cpp \
	device/gycustomcontrol.cpp \
    device/motionwidget.cpp \
    device/serialcomunicator.cpp \
    device/xraysource.cpp
HEADERS += \
    device/motioncommand.h \
    device/motionconfig.h \
    device/motioncontroller.h \
    device/motiondef.h \
    device/motionhelper.h \
    device/motionmanager.h \
    gui/items/ROI.h \
    gui/items/handle.h \
    gui/items/rectroi.h \
    gui/states/dragginghandlestate.h \
    gui/states/drawingrectstate.h \
    gui/states/genericdrawingstate.h \
    gui/states/idlestate.h \
    gui/states/panningstate.h \
    gui/widgets/logwidget.h \
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
    gui/states/drawingstatemachine.h \
    util/logger/logger.h \
    util/logger/qtwidgetappender.h \
    # device/gongyu.h \
	device/motionwidget.h \
    device/gycustomcontrol.h \
    device/serialcomunicator.h \
    device/xraysource.h
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    gui/Img.qrc


# 1. 确定最终的目标目录
# ---------------------------------
# 默认为 Release 目录
DESTINATION_DIR = $$OUT_PWD/release
# 如果是 Debug 模式，则改为 Debug 目录
CONFIG(debug, debug|release) {
    DESTINATION_DIR = $$OUT_PWD/debug
}

# 2. 定义源文件路径
CONFIG_FILE = $$PWD/config/log4qt.properties
OPENCV_DLL_FILE = $$PWD/../sdk/opencv/bin/opencv_world470.dll
LOG4QT_DLL_FILE = $$PWD/../sdk/log4qt/bin/log4qt.dll
CONFIG(debug, debug|release) {
    LOG4QT_DLL_FILE = $$PWD/../sdk/log4qt/bin/log4qt_d.dll
}

# 3. 定义并执行复制命令
QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$CONFIG_FILE) $$shell_path($$DESTINATION_DIR) $$escape_expand(\\n\\t)
QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$LOG4QT_DLL_FILE) $$shell_path($$DESTINATION_DIR) $$escape_expand(\\n\\t)
QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$OPENCV_DLL_FILE) $$shell_path($$DESTINATION_DIR) $$escape_expand(\\n\\t)
