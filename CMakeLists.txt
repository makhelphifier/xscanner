cmake_minimum_required(VERSION 3.16)
project(xscanner)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort )

qt_standard_project_setup()

# 定义源文件
set(SOURCES
    app/main.cpp
    gui/items/ROI.cpp
    gui/items/handle.cpp
    gui/items/rectroi.cpp
    gui/widgets/logwidget.cpp
    gui/widgets/extractedimageviewer.cpp
    service/imageprocessor.cpp
    util/logger/logger.cpp
    util/logger/qtwidgetappender.cpp
    # GUI - Views
    gui/views/imageviewer.cpp
    gui/views/mainwindow.cpp
    # GUI - Widgets
    gui/widgets/toprightinfowidget.cpp
    # GUI - Items
    gui/items/annotationpointitem.cpp
    gui/items/linesegmentroi.cpp
    # GUI - States
    gui/states/drawingstate.cpp
    gui/states/drawingstatemachine.cpp
    gui/states/idlestate.cpp
    gui/states/panningstate.cpp
    gui/states/dragginghandlestate.cpp
    # device
    device/gycustomcontrol.cpp
    device/motionwidget.cpp
    device/serialcomunicator.cpp
    device/xraysource.cpp
    device/motioncommand.cpp
    device/motionconfig.cpp
    device/motioncontroller.cpp
    device/motionhelper.cpp
    device/motionmanager.cpp
)

# 定义头文件
set(HEADERS
    gui/items/ROI.h
    gui/items/handle.h
    gui/items/rectroi.h
    gui/states/genericdrawingstate.h
    gui/widgets/logwidget.h
    service/imageprocessor.h
    util/logger/logger.h
    util/logger/qtwidgetappender.h
    # GUI - Views
    gui/views/imageviewer.h
    gui/views/mainwindow.h
    # GUI - Widgets
    gui/widgets/toprightinfowidget.h
    gui/widgets/extractedimageviewer.h
    # GUI - Items
    gui/items/annotationpointitem.h
    gui/items/linesegmentroi.h
    # GUI - States
    gui/states/drawingstate.h
    gui/states/drawingstatemachine.h
    gui/states/idlestate.h
    gui/states/panningstate.h
    gui/states/dragginghandlestate.h
    # device
    device/motionwidget.h
    device/gycustomcontrol.h
    device/serialcomunicator.h
    device/xraysource.h
    device/motioncommand.h
    device/motionconfig.h
    device/motioncontroller.h
    device/motiondef.h
    device/motionhelper.h
    device/motionmanager.h
)



if(MSVC)
    # MSVC: /utf-8 选项同时设置源字符集和执行字符集
    add_compile_options(/utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang: 需要分别设置源字符集和执行字符集
    add_compile_options(-finput-charset=UTF-8)
    add_compile_options(-fexec-charset=UTF-8)
endif()


# 添加可执行文件目标
qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    Img.qrc
    gui/items/infinitelineitem.h gui/items/infinitelineitem.cpp
    gui/items/infinitelinelabel.h gui/items/infinitelinelabel.cpp
    gui/items/angledlineroi.h gui/items/angledlineroi.cpp
    gui/items/angleroi.h gui/items/angleroi.cpp
    gui/states/angledrawingstate.h gui/states/angledrawingstate.cpp
    gui/items/pointmeasureitem.h gui/items/pointmeasureitem.cpp
)


# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Log4Qt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/device/motor/include

)

# 链接库 (Qt, OpenCV, log4qt)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort

    # 外部库
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv/lib/opencv_world4120.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Log4Qt/lib/log4qt.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/device/motor/lib/LTDMC.lib"

)

# 设置运行时输出目录（统一到bin/Debug或bin/Release）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)


# 复制配置文件和DLL到输出目录（POST_BUILD）
set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/config/log4qt.properties")
set(LOG4QT_DLL_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Log4Qt/bin/log4qt.dll")
set(LTDMC_DLL "${CMAKE_CURRENT_SOURCE_DIR}/third_party/device/motor/bin/LTDMC.dll")
set (MOTION_CTRL_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/third_party/device/motor/MotionCtrlConfig.ini")
set (ADAPTOR_INI "${CMAKE_CURRENT_SOURCE_DIR}/third_party/device/motor/adaptor.ini")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CONFIG_FILE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LOG4QT_DLL_RELEASE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LTDMC_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MOTION_CTRL_CONFIG}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ADAPTOR_INI}"
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
