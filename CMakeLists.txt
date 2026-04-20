cmake_minimum_required(VERSION 3.20)
project(MusicCode VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    Multimedia
    Svg
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/audio_engine.h
    src/core/audio_engine.cpp
    src/core/player.h
    src/core/player.cpp
    src/compression/lossless_compressor.h
    src/compression/lossless_compressor.cpp
    src/transition/livesort_algorithm.h
    src/transition/livesort_algorithm.cpp
    src/transition/audio_analyzer.h
    src/transition/audio_analyzer.cpp
    src/models/song.h
    src/models/song.cpp
    src/models/playlist.h
    src/models/playlist.cpp
    src/services/file_scanner.h
    src/services/file_scanner.cpp
    ui/main_window.h
    ui/main_window.cpp
    ui/sidebar/sidebar.h
    ui/sidebar/sidebar.cpp
    ui/content/content_area.h
    ui/content/content_area.cpp
    ui/panel/right_panel.h
    ui/panel/right_panel.cpp
    ui/statusbar/status_bar.h
    ui/statusbar/status_bar.cpp
    resources/resources.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ui
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Multimedia
    Qt6::Svg
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
