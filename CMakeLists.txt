cmake_minimum_required(VERSION 3.16)
project(xscanner)

# 查找Qt6（包括Core、Gui、Widgets 和 SerialPort）
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets )

# 自动设置 CMAKE_AUTOMOC, CMAKE_AUTORCC, CMAKE_AUTOUIC
# 并且会设置 CMAKE_CXX_STANDARD 17 (Qt6 默认)
qt_standard_project_setup()

# 定义源文件
set(SOURCES
    app/main.cpp
    gui/items/ROI.cpp
    gui/items/handle.cpp
    gui/items/rectroi.cpp
    gui/widgets/logwidget.cpp
    service/imageprocessor.cpp
    # GUI - Views
    gui/views/imageviewer.cpp
    gui/views/mainwindow.cpp
    # GUI - Widgets
    gui/widgets/toprightinfowidget.cpp
    # GUI - Items
    gui/items/annotationellipseitem.cpp
    gui/items/annotationhorizontallineitem.cpp
    gui/items/annotationlineitem.cpp
    gui/items/annotationpointitem.cpp
    gui/items/annotationrectitem.cpp
    gui/items/annotationverticallineitem.cpp
    # GUI - States
    gui/states/drawingstate.cpp
    gui/states/drawingstatemachine.cpp
    gui/states/idlestate.cpp
    gui/states/panningstate.cpp
    gui/states/dragginghandlestate.cpp
    # device



)

# 定义头文件
set(HEADERS
    gui/items/ROI.h
    gui/items/handle.h
    gui/items/rectroi.h
    gui/states/genericdrawingstate.h
    gui/widgets/logwidget.h
    service/imageprocessor.h
    # GUI - Views
    gui/views/imageviewer.h
    gui/views/mainwindow.h
    # GUI - Widgets
    gui/widgets/toprightinfowidget.h
    # GUI - Items
    gui/items/annotationellipseitem.h
    gui/items/annotationhorizontallineitem.h
    gui/items/annotationlineitem.h
    gui/items/annotationpointitem.h
    gui/items/annotationrectitem.h
    gui/items/annotationverticallineitem.h
    # GUI - States
    gui/states/drawingstate.h
    gui/states/drawingstatemachine.h
    gui/states/idlestate.h
    gui/states/panningstate.h
    gui/states/dragginghandlestate.h


    # device



)

# 添加可执行文件目标
qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    Img.qrc
    util/logger/logger.h util/logger/logger.cpp
    util/logger/qtwidgetappender.h util/logger/qtwidgetappender.cpp  # Qt资源文件（AUTORCC会自动处理）

)


# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk/opencv/build/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk/Log4Qt/x64-Release/include
)

# 链接库 (Qt, OpenCV, log4qt)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets

    # 外部库
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk/opencv/build/x64/vc16/lib/opencv_world4120.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk/Log4Qt/x64-Release/lib/log4qt.lib"
)

# 设置运行时输出目录（统一到bin/Debug或bin/Release）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# 复制配置文件和DLL到输出目录（POST_BUILD）
set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/config/log4qt.properties")
set(LOG4QT_DLL_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sdk/Log4Qt/x64-Release/bin/log4qt.dll")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CONFIG_FILE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LOG4QT_DLL_RELEASE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying config and DLLs to output directory"
)
# 复制整个 Resources 文件夹到可执行文件 (.exe) 旁边
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${RESOURCE_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources"
    COMMENT "Copying Resources folder to output directory"
)

# 安装规则
if(UNIX AND NOT ANDROID)
    install(TARGETS ${PROJECT_NAME} DESTINATION /opt/${PROJECT_NAME}/bin)
elseif(WIN32)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
