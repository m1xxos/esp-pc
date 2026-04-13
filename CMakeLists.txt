cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED APP_PROTOCOL)
    set(APP_PROTOCOL "matter" CACHE STRING "Protocol profile: matter|zigbee|none")
endif()
set_property(CACHE APP_PROTOCOL PROPERTY STRINGS matter zigbee none)

# ESP-IDF v5.2 defaults to gnu++2b, but current esp_matter release in this
# project has known incompatibilities with that mode and strict format warning
# settings under GCC 13. Apply local compatibility flags unless user already
# provides explicit equivalents.
set(_project_extra_cxxflags "$ENV{EXTRA_CXXFLAGS}")
if(NOT _project_extra_cxxflags MATCHES "(^|[ \t])-std=")
    string(APPEND _project_extra_cxxflags " -std=gnu++17")
endif()
if(NOT _project_extra_cxxflags MATCHES "(^|[ \t])-Wno-error=format-nonliteral([ \t]|$)")
    string(APPEND _project_extra_cxxflags " -Wno-error=format-nonliteral")
endif()
string(STRIP "${_project_extra_cxxflags}" _project_extra_cxxflags)
set(ENV{EXTRA_CXXFLAGS} "${_project_extra_cxxflags}")

set(PROJECT_VER "0.1.0")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp_pc_switch)
