cmake_minimum_required(VERSION 3.30)
project(COP3530_Project_2_Group_163)

set(CMAKE_CXX_STANDARD 20)

# from https://stackoverflow.com/questions/18213683/how-locate-cpp-files-in-a-src-directory-using-cmake
file(GLOB SOURCES "src/*.cpp")

add_executable(semantic ${SOURCES}
        src/Words.h)
