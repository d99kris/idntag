# Project
cmake_minimum_required(VERSION 3.14...3.22 FATAL_ERROR)
project(idntag LANGUAGES CXX C)
set(APP_TARGET ${PROJECT_NAME})
include(CheckIncludeFile)
set(CMAKE_CXX_STANDARD 17)
message(STATUS "Using cmake ${CMAKE_VERSION}")

# Modules
include(CheckIncludeFiles)

# Packages
find_package(PkgConfig REQUIRED)

# Ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Found ccache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# Build type
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Using build type '${DEFAULT_BUILD_TYPE}' (default).")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}")
else()
  message(STATUS "Using build type '${CMAKE_BUILD_TYPE}'.")
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Application
add_executable(${APP_TARGET}
  src/acoustid.cpp
  src/acoustid.h
  src/editor.cpp
  src/editor.h
  src/log.cpp
  src/log.h
  src/main.cpp
  src/main.h
  src/tag.cpp
  src/tag.h
  src/util.cpp
  src/util.h
  src/version.cpp
  src/version.h
)
install(TARGETS ${APP_TARGET} DESTINATION bin)

# Compiler flags
set_target_properties(${APP_TARGET} PROPERTIES COMPILE_FLAGS
                      "-Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith \
                       -Wcast-qual -Wno-missing-braces -Wswitch-default \
                       -Wunreachable-code -Wuninitialized -Wcast-align")

# Platform specifics
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-D_XOPEN_SOURCE_EXTENDED)
  execute_process(COMMAND sh "-c"
                  "command -v brew &> /dev/null && brew --prefix ncurses 2> /dev/null | tr -d '\n'"
                  OUTPUT_VARIABLE CMAKE_PREFIX_PATH)
  if (EXISTS "${CMAKE_PREFIX_PATH}")
    message(STATUS "Ncurses cmake prefix '${CMAKE_PREFIX_PATH}' (detected).")
  else()
    set(CMAKE_PREFIX_PATH /opt/local)
    message(STATUS "Ncurses cmake prefix '${CMAKE_PREFIX_PATH}' (default).")
  endif()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
  add_compile_definitions(_XOPEN_SOURCE_EXTENDED)
  if (DEFINED ENV{PREFIX} AND "$ENV{PREFIX}" MATCHES "^/data/data/com\\.termux/files/usr$")
    message(STATUS "Found termux")
    add_compile_definitions(HAVE_TERMUX)
  endif()
endif()

# Dependency curl
find_package(CURL REQUIRED)
target_link_libraries(${APP_TARGET} PRIVATE CURL::libcurl)

# Dependency ncurses
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)
target_link_libraries(${APP_TARGET} PRIVATE ${CURSES_LIBRARIES})
target_include_directories(${APP_TARGET} PRIVATE ${CURSES_INCLUDE_DIR})

# Dependency tag
pkg_check_modules(TAGLIB REQUIRED taglib)
target_include_directories(${APP_TARGET} PRIVATE ${TAGLIB_INCLUDE_DIRS})
target_link_directories(${APP_TARGET} PRIVATE ${TAGLIB_LIBRARY_DIRS})
target_link_libraries(${APP_TARGET} PRIVATE ${TAGLIB_LIBRARIES})

# Dependency nlohmann-json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${APP_TARGET} PRIVATE nlohmann_json::nlohmann_json)

# Manual
install(FILES src/${APP_TARGET}.1 DESTINATION share/man/man1)

# Uninstall
add_custom_target(uninstall
  COMMAND "${CMAKE_COMMAND}" -E remove "${CMAKE_INSTALL_PREFIX}/bin/${APP_TARGET}"
  COMMAND "${CMAKE_COMMAND}" -E remove "${CMAKE_INSTALL_PREFIX}/share/man/man1/${APP_TARGET}.1"
)

# Test init
enable_testing()

# Test macro add_unit_test
macro (add_unit_test testname)
  configure_file(tests/${testname} ${CMAKE_CURRENT_BINARY_DIR}/${testname} COPYONLY)
  add_test(${testname} "${PROJECT_BINARY_DIR}/${testname}")
  set_tests_properties(${testname} PROPERTIES RUN_SERIAL TRUE)
endmacro (add_unit_test)

# Tests
add_unit_test(test001)
add_unit_test(test002)
add_unit_test(test003)
add_unit_test(test004)
add_unit_test(test005)
add_unit_test(test006)
add_unit_test(test007)
