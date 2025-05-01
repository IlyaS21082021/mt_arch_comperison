cmake_minimum_required(VERSION 3.16)

project(DGMT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(DGMT main.cpp
    mclass.h mclass.cpp)

include(GNUInstallDirs)
install(TARGETS DGMT
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

target_link_libraries(DGMT pthread)
