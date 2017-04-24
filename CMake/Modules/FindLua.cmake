# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindLua
# -------
#
#
#
# Locate Lua library This module defines
#
# ::
#
#   LUA_FOUND          - if false, do not try to link to Lua
#   LUA_LIBRARIES      - both lua and lualib
#   LUA_INCLUDE_DIR    - where to find lua.h
#   LUA_VERSION_STRING - the version of Lua found
#   LUA_VERSION_MAJOR  - the major version of Lua
#   LUA_VERSION_MINOR  - the minor version of Lua
#   LUA_VERSION_PATCH  - the patch version of Lua
#
#
#
# Note that the expected include convention is
#
# ::
#
#   #include "lua.h"
#
# and not
#
# ::
#
#   #include <lua/lua.h>
#
# This is because, the lua location is not standardized and may exist in
# locations other than lua/


unset(_lua_include_subdirs)
unset(_lua_library_names)
unset(_lua_append_versions)

# this is a function only to have all the variables inside go away automatically
function(_lua_set_version_vars)
    set(LUA_VERSIONS5 5.3 5.2 5.1 5.0)

    if (Lua_FIND_VERSION_EXACT)
        if (Lua_FIND_VERSION_COUNT GREATER 1)
            set(_lua_append_versions ${Lua_FIND_VERSION_MAJOR}.${Lua_FIND_VERSION_MINOR})
        endif ()
    elseif (Lua_FIND_VERSION)
        # once there is a different major version supported this should become a loop
        if (NOT Lua_FIND_VERSION_MAJOR GREATER 5)
            if (Lua_FIND_VERSION_COUNT EQUAL 1)
                set(_lua_append_versions ${LUA_VERSIONS5})
            else ()
                foreach (subver IN LISTS LUA_VERSIONS5)
                    if (NOT subver VERSION_LESS ${Lua_FIND_VERSION})
                        list(APPEND _lua_append_versions ${subver})
                    endif ()
                endforeach ()
            endif ()
        endif ()
    else ()
        # once there is a different major version supported this should become a loop
        set(_lua_append_versions ${LUA_VERSIONS5})
    endif ()

    list(APPEND _lua_include_subdirs "include/lua" "include")

    foreach (ver IN LISTS _lua_append_versions)
        string(REGEX MATCH "^([0-9]+)\\.([0-9]+)$" _ver "${ver}")
        list(APPEND _lua_include_subdirs
             include/lua${CMAKE_MATCH_1}${CMAKE_MATCH_2}
             include/lua${CMAKE_MATCH_1}.${CMAKE_MATCH_2}
             include/lua-${CMAKE_MATCH_1}.${CMAKE_MATCH_2}
        )
    endforeach ()

    set(_lua_include_subdirs "${_lua_include_subdirs}" PARENT_SCOPE)
    set(_lua_append_versions "${_lua_append_versions}" PARENT_SCOPE)
endfunction(_lua_set_version_vars)

function(_lua_check_header_version _hdr_file)
    # At least 5.[012] have different ways to express the version
    # so all of them need to be tested. Lua 5.2 defines LUA_VERSION
    # and LUA_RELEASE as joined by the C preprocessor, so avoid those.
    file(STRINGS "${_hdr_file}" lua_version_strings
         REGEX "^#define[ \t]+LUA_(RELEASE[ \t]+\"Lua [0-9]|VERSION([ \t]+\"Lua [0-9]|_[MR])).*")

    string(REGEX REPLACE ".*;#define[ \t]+LUA_VERSION_MAJOR[ \t]+\"([0-9])\"[ \t]*;.*" "\\1" LUA_VERSION_MAJOR ";${lua_version_strings};")
    if (LUA_VERSION_MAJOR MATCHES "^[0-9]+$")
        string(REGEX REPLACE ".*;#define[ \t]+LUA_VERSION_MINOR[ \t]+\"([0-9])\"[ \t]*;.*" "\\1" LUA_VERSION_MINOR ";${lua_version_strings};")
        string(REGEX REPLACE ".*;#define[ \t]+LUA_VERSION_RELEASE[ \t]+\"([0-9])\"[ \t]*;.*" "\\1" LUA_VERSION_PATCH ";${lua_version_strings};")
        set(LUA_VERSION_STRING "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}.${LUA_VERSION_PATCH}")
    else ()
        string(REGEX REPLACE ".*;#define[ \t]+LUA_RELEASE[ \t]+\"Lua ([0-9.]+)\"[ \t]*;.*" "\\1" LUA_VERSION_STRING ";${lua_version_strings};")
        if (NOT LUA_VERSION_STRING MATCHES "^[0-9.]+$")
            string(REGEX REPLACE ".*;#define[ \t]+LUA_VERSION[ \t]+\"Lua ([0-9.]+)\"[ \t]*;.*" "\\1" LUA_VERSION_STRING ";${lua_version_strings};")
        endif ()
        string(REGEX REPLACE "^([0-9]+)\\.[0-9.]*$" "\\1" LUA_VERSION_MAJOR "${LUA_VERSION_STRING}")
        string(REGEX REPLACE "^[0-9]+\\.([0-9]+)[0-9.]*$" "\\1" LUA_VERSION_MINOR "${LUA_VERSION_STRING}")
        string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]).*" "\\1" LUA_VERSION_PATCH "${LUA_VERSION_STRING}")
    endif ()
    foreach (ver IN LISTS _lua_append_versions)
        if (ver STREQUAL "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}")
            set(LUA_VERSION_MAJOR ${LUA_VERSION_MAJOR} PARENT_SCOPE)
            set(LUA_VERSION_MINOR ${LUA_VERSION_MINOR} PARENT_SCOPE)
            set(LUA_VERSION_PATCH ${LUA_VERSION_PATCH} PARENT_SCOPE)
            set(LUA_VERSION_STRING ${LUA_VERSION_STRING} PARENT_SCOPE)
            return()
        endif ()
    endforeach ()
endfunction(_lua_check_header_version)

_lua_set_version_vars()

if (LUA_INCLUDE_DIR AND EXISTS "${LUA_INCLUDE_DIR}/lua.h")
    _lua_check_header_version("${LUA_INCLUDE_DIR}/lua.h")
endif ()

if (NOT LUA_VERSION_STRING)
    foreach (subdir IN LISTS _lua_include_subdirs)
        unset(LUA_INCLUDE_PREFIX CACHE)
        find_path(LUA_INCLUDE_PREFIX ${subdir}/lua.h
          HINTS
            ENV LUA_DIR
          PATHS
          ~/Library/Frameworks
          /Library/Frameworks
          /sw # Fink
          /opt/local # DarwinPorts
          /opt/csw # Blastwave
          /opt
        )
        if (LUA_INCLUDE_PREFIX)
            _lua_check_header_version("${LUA_INCLUDE_PREFIX}/${subdir}/lua.h")
            if (LUA_VERSION_STRING)
                set(LUA_INCLUDE_DIR "${LUA_INCLUDE_PREFIX}/${subdir}")
                break()
            endif ()
        endif ()
    endforeach ()
endif ()
unset(_lua_include_subdirs)
unset(_lua_append_versions)

#MESSAGE(STATUS "Library prefix: ${CMAKE_FIND_LIBRARY_PREFIXES} Suffix: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
if (LUA_VERSION_STRING)
    set(_lua_library_names
        lua${LUA_VERSION_MAJOR}${LUA_VERSION_MINOR}
        lua${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}
        lua-${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}
        lua.${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}
    )
endif ()

#MESSAGE("LUA_LIBRARY is ${LUA_LIBRARY_PATH}")

IF(NOT LUA_LIBRARY_PATH )

find_library(LUA_LIBRARY_PATH
  NAMES ${_lua_library_names} lua
  HINTS
  ENV LUA_DIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
)
ENDIF()

unset(_lua_library_names)

IF(NOT LUA_LIBRARY_PATH OR HORDE3D_FORCE_DOWNLOAD_LUA)
    IF(MINGW)
       SET(LUA_PLATFORM "LUA_BUILD_AS_DLL")
    ELSEIF(UNIX AND NOT APPLE )
       SET(LUA_PLATFORM "LUA_USE_LINUX")
    ELSEIF(APPLE)
       SET(LUA_PLATFORM "LUA_USE_MACOSX")
    ELSE()
       SET(LUA_PLATFORM "")
    ENDIF()
    # If not found, try to build with local sources.
    # It uses CMake's "ExternalProject_Add" target.
    MESSAGE(STATUS "Preparing external LUA project ${CMAKE_CURRENT_LIST_DIR}")

    INCLUDE(ExternalProject)
    ExternalProject_Add(project_lua
        URL https://www.lua.org/ftp/lua-5.3.4.tar.gz
        URL_MD5 53a9c68bcc0eda58bdc2095ad5cdfc63
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DDEFINITIONS=${LUA_PLATFORM}
        PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/LuaCMake.txt <SOURCE_DIR>/CMakeLists.txt
        LOG_DOWNLOAD 1
        LOG_UPDATE 1
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_TEST 1
        LOG_INSTALL 1
    )
    MESSAGE(STATUS "External LUA project done")

    ExternalProject_Get_Property(project_lua install_dir)
    SET(LUA_INCLUDE_DIR ${install_dir}/include )

    IF(MSVC)
       SET(LUA_LIBRARY_PATH  ${install_dir}/lib/lua.lib )
    ELSE(MSVC)
       SET(LUA_LIBRARY_PATH ${install_dir}/lib/liblua.a )
    ENDIF(MSVC)

    add_library(LUA_LIBRARY UNKNOWN IMPORTED)
    set_property(TARGET LUA_LIBRARY PROPERTY IMPORTED_LOCATION  ${LUA_LIBRARY_PATH} )
    set_property(TARGET LUA_LIBRARY PROPERTY IMPORTED_LOCATION  ${LUA_LIBRARY_PATH} )
    add_custom_command(OUTPUT ${LUA_LIBRARY_PATH} VERBATIM COMMAND ${CMAKE_COMMAND} -E echo "Building LUA" DEPENDS project_lua)
    add_custom_target(LUA_LIBRARY_EXTERN DEPENDS ${LUA_LIBRARY_PATH} project_lua)
    add_dependencies(LUA_LIBRARY project_lua)
    add_dependencies(LUA_LIBRARY LUA_LIBRARY_EXTERN)
    get_filename_component(LUA_LIB_EXT ${LUA_LIBRARY_PATH} EXT)
    set(LUA_LIBRARY_PATH LUA_LIBRARY )
ELSE(NOT LUA_LIBRARY_PATH OR HORDE3D_FORCE_DOWNLOAD_LUA)
    get_filename_component(LUA_LIB_EXT ${LUA_LIBRARY_PATH} EXT)
ENDIF(NOT LUA_LIBRARY_PATH OR HORDE3D_FORCE_DOWNLOAD_LUA)


if (LUA_LIBRARY_PATH)
    # include the math library for Unix
    if (UNIX AND NOT APPLE AND NOT BEOS)
        find_library(LUA_MATH_LIBRARY m)
        set(LUA_LIBRARIES "${LUA_LIBRARY_PATH};${LUA_MATH_LIBRARY}")

        # include dl library for statically-linked Lua library
        if(LUA_LIB_EXT STREQUAL CMAKE_STATIC_LIBRARY_SUFFIX)
          list(APPEND LUA_LIBRARIES ${CMAKE_DL_LIBS})
        endif()

    # For Windows and Mac, don't need to explicitly include the math library
    else ()
        set(LUA_LIBRARIES "${LUA_LIBRARY_PATH}")
    endif ()

    # Set the final string here so the GUI reflects the final state.
    SET(LUA_LIBRARY_PATH ${LUA_LIBRARY_PATH} CACHE STRING "Where the LUA Library can be found")

    #MESSAGE("LUA_LIBRARY is now ${LUA_LIBRARY_PATH}")

endif ()

#include(FindPackageHandleStandardArgs)
## handle the QUIETLY and REQUIRED arguments and set LUA_FOUND to TRUE if
## all listed variables are TRUE
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lua
#                                  REQUIRED_VARS LUA_LIBRARIES LUA_INCLUDE_DIR
#                                  VERSION_VAR LUA_VERSION_STRING)

#mark_as_advanced(LUA_INCLUDE_DIR LUA_LIBRARY LUA_MATH_LIBRARY)

MESSAGE( STATUS "Lua Found: ${LUA_INCLUDE_DIR} ${LUA_LIBRARIES}" )
