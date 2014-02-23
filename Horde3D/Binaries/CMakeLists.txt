# Copy platform-dependent files to runtime output path.
FILE(GLOB_RECURSE in_file_list ${CMAKE_SYSTEM_NAME}/*)
FOREACH(in_file ${in_file_list})
    FILE(COPY ${in_file} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
ENDFOREACH(in_file)

# Copy content assets to output path.
FILE(COPY Content DESTINATION ${HORDE3D_OUTPUT_PATH_PREFIX})
