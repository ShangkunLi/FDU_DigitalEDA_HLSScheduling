cmake_minimum_required(VERSION 3.5)

project(Schedule)

set(SOURCES 
    src/controlflow.cpp
    src/dataflow.cpp
    src/hls.cpp
    src/main.cpp
    src/parser.cpp
    src/schedule.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)