# Copy content assets to output path.
FILE(COPY Content DESTINATION ${HORDE3D_OUTPUT_PATH_PREFIX})

# Copy platform-dependent files to runtime output path.
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME})

ADD_CUSTOM_TARGET(CopyPlatformFiles ALL)

SET(out_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
IF(NOT ${CMAKE_CFG_INTDIR} STREQUAL ".")
    SET(out_dir ${out_dir}/${CMAKE_CFG_INTDIR}/)
ENDIF(NOT ${CMAKE_CFG_INTDIR} STREQUAL ".")



ADD_CUSTOM_COMMAND(TARGET CopyPlatformFiles PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E
                   copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME} ${out_dir})

ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME})
