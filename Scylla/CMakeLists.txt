cmake_minimum_required(VERSION 3.1)

project(Scylla CXX)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose build type.")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
#set_target_properties(${PROJECT_NAME} PROPERTIES BITNESS x86 x64)

#if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
#    set_property()
#endif()

#set_target_properties(${TARGET} PROPERTIES
#    CXX_STANDARD 17
#    CXX_STANDARD_REQUIRED YES
#    CXX_EXTENSIONS NO
#)

#message(${TARGET})

if(MINGW)
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
endif()

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

add_definitions(-DWINVER=0x0501 -D_WIN32_WINNT=0x0501 -DNTDDI_VERSION=0x05010000)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE .. PUBLIC ../3rdparty)

#target_link_directories(${PROJECT_NAME} INTERFACE ../3rdparty)
target_link_libraries(${PROJECT_NAME} INTERFACE ../3rdparty/ntdll_${BITNESS}.lib)

set(CMAKE_CXX_FLAGS "-fno-ident -s")