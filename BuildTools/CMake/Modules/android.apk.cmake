#*********************************************************#
#*  File: Apk.cmake                                      *
#*    Android apk tools
#*
#*  Copyright (C) 2002-2013 The PixelLight Team (http://www.pixellight.org/)
#*	Copyright (C) 2019 The Horde3D Team (http://www.horde3d.org/)
#*	
#*
#*  This file is part of PixelLight.
#*
#*  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
#*  and associated documentation files (the "Software"), to deal in the Software without
#*  restriction, including without limitation the rights to use, copy, modify, merge, publish,
#*  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
#*  Software is furnished to do so, subject to the following conditions:
#*
#*  The above copyright notice and this permission notice shall be included in all copies or
#*  substantial portions of the Software.
#*
#*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
#*  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#*  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
#*  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#*  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#*********************************************************#


##################################################
## Options
##################################################
set(ANDROID_APK_API_LEVEL "24" CACHE STRING "Android APK API level")
set(ANDROID_APK_INSTALL "0" CACHE BOOL "Install created apk file on the device automatically?")
set(ANDROID_APK_RUN "0" CACHE BOOL "Run created apk file on the device automatically? (installs it automatically as well, \"ANDROID_APK_INSTALL\"-option is ignored)")
set(ANDROID_APK_SIGNER_KEYSTORE	"~/my-release-key.keystore" CACHE STRING "Keystore for signing the apk file (only required for release apk)")
set(ANDROID_APK_SIGNER_ALIAS "myalias" CACHE STRING "Alias for signing the apk file (only required for release apk)")

##################################################
## Variables
##################################################
set(ANDROID_THIS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})	# Directory this CMake file is in

##################################################
## MACRO: android_create_apk
##
## Create/copy Android apk related files
##
## @param name
##   Name of the project (e.g. "MyProject"), this will also be the name of the created apk file
## @param apk_package_name
##   Package name of the application
## @param apk_directory
##   Directory where to construct the apk file in (e.g. "${CMAKE_BINARY_DIR}/apk")
## @param libs_directory
##   Directory where the built android libraries will be POST_BUILD, e.g ${CMAKE_SOURCE_DIR}/libs 
## @param assets_directory
##   Directory where the assets for the application are located
##   
## @remarks
##   Requires the following tools to be found automatically
##   - "adb" (part of the Android SDK)
##   - "gradle" (included)
##   - "jarsigner" (part of the JDK)
##   - "zipalign" (part of the Android SDK)
##################################################
macro(android_create_apk name apk_package_name apk_directory libs_directory android_directory assets_directory)
  set(ANDROID_NAME ${name})
  set(ANDROID_APK_PACKAGE ${apk_package_name})

  # Set gradle find path
  if( ${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows" )
    set( GRADLE_BIN ${apk_directory}/gradlew.bat)
  
  endif()
  
  
  # Copy project
  add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${android_directory}" "${apk_directory}")
  
  # Create the directory for the libraries
  add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${apk_directory}/app/src/main/jnilibs/${ANDROID_ABI}")
  add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${apk_directory}/app/src/main/jnilibs/${ANDROID_ABI}")
  # add_custom_command(TARGET ${ANDROID_NAME} POST_BUILD
  #   COMMAND ${CMAKE_COMMAND} -E copy_directory
  #   "${CMAKE_SOURCE_DIR}/libs" "${apk_directory}/libs/")
  
  if(CMAKE_BUILD_TYPE MATCHES Release)
    set(ANDROID_APK_DEBUGGABLE "false")
  else()
    set(ANDROID_APK_DEBUGGABLE "true")
  endif()

  # Configure gradle project with correct package name
  configure_file("${android_directory}/app/build.gradle.in" "${apk_directory}/app/build.gradle")

  # Copy assets
  add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${apk_directory}/app/src/main/assets")
  add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${apk_directory}/app/src/main/assets/Content")
  add_custom_command(TARGET ${ANDROID_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${assets_directory} "${apk_directory}/app/src/main/assets/Content")

  # Build the apk file
  if(CMAKE_BUILD_TYPE MATCHES Release)
    # Let Gradle create the unsigned apk file
    add_custom_command(TARGET ${ANDROID_NAME}
      COMMAND ${GRADLE_BIN} assembleRelease
      WORKING_DIRECTORY "${apk_directory}/app")

    # Sign the apk file
    add_custom_command(TARGET ${ANDROID_NAME}
      COMMAND jarsigner -verbose -keystore ${ANDROID_APK_SIGNER_KEYSTORE} bin/${ANDROID_NAME}-unsigned.apk ${ANDROID_APK_SIGNER_ALIAS}
      WORKING_DIRECTORY "${apk_directory}/app")

    # Align the apk file
    add_custom_command(TARGET ${ANDROID_NAME}
      COMMAND zipalign -v -f 4 bin/${ANDROID_NAME}-unsigned.apk bin/${ANDROID_NAME}.apk
      WORKING_DIRECTORY "${apk_directory}/app")
    
    # Install current version on the device/emulator
    # if(ANDROID_APK_INSTALL OR ANDROID_APK_RUN)
    #   add_custom_command(TARGET ${ANDROID_NAME}
    #   COMMAND adb install -r bin/${ANDROID_NAME}.apk
    #   WORKING_DIRECTORY "${apk_directory}/app")
    # endif()
  else()
    # Let Gradle create the unsigned apk file
    add_custom_command(TARGET ${ANDROID_NAME} POST_BUILD
      COMMAND ${GRADLE_BIN} assembleDebug --no-daemon
      WORKING_DIRECTORY "${apk_directory}/app")
    
    # Rename the 'app' apk to target name
    add_custom_command(TARGET ${ANDROID_NAME}
      COMMAND ${CMAKE_COMMAND} -E rename 
      "${apk_directory}../../../Binaries/Android/${CMAKE_BUILD_TYPE}/app-debug.apk" 
      "${apk_directory}../../../Binaries/Android/${CMAKE_BUILD_TYPE}/${ANDROID_NAME}-debug.apk" )
    
    # Install current version on the device/emulator
    # if(ANDROID_APK_INSTALL OR ANDROID_APK_RUN)
    #   add_custom_command(TARGET ${ANDROID_NAME}
    #   COMMAND adb install -r bin/${ANDROID_NAME}-debug.apk
    #   WORKING_DIRECTORY "${apk_directory}/app")
    # endif()
  endif()

  # Start the application
  # if(ANDROID_APK_RUN)
  #   add_custom_command(TARGET ${ANDROID_NAME}
  #     COMMAND adb shell am start -n ${ANDROID_APK_PACKAGE}/android.app.NativeActivity)
  # endif()
endmacro(android_create_apk name apk_directory libs_directory assets_directory)
