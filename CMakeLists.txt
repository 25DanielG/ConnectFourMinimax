cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)

project(connectFour)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
project(${PROJECT_NAME} CXX)


find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(Boost REQUIRED serialization)
find_package(Boost COMPONENTS system REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} connectFourBuild)
include_directories(${Boost_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${Boost_SYSTEM_LIBRARY} Boost::boost Boost::serialization)
target_compile_options(${PROJECT_NAME} PUBLIC -std=c++20)