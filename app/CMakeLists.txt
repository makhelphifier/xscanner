# 1. 查找 app 自己的文件
file(GLOB APP_SOURCES "*.cpp")
file(GLOB APP_HEADERS "*.h")

# 2. 定义可执行文件
#    注意：Img.qrc 在上一层目录，所以我们使用 ../Img.qrc
qt_add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_HEADERS}
    "${CMAKE_CURRENT_SOURCE_DIR}/../Img.qrc"
)

# 3. 链接到 gui 库
#    xscanner_gui 会通过 PUBLIC 依赖传递，自动链接所有其他库
#    (service, device, logger, opencv, qt, qcustomplot 等)
target_link_libraries(${PROJECT_NAME} PRIVATE
    xscanner_gui
)

# 4. 包含 app 目录 (以便 main.cpp 能 #include "app.h")
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 复制配置文件和DLL到输出目录（POST_BUILD）
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config/log4qt.properties")
set(LOG4QT_DLL_RELEASE "${CMAKE_SOURCE_DIR}/third_party/Log4Qt/bin/log4qt.dll")
set(LTDMC_DLL "${CMAKE_SOURCE_DIR}/third_party/device/motor/bin/LTDMC.dll")
# set (MOTION_CTRL_CONFIG "${CMAKE_SOURCE_DIR}/third_party/device/motor/MotionCtrlConfig.ini")
# set (ADAPTOR_INI "${CMAKE_SOURCE_DIR}/third_party/device/motor/adaptor.ini")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CONFIG_FILE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LOG4QT_DLL_RELEASE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/third_party/opencv/bin/opencv_world4120.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LTDMC_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    # COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #         "${MOTION_CTRL_CONFIG}"
    #         "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

    # COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #         "${ADAPTOR_INI}"
    #         "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying config and DLLs to output directory"
)
# 复制整个 Resources 文件夹到可执行文件 (.exe) 旁边
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${RESOURCE_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources"
    COMMENT "Copying Resources folder to output directory"
)
