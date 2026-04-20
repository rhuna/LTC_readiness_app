cmake_minimum_required(VERSION 3.20)
project(LTCSurveyReadinessApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(LTCSurveyReadinessApp WIN32
    src/main.cpp
    src/core/App.cpp
    src/core/App.h
    src/models/Models.h
    src/services/DataStore.cpp
    src/services/DataStore.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
)

target_include_directories(LTCSurveyReadinessApp PRIVATE src)


target_compile_definitions(LTCSurveyReadinessApp PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(LTCSurveyReadinessApp PRIVATE
    comctl32
)
