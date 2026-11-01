cmake_minimum_required(VERSION 3.16)

project(SimpleTextEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 同时兼容 Qt6 / Qt5（Qt 5.12 及以上）
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(TextEditor
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    codeeditor.cpp
    codeeditor.h
    finddialog.cpp
    finddialog.h
    replacedialog.cpp
    replacedialog.h
    textsearch.cpp
    textsearch.h
)

target_link_libraries(TextEditor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# .ui 里提升(Promote)出来的 CodeEditor 需要能 include "codeeditor.h"
target_include_directories(TextEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
    # 生成窗口程序而非控制台程序
    set_target_properties(TextEditor PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
