# gui/CMakeLists.txt

# 1. 查找当前模块的文件
file(GLOB_RECURSE GUI_SOURCES "*.cpp")
file(GLOB_RECURSE GUI_HEADERS "*.h")
file(GLOB GUI_UI_FILES "*.ui")

# 2. 处理 .ui 文件
qt_wrap_ui(GUI_UI_HEADERS ${GUI_UI_FILES})

# 3. 定义 gui 库
add_library(xscanner_gui STATIC
    ${GUI_SOURCES}
    ${GUI_HEADERS}
    ${GUI_UI_HEADERS}
)

# 4. 声明 gui 库的 PUBLIC 依赖项
#    gui 依赖于 service, qcustomplot, 以及它需要的 Qt 模块
target_link_libraries(xscanner_gui PUBLIC
    Qt6::Widgets
    Qt6::Core5Compat
    Qt6::AxContainer #  原始 find_package 中包含
    
    qcustomplot
    xscanner_service
)

target_include_directories(xscanner_gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
