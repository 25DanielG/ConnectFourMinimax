cmake_minimum_required(VERSION 3.13.4)
project(connectFour)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
link_libraries(${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
add_executable(${PROJECT_NAME} main.cpp)
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} connectFourBuild)