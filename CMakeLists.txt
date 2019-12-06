cmake_minimum_required(VERSION 3.15)
project(FinalProject)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /Users/bugenzhao/Qt5.13.0/5.13.0/clang_64/lib/cmake)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Concurrent REQUIRED)

add_executable(FinalProject main.cpp mainwindow.cpp querywidget.cpp dataimportingthread.cpp pathsearchwidget.cpp utilities/base.hpp)
target_link_libraries(FinalProject Qt5::Widgets Qt5::Sql Qt5::Concurrent)