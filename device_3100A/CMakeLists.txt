# device_3100A/CMakeLists.txt

# 1. 查找当前模块的文件 (全部使用 GLOB_RECURSE)
file(GLOB_RECURSE DEVICE_SOURCES "*.cpp")
file(GLOB_RECURSE DEVICE_HEADERS "*.h")
file(GLOB_RECURSE DEVICE_UI_FILES "*.ui")

# 2. 处理 .ui 文件 (需要 Qt)
qt_wrap_ui(DEVICE_UI_HEADERS ${DEVICE_UI_FILES})

# 3. 定义 device_3100A 库
add_library(device_3100A_lib STATIC
    ${DEVICE_SOURCES}
    ${DEVICE_HEADERS}
    ${DEVICE_UI_HEADERS} # 添加由 .ui 生成的头文件
)

# 4. 声明 device_3100A_lib 库的 PUBLIC 依赖项
#    任何链接它的目标，都将自动：
#    a. 链接到 Qt (Widgets, SerialPort)、马达库(ltdmc_lib) 和 日志库(xscanner_logger)
#    b. 包含当前目录 (为了 #include "xxx.h")
target_link_libraries(device_3100A_lib PUBLIC
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::AxContainer
    Qt6::Core5Compat
    Qt6::Concurrent
    LTDMC
    xscanner_logger
)

target_include_directories(device_3100A_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
