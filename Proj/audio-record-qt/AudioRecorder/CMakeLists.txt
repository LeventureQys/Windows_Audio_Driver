cmake_minimum_required(VERSION 3.5.0)
project(AudioRecorder_Qt VERSION 0.1.0 LANGUAGES C CXX)
find_package(Qt5 COMPONENTS Core Widgets Multimedia REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(AudioRecorder_Qt SHARED AudioRecorder.cpp AudioRecorder.h)

target_link_libraries(AudioRecorder_Qt PRIVATE Qt5::Core Qt5::Widgets Qt5::Multimedia)
target_compile_definitions(AudioRecorder_Qt PRIVATE AUDIORECORDER_EXPORT)
target_include_directories(AudioRecorder_Qt PUBLIC ./)
target_compile_definitions(AudioRecorder_Qt PRIVATE AUDIORECORDER)