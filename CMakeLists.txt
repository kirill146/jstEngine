cmake_minimum_required(VERSION 3.27 FATAL_ERROR)
project(jstEngine LANGUAGES C CXX)

add_subdirectory(engine)
add_subdirectory(editor)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT editor)