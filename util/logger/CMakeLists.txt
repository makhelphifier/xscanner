# util/logger/CMakeLists.txt

# 1. 查找当前模块的源文件
file(GLOB LOGGER_SOURCES "*.cpp")
file(GLOB LOGGER_HEADERS "*.h")

# 2. 定义 logger 库
add_library(xscanner_logger STATIC
    ${LOGGER_SOURCES}
    ${LOGGER_HEADERS}
)

# 3. 声明 logger 库的 PUBLIC 依赖项
#    任何链接 xscanner_logger 的目标 (比如您未来的 service 库或 gui 库)
#    都将自动：
#    a. 链接到 log4qt_lib 和 Qt6::Core
#    b. 包含当前目录 (为了能 #include "logger.h")
target_link_libraries(xscanner_logger PUBLIC
    log4qt
    Qt6::Core
)
target_include_directories(xscanner_logger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/third_party/Log4Qt/include"
)
