cmake_minimum_required(VERSION 2.8)

project(agb)

find_package( OpenCV REQUIRED )

add_definitions(-std=c++11 -lwiringPi -Wall)

add_executable(agb picamdriver.cpp XLoBorg.c xloborgdriver.cpp hwexception.cpp main.cpp)

install(TARGETS agb RUNTIME DESTINATION bin)

target_link_libraries( agb ${OpenCV_LIBS} pthread )

