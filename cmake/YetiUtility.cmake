# utility functions

# add include dir
function(add_include_dir NAME MODE)
    target_include_directories(
            ${NAME} ${MODE}
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
endfunction()

# add header files to installation
function(add_header_files NAME MODE)
    set(_options)
    set(_keys)
    set(_lists HEADERS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    target_sources(
            ${NAME} ${MODE}
            # using FILE_SET - cmake >= 3.23 required
            FILE_SET HEADERS TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
            FILES ${var_HEADERS}
#            # CMake error ... property contains path ... which is prefixed in the source directory
#            ${var_HEADERS}
    )

#    # PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Development
#    set_target_properties(${NAME}
#            # note: using of quotation marks is required for list passing
#            PROPERTIES PUBLIC_HEADER "${var_HEADERS}"
#    )
endfunction()
