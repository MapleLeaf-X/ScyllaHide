cmake_minimum_required(VERSION 2.8)

project(Scylla CXX)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

set(HEADERS
    Logger.h
    NtApiShim.h
    PebHider.h
    Resource.h
    Settings.h
    OsInfo.h
    Peb.h
    User32Loader.h
    Util.h
    Version.h
    Win32kSyscalls.h
)

set(SOURCES
    Logger.cpp
    PebHider.cpp
    Settings.cpp
    OsInfo.cpp
    Peb.cpp
    User32Loader.cpp
    Util.cpp
    Version.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_dependencies(${PROJECT_NAME} SCMRevGen)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-ident -s")