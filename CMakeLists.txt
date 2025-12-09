cmake_minimum_required(VERSION 3.5)

project(PROYECTO_FINAl_ALSE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activar automoc, autouic, autorcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

# Include
include_directories(include)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/component.cpp
    src/DatabaseManager.cpp
    src/InventoryManager.cpp
    src/report.cpp

    include/mainwindow.h
    include/component.h
    include/DatabaseManager.h
    include/InventoryManager.h
    include/report.h
    include/delegate.h
    ui/mainwindow.ui
)

qt_add_executable(PROYECTO_FINAl_ALSE
    ${PROJECT_SOURCES}
)

target_link_libraries(PROYECTO_FINAl_ALSE PRIVATE
    Qt6::Widgets
    Qt6::Sql
)

qt_finalize_executable(PROYECTO_FINAl_ALSE)
