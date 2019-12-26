cmake_minimum_required(VERSION 3.15)
project(FinalProject)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /Users/bugenzhao/Qt5.13.2/5.13.2/clang_64/lib/cmake)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Charts REQUIRED)


add_executable(FinalProject main.cpp mainwindow.cpp
        querywidget.cpp
        pathsearchwidget.cpp
        utilities/base.hpp
        flowplotwidget.cpp
        utilities/BDateTime.cpp
        preferencesdialog.cpp
        utilities/hint.hpp
        utilities/bdatabasemanager.cpp
        baseplotwidget.cpp
        totalflowplotwidget.cpp
        withlineflowplotwidget.cpp
        stationflowplotwidget.cpp
        previewwidget.cpp
        )

target_link_libraries(FinalProject Qt5::Widgets Qt5::Sql Qt5::Concurrent Qt5::Charts)