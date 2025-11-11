# third_party/QCustomPlot/CMakeLists.txt

# 将 qcustomplot 定义为一个库（我们使用 STATIC，但 INTERFACE 也可以）
add_library(qcustomplot STATIC
    "include/qcustomplot.cpp"
    "include/qcustomplot.h"
)

# 声明这个库的 PUBLIC 属性：
# 1. 它对外暴露的 include 目录
# 2. 它自己需要链接的 Qt 模块
target_include_directories(qcustomplot PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(qcustomplot PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
)