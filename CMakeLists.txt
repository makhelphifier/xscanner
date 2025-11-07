cmake_minimum_required(VERSION 3.16)
project(xscanner)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort PrintSupport Core5Compat Concurrent)

qt_standard_project_setup()

file(GLOB_RECURSE ALL_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "app/*.cpp"
    "gui/*.cpp"
    "service/*.cpp"
    "util/logger/*.cpp"
    "device_3100A/*.cpp"
)
list(APPEND ALL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/QCustomPlot/include/qcustomplot.cpp"
)
file(GLOB_RECURSE ALL_HEADERS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "app/*.h"
    "gui/*.h"
    "service/*.h"
    "util/logger/*.h"
    "device_3100A/*.h"
)
file(GLOB_RECURSE PROJECT_UI_FILES CONFIGURE_DEPENDS
    "gui/*.ui"
    "device_3100A/*.ui"
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
    ${ALL_SOURCES}
    ${ALL_HEADERS}
    ${PROJECT_UI_FILES}
    Img.qrc
)

# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Log4Qt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/QCustomPlot/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/device/motor/include

)

# 链接库 (Qt, OpenCV, log4qt)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::PrintSupport
    Qt6::Core5Compat
    Qt6::Concurrent
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
