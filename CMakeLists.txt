cmake_minimum_required(VERSION 3.16)

project(CalculatorWithKeyboard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Calculator
    main.cpp
    calculator.cpp
    calculator.h
    calculator.ui
)

target_link_libraries(Calculator PRIVATE Qt6::Widgets)
