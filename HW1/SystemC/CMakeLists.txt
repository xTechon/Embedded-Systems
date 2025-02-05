cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 17)
set(SystemC_CXX_STANDARD 17)

project(hw1-sysc C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(SystemCLanguage CONFIG REQUIRED)
find_package(SystemCTLM)

add_executable(${PROJECT_NAME}
    src/hello_world.cpp
)

target_link_libraries(${PROJECT_NAME} SystemC::systemc)

