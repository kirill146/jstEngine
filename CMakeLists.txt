cmake_minimum_required(VERSION 3.27 FATAL_ERROR)
project(jstEngine LANGUAGES C CXX)

set(AGILITY_SDK_VERSION 1.618.2)

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(editor)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT editor)