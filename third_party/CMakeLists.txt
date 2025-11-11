# OpenCV (目标名改为 opencv_world4120)
add_library(opencv_world4120 STATIC IMPORTED GLOBAL)
set_target_properties(opencv_world4120 PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/opencv/lib/opencv_world4120.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/opencv/include"
)

# Log4Qt (目标名改为 log4qt)
add_library(log4qt SHARED IMPORTED GLOBAL)
set_target_properties(log4qt PROPERTIES
    # [修复] IMPORTED_LOCATION 应指向 .dll (位于 bin 目录)
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/Log4Qt/bin/log4qt.dll"
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/Log4Qt/lib/log4qt.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/Log4Qt/include"
)

# Motor Lib (LTDMC) (目标名改为 LTDMC)
add_library(LTDMC SHARED IMPORTED GLOBAL)
set_target_properties(LTDMC PROPERTIES
    # [修复] IMPORTED_LOCATION 应指向 .dll (位于 bin 目录)
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/device/motor/bin/LTDMC.dll"
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/device/motor/lib/LTDMC.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/device/motor/include"
)

add_subdirectory(QCustomPlot)
