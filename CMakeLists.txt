cmake_minimum_required(VERSION 3.16)
project(HospitalSim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics REQUIRED)

add_executable(hospital
    src/main.cpp
    src/App.cpp
    src/Simulation.cpp
    src/Doctor.cpp
    src/Patient.cpp)

# data/ and assets/ are read from the source tree, so run from anywhere.
target_compile_definitions(hospital PRIVATE DATA_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(hospital PRIVATE SFML::Graphics)
