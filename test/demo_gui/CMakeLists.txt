# test/demo_gui_test/CMakeLists.txt

# 1. 定义可执行文件
# (假设您在 demo_gui_test 目录下创建了一个 demo_main.cpp)
qt_add_executable(demo_gui_test
    demo_main.cpp
)

# 2. 链接到您想测试的库 (例如 xscanner_gui)
# 这是最关键的一步：
# 链接 xscanner_gui 会自动拉取它所有的 PUBLIC 依赖
# (包括 Qt6::Widgets, qcustomplot, xscanner_service 等)
#
target_link_libraries(demo_gui_test PRIVATE
    xscanner_gui
)

# 3. 包含 app 目录 (如果需要，比如 #include "app.h")
target_include_directories(demo_gui_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 4. [重要!] 复制 Demo 运行所需的 DLL 和资源
#    (这部分与 app/CMakeLists.txt 几乎一样，
#     只是 TARGET 必须改成 demo_gui_test)
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config/log4qt.properties")
set(LOG4QT_DLL_RELEASE "${CMAKE_SOURCE_DIR}/third_party/Log4Qt/bin/log4qt.dll")
set(LTDMC_DLL "${CMAKE_SOURCE_DIR}/third_party/device/motor/bin/LTDMC.dll")

add_custom_command(TARGET demo_gui_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CONFIG_FILE}"
            "$<TARGET_FILE_DIR:demo_gui_test>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LOG4QT_DLL_RELEASE}"
            "$<TARGET_FILE_DIR:demo_gui_test>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/third_party/opencv/bin/opencv_world4120.dll"
            "$<TARGET_FILE_DIR:demo_gui_test>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LTDMC_DLL}"
            "$<TARGET_FILE_DIR:demo_gui_test>"
    COMMENT "Copying config and DLLs to demo output directory"
)
# 复制 Resources 文件夹
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources")
add_custom_command(TARGET demo_gui_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${RESOURCE_DIR}"
            "$<TARGET_FILE_DIR:demo_gui_test>/Resources"
    COMMENT "Copying Resources folder to demo output directory"
)