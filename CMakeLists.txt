cmake_minimum_required(VERSION 3.16)
project(ffmpeg_converter_qt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent Multimedia MultimediaWidgets)

qt_standard_project_setup()

add_executable(ffmpeg_converter_qt
    main.cpp
    MainWindow.cpp
    SettingsDialog.cpp
    converter.cpp
    av1tab.cpp
    x265tab.cpp
    vp9tab.cpp
    x264tab.cpp
    combinetab.cpp
    presets.cpp
    trimtab.cpp
)

target_link_libraries(ffmpeg_converter_qt PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)
