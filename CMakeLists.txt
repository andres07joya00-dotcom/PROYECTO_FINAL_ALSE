cmake_minimum_required(VERSION 3.5)

project(PROYECTO_FINAl_ALSE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/component.cpp
    src/DatabaseManager.cpp
    src/InventoryManager.cpp
    include/mainwindow.h
    include/component.h
    include/DatabaseManager.h
    include/mainwindow.h
    ui/mainwindow.ui
)

qt_add_executable(PROYECTO_FINAl_ALSE
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(PROYECTO_FINAl_ALSE PRIVATE
    Qt6::Widgets
    Qt6::Sql
)

qt_finalize_executable(PROYECTO_FINAl_ALSE)

