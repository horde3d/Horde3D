# - Try to find OpenGLES and EGL
# Once done this will define
#  
#  OPENGLES2_FOUND        - system has OpenGLES
#  OPENGLES2_INCLUDE_DIR  - the GL include directory
#  OPENGLES2_LIBRARIES    - Link these to use OpenGLES
#
#  EGL_FOUND        - system has EGL
#  EGL_INCLUDE_DIR  - the EGL include directory
#  EGL_LIBRARIES    - Link these to use EGL

IF (WIN32)
  IF (CYGWIN)

    FIND_PATH(OPENGLES2_INCLUDE_DIR GLES2/gl2.h )

    FIND_LIBRARY(OPENGLES2_gl_LIBRARY libGLESv2 )

  ELSE (CYGWIN)

    IF(BORLAND)
      SET (OPENGLES2_gl_LIBRARY import32 CACHE STRING "OpenGL ES 2.x library for win32")
    ELSE(BORLAND)
	  #MS compiler - todo - fix the following line:
      SET (OPENGLES2_gl_LIBRARY  /lib/release/libGLESv2.lib CACHE STRING "OpenGL ES 2.x library for win32")
	  SET (OPENGLES2_INCLUDE_DIR /include/ CACHE STRING "OpenGL ES 3.x include paths for win32")
      SET (EGL_egl_LIBRARY       /lib/release/libEGL.lib CACHE STRING "EGL library for win32")	
	  SET (EGL_INCLUDE_DIR       /include/ CACHE STRING "EGL include paths for win32")	  
    ENDIF(BORLAND)

  ENDIF (CYGWIN)

ELSE (WIN32)

  IF (APPLE)

  # Construct search paths for includes and libraries from a PREFIX_PATH
  macro(create_search_paths PREFIX)
  foreach(dir ${${PREFIX}_PREFIX_PATH})
    set(${PREFIX}_INC_SEARCH_PATH ${${PREFIX}_INC_SEARCH_PATH}
      ${dir}/include ${dir}/include/${PREFIX} ${dir}/Headers)
    set(${PREFIX}_LIB_SEARCH_PATH ${${PREFIX}_LIB_SEARCH_PATH}
      ${dir}/lib ${dir}/lib/${PREFIX} ${dir}/Libs)
  endforeach(dir)
  set(${PREFIX}_FRAMEWORK_SEARCH_PATH ${${PREFIX}_PREFIX_PATH})
  endmacro(create_search_paths)

  # Slightly customised framework finder
MACRO(findpkg_framework fwk)
IF(APPLE)
  SET(${fwk}_FRAMEWORK_PATH
    ${${fwk}_FRAMEWORK_SEARCH_PATH}
    ${CMAKE_FRAMEWORK_PATH}
    ~/Library/Frameworks
    /Library/Frameworks
    /System/Library/Frameworks
    /Network/Library/Frameworks
    /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.0.sdk/System/Library/Frameworks/
    /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/System/Library/Frameworks;
  )
  FOREACH(dir ${${fwk}_FRAMEWORK_PATH})
    SET(fwkpath ${dir}/${fwk}.framework)
    IF(EXISTS ${fwkpath})
      SET(${fwk}_FRAMEWORK_INCLUDES ${${fwk}_FRAMEWORK_INCLUDES}
        ${fwkpath}/Headers ${fwkpath}/PrivateHeaders)
      if (NOT ${fwk}_LIBRARY_FWK)
        SET(${fwk}_LIBRARY_FWK "-framework ${fwk}")
      endif ()
    ENDIF(EXISTS ${fwkpath})
  ENDFOREACH(dir)
ENDIF(APPLE)
ENDMACRO(findpkg_framework)

	create_search_paths(/Applications/Xcode.app/Contents/Developer/Platforms)
	findpkg_framework(OpenGLES)
    set(OPENGLES2_gl_LIBRARY "-framework OpenGLES")

  ELSE(APPLE)

    FIND_PATH(OPENGLES2_INCLUDE_DIR GLES2/gl2.h
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include /usr/X11R6/include
      /usr/include
    )

    FIND_LIBRARY(OPENGLES2_gl_LIBRARY
      NAMES GLESv2
      PATHS /opt/graphics/OpenGL/lib
            /usr/openwin/lib
            /usr/shlib /usr/X11R6/lib
            /usr/lib
    )

    FIND_PATH(EGL_INCLUDE_DIR EGL/egl.h
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include /usr/X11R6/include
      /usr/include
    )

    FIND_LIBRARY(EGL_egl_LIBRARY
      NAMES EGL
      PATHS /opt/graphics/OpenGL/lib
            /usr/openwin/lib
            /usr/shlib /usr/X11R6/lib
            /usr/lib
    )

    # On Unix OpenGL most certainly always requires X11.
    # Feel free to tighten up these conditions if you don't 
    # think this is always true.
    # It's not true on OSX.

    IF (OPENGLES2_gl_LIBRARY)
      IF(NOT X11_FOUND)
        INCLUDE(FindX11)
      ENDIF(NOT X11_FOUND)
      IF (X11_FOUND)
        IF (NOT APPLE)
          SET (OPENGLES2_LIBRARIES ${X11_LIBRARIES})
        ENDIF (NOT APPLE)
      ENDIF (X11_FOUND)
    ENDIF (OPENGLES2_gl_LIBRARY)

  ENDIF(APPLE)
ENDIF (WIN32)

SET( OPENGLES2_FOUND "YES" )
IF(OPENGLES2_gl_LIBRARY AND EGL_egl_LIBRARY)

    SET( OPENGLES2_LIBRARIES ${OPENGLES2_gl_LIBRARY} ${OPENGLES2_LIBRARIES})
    SET( EGL_LIBRARIES ${EGL_egl_LIBRARY} ${EGL_LIBRARIES})
    SET( OPENGLES2_FOUND "YES" )

ENDIF(OPENGLES2_gl_LIBRARY AND EGL_egl_LIBRARY)

MARK_AS_ADVANCED(
  OPENGLES2_INCLUDE_DIR
  OPENGLES2_gl_LIBRARY
  EGL_INCLUDE_DIR
  EGL_egl_LIBRARY
)
