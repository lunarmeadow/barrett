# - Try to find ctrulib
# You can set CTRULIB_ROOT to specify a certain directory to look in first.
# Once done this will define
#  CTRULIB_FOUND - System has ctrulib
#  CTRULIB_INCLUDE_DIRS - The ctrulib include directories
#  CTRULIB_LIBRARIES - The libraries needed to use ctrulib
# It also adds an imported target named `3ds::ctrulib`, Linking against it is
# equivalent to:
# target_link_libraries(mytarget ${CTRULIB_LIBRARY})
# target_include_directories(mytarget PRIVATE ${CTRULIB_INCLUDE_DIRS})

if(NOT 3DS)
    message(FATAL_ERROR "This module can only be used if you are using the 3DS toolchain file. Please erase this build directory or create another one, and then use -DCMAKE_TOOLCHAIN_FILE=DevkitArm3DS.cmake when calling cmake for the 1st time. For more information, see the Readme.md for more information.")
endif()

include(LibFindMacros)
include(try_add_imported_target)

set(_CTRULIB_SEARCHES)

# Search CTRULIB_ROOT first if it is set.
if(CTRULIB_ROOT)
  set(_CTRULIB_SEARCH_ROOT
    PATHS ${CTRULIB_ROOT}
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH)
  list(APPEND _CTRULIB_SEARCHES _CTRULIB_SEARCH_ROOT)
endif()

# Search below ${DEVKITPRO}, ${DEVKITARM} etc.
set(_CTRULIB_SEARCH_NORMAL
  PATHS / /libctru /ctrulib
  NO_DEFAULT_PATH
  ONLY_CMAKE_FIND_ROOT_PATH)
list(APPEND _CTRULIB_SEARCHES _CTRULIB_SEARCH_NORMAL)

foreach(search ${_CTRULIB_SEARCHES})
  find_path(CTRULIB_INCLUDE_DIR NAMES 3ds.h
    ${${search}}
    PATH_SUFFIXES include libctru/include)
  find_library(CTRULIB_LIBRARY NAMES ctru libctru.a
    ${${search}}
    PATH_SUFFIXES lib libctru/lib)
endforeach()

set(CTRULIB_PROCESS_INCLUDES CTRULIB_INCLUDE_DIR)
set(CTRULIB_PROCESS_LIBS CTRULIB_LIBRARY)

libfind_process(CTRULIB)

try_add_imported_target(CTRULIB)

if(CTRULIB_FOUND AND NOT DEFINED CTRULIB)
  get_filename_component(CTRULIB ${CTRULIB_INCLUDE_DIR} DIRECTORY)
  message(STATUS "Setting CTRULIB to ${CTRULIB}")
endif()
