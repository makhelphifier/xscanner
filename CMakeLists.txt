cmake_minimum_required(VERSION 3.16)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5（包括Core、Gui、Widgets）
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

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
    util/logger/logger.cpp
    util/logger/qtwidgetappender.cpp
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
    util/logger/logger.h
    util/logger/qtwidgetappender.h
)

# 添加可执行文件目标（启用AUTOMOC和AUTORCC）
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    gui/Img.qrc  # Qt资源文件（AUTORCC会自动处理）
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    gui
    app
    ${CMAKE_CURRENT_SOURCE_DIR}/../sdk/opencv/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../sdk/log4qt/include
)

# 链接外部库（OpenCV和log4qt，使用完整路径）
target_link_libraries(${PROJECT_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/../sdk/opencv/lib/opencv_world470.lib"  # 假设Windows .lib
    "${CMAKE_CURRENT_SOURCE_DIR}/../sdk/log4qt/lib/log4qt.lib"           # Release版本
)

# 处理Debug版本的log4qt库（如果需要不同库文件）
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    target_link_libraries(${PROJECT_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/../sdk/log4qt/lib/log4qt_d.lib"  # Debug版本库（如果存在）
    )
endif()

# 设置运行时输出目录（统一到bin/Debug或bin/Release）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# 复制配置文件和DLL到输出目录（POST_BUILD）
set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/config/log4qt.properties")
set(OPENCV_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../sdk/opencv/bin/opencv_world470.dll")
set(LOG4QT_DLL_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../sdk/log4qt/bin/log4qt.dll")
set(LOG4QT_DLL_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../sdk/log4qt/bin/log4qt_d.dll")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CONFIG_FILE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENCV_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,$<IF:$<BOOL:${LOG4QT_DLL_DEBUG}>,${LOG4QT_DLL_DEBUG},${LOG4QT_DLL_RELEASE}>,${LOG4QT_DLL_RELEASE}>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying config and DLLs to output directory"
)

# 安装规则（可选，类似于.pro中的deployment）
if(UNIX AND NOT ANDROID)
    install(TARGETS ${PROJECT_NAME} DESTINATION /opt/${PROJECT_NAME}/bin)
elseif(WIN32)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
