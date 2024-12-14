# tools for compile programs

function(find_riscv_toolchain)
    set(TOOL_PREFIX rv_tools)
    set(_prefix "riscv64-unknown-elf-")
    set(_gcc "${_prefix}gcc")
    set(_c "${_prefix}gcc")
    set(_cxx "${_prefix}g++")
    set(_asm "${_prefix}as")
    set(_ar "${_prefix}ar")
    set(_linker "${_prefix}ld")
    set(_objcopy "${_prefix}objcopy")
    set(_objdump "${_prefix}objdump")

    set(_var_prefix "rv_tool")

    set(_var_names
            _gcc
            _c
            _cxx
            _asm
            _ar
            _linker
            _objcopy
            _objdump
    )
    foreach (_var IN LISTS _var_names)
        set(_var_name "${_var_prefix}${_var}")
        set(_var_value "${${_var}}")
        message(DEBUG "look for ${_var_value}")
        find_program(${_var_name} ${_var_value} REQUIRED)
        message(DEBUG "\t use ${${_var_name}}")
        set(_tool "${TOOL_PREFIX}::${_var}")
        add_executable(${_tool} IMPORTED)
        set_property(TARGET ${_tool} PROPERTY IMPORTED_LOCATION "${${_var_name}}")
    endforeach ()

#    set(_options "${TOOL_PREFIX}_options")
#    set(_tool_options "${TOOL_PREFIX}::_options")
#    add_library(${_options} INTERFACE)
#
#    add_library(${_tool_options} ALIAS ${_options})
endfunction()


function(riscv_add_dummy NAME EXT DEPS)
    set(_file "${NAME}.dummy.${EXT}")
    add_custom_command(
            OUTPUT ${_file}
            COMMENT "Generate dummy source ${_file}"
            COMMAND make_dummy "${_file}" "${NAME}"
            DEPENDS "${DEPS}"
    )
    add_library("${NAME}" SHARED)
    target_sources(
        "${NAME}"
        PUBLIC
            "${_file}"
    )
endfunction()

function(riscv_compile_source NAME)
    set(_options)
    set(_keys SOURCE OUTPUT)
    set(_lists OPTIONS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    set(_command rv_tools::_gcc) # TODO: c++ support

    add_custom_command(
            OUTPUT "${var_OUTPUT}"
            COMMENT "compile ${var_SOURCE}"
            COMMAND ${_command}
            ARGS ${var_OPTIONS} -c "${var_SOURCE}" -o "${var_OUTPUT}"
            DEPENDS "${var_SOURCE}"
    )
endfunction()

function(riscv_compile NAME)
    set(_options)
    set(_keys OUTPUT)
    set(_lists SOURCES OPTIONS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    if (NOT DEFINED var_OUTPUT)
        message(FATAL_ERROR "OUTPUT is not set")
    endif ()
    set(_objects)
    foreach (_src IN LISTS var_SOURCES)
        set(_obj "${CMAKE_CURRENT_BINARY_DIR}/${_src}.o")
        list(APPEND _objects "${_obj}")
        riscv_compile_source(
                "${NAME}"
                SOURCE "${CMAKE_CURRENT_LIST_DIR}/${_src}"
                OUTPUT "${_obj}"
                OPTIONS ${var_OPTIONS}
        )
    endforeach ()
    set(${var_OUTPUT} ${_objects} PARENT_SCOPE)
endfunction()

function(riscv_create_lib NAME)
    set(_options)
    set(_keys OUTPUT LIB_FILE)
    set(_lists SOURCES OPTIONS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    if (NOT DEFINED var_OUTPUT)
        message(FATAL_ERROR "OUTPUT is not set")
    endif ()
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/lib${var_OUTPUT}.a")

    add_custom_command(
            OUTPUT "${_output}"
            COMMENT "create lib ${NAME}"
            COMMAND rv_tools::_ar
            ARGS crs "${_output}" ${var_SOURCES}
            DEPENDS ${var_SOURCES}
    )
    set(${var_LIB_FILE} ${_output} PARENT_SCOPE)
endfunction()

function(riscv_link_exe NAME)
    set(_options)
    set(_keys OUTPUT LINK_SCRIPT)
    set(_lists SOURCES OPTIONS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    set(_command rv_tools::_gcc) # TODO: c++ support

    add_custom_command(
            OUTPUT "${var_OUTPUT}"
            COMMENT "link ${var_OUTPUT}"
            COMMAND ${_command}
            # note: order of args - objects, link params, output
            ARGS ${var_SOURCES} ${var_OPTIONS} -o "${var_OUTPUT}"
            DEPENDS ${var_SOURCES} ${var_LINK_SCRIPT}
    )
endfunction()

function(riscv_make_bin INPUT OUTPUT)
    message(DEBUG "${INPUT} : generate bin '${OUTPUT}'")
    add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}.bin"
            COMMAND rv_tools::_objcopy
            ARGS -O binary "${CMAKE_CURRENT_BINARY_DIR}/${INPUT}.elf" "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}.bin"
            COMMENT "${INPUT} : generate bin '${OUTPUT}'"
            DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${INPUT}.elf"
            )
endfunction()

function(riscv_make_hex INPUT OUTPUT)
    message(DEBUG "${INPUT} : generate hex '${OUTPUT}'")
    add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}.hex"
            COMMAND rv_tools::_objcopy
            ARGS -O ihex "${CMAKE_CURRENT_BINARY_DIR}/${INPUT}.elf" "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}.hex"
            COMMENT "${INPUT} : generate hex '${OUTPUT}'"
            DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${INPUT}.elf"
    )
endfunction()

function(riscv_add_library NAME)
    set(_options)
    set(_lists SOURCES HEADERS LIBS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    message(FATAL_ERROR "riscv_add_library: libraries is not supported") # FIXME: temporary disabled
    message(DEBUG "add lib: ${NAME}")
    message(DEBUG "${NAME} : LINK_SCRIPT = '${var_LINK_SCRIPT}'")
    message(DEBUG "${NAME} : SOURCES = '${var_SOURCES}'")
    message(DEBUG "${NAME} : HEADERS = '${var_HEADERS}'")

    set(_objects)
    set(_options -march=rv32im -mabi=ilp32 -nostdlib)
    riscv_compile(
            "${NAME}"
            OUTPUT _objects
            SOURCES ${var_SOURCES}
            OPTIONS ${_options}
    )
    riscv_create_lib(
            "${NAME}"
            OUTPUT "${NAME}"
            LIB_FILE _deps
            SOURCES ${_objects}
    )

    riscv_add_dummy("${NAME}" "c" "${_deps}")
endfunction()


function(riscv_add_executable NAME)
    set(_options BIN HEX)
    set(_keys LINK_SCRIPT)
    set(_lists SOURCES HEADERS LIBS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    message(DEBUG "add exe: ${NAME}")
    message(DEBUG "${NAME} : ${CMAKE_CURRENT_BINARY_DIR}")
    message(DEBUG "${NAME} : ${CMAKE_CURRENT_LIST_DIR}")
    message(DEBUG "${NAME} : LINK_SCRIPT = '${var_LINK_SCRIPT}'")
    message(DEBUG "${NAME} : SOURCES = '${var_SOURCES}'")
    message(DEBUG "${NAME} : HEADERS = '${var_HEADERS}'")
    message(DEBUG "${NAME} : BIN = '${var_BIN}'")
    message(DEBUG "${NAME} : HEX = '${var_HEX}'")

    set(_objects)
    set(_options -march=rv32im -mabi=ilp32 -nostdlib)

    riscv_compile(
            "${NAME}"
            OUTPUT _objects
            SOURCES ${var_SOURCES}
            OPTIONS ${_options}
    )

    set(_link_script)
    if (DEFINED var_LINK_SCRIPT)
        get_filename_component(_link_abs "${var_LINK_SCRIPT}" ABSOLUTE)
        set(_link_script -T "${_link_abs}")
    else ()
        message(FATAL_ERROR "${NAME} : LINK_SCRIPT is not set")
    endif ()

    riscv_link_exe(
            "${NAME}"
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.elf"
            LINK_SCRIPT "${_link_abs}"
            OPTIONS ${_options}
                ${_link_script}
            SOURCES ${_objects}
    )

    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.elf")
    if (var_BIN)
        riscv_make_bin("${NAME}" "${NAME}")
        list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.bin")
    endif ()
    if (var_HEX)
        riscv_make_hex("${NAME}" "${NAME}")
        list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.hex")
    endif ()

    riscv_add_dummy("${NAME}" "c" "${_deps}")
endfunction()
