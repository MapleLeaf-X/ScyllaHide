cmake_minimum_required(VERSION 2.8)

project(SCMRevGen NONE)

add_custom_target(${PROJECT_NAME} ALL cmd.exe /C "make_scmrev.h.bat"
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})