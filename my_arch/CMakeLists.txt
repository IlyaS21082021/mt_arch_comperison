cmake_minimum_required(VERSION 3.16)

project(MYMT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MYMT main.cpp
    mclass.h mclass.cpp)

include(GNUInstallDirs)
install(TARGETS MYMT
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
