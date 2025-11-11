# service/CMakeLists.txt

# 1. 查找当前模块的文件
file(GLOB SERVICE_SOURCES "*.cpp")
file(GLOB SERVICE_HEADERS "*.h")

# 2. 定义 service 库
add_library(xscanner_service STATIC
    ${SERVICE_SOURCES}
    ${SERVICE_HEADERS}
)

# 3. 声明 service 库的 PUBLIC 依赖项
#    根据您项目的结构，service 层很可能需要：
#    a. 日志库 (xscanner_logger)
#    b. 设备库 (device_3100A_lib)
#    c. OpenCV (opencv_lib) (如果 service 层处理图像)
#    d. Qt (Core, Concurrent)
target_link_libraries(xscanner_service PUBLIC
    Qt6::Core
    Qt6::Concurrent

    xscanner_logger
    device_3100A_lib
    opencv_world4120
)

target_include_directories(xscanner_service PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/third_party/opencv/include"
)
