# tools for compile programs

function(find_riscv_toolchain)
    set(TOOL_PREFIX rv_tools)
    set(_prefix "riscv64-unknown-elf-")
    set(_c "${_prefix}gcc")
    set(_cxx "${_prefix}g++")
    set(_asm "${_prefix}as")
    set(_ar "${_prefix}ar")
    set(_linker "${_prefix}ld")
    set(_objcopy "${_prefix}objcopy")
    set(_objdump "${_prefix}objdump")

    set(_var_prefix "rv_tool")

    set(_var_names
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
endfunction()

function(riscv_compile_source NAME)
    set(_options)
    set(_keys SOURCE OUTPUT)
    set(_lists OPTIONS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})
endfunction()

function(riscv_create_lib NAME)
    set(_options)
    set(_keys OUTPUT)
    set(_lists SOURCES OPTIONS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})
endfunction()

function(riscv_link_exe NAME)
    set(_options)
    set(_keys OUTPUT)
    set(_lists SOURCES OPTIONS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})
endfunction()

function(riscv_make_bin INPUT OUTPUT)
    message(STATUS "${INPUT} : generate bin '${OUTPUT}'")
    add_custom_command(
            TARGET "${INPUT}"
            COMMAND rv_tools::_objcopy
            ARGS -O binary "${INPUT}.elf" "${OUTPUT}.bin"
            COMMENT "${INPUT} : generate bin '${OUTPUT}'"
            DEPENDS "${INPUT}.elf"
            )
endfunction()

function(riscv_make_hex INPUT OUTPUT)
    add_custom_command(
            TARGET "${INPUT}"
            COMMAND rv_tools::_objcopy
            ARGS -O ihex "${INPUT}.elf" "${OUTPUT}.bin"
            COMMENT "${INPUT} : generate hex '${OUTPUT}'"
            DEPENDS "${INPUT}.elf"
    )
endfunction()

function(riscv_add_library NAME)
    set(_options)
    set(_keys LINK_SCRIPT)
    set(_lists SOURCES HEADERS LIBS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    message(STATUS "add lib: ${NAME}")
    message(STATUS "${NAME} : LINK_SCRIPT = '${var_LINK_SCRIPT}'")
    message(STATUS "${NAME} : SOURCES = '${var_SOURCES}'")
    message(STATUS "${NAME} : HEADERS = '${var_HEADERS}'")
endfunction()


function(riscv_add_executable NAME)
    set(_options BIN HEX)
    set(_keys LINK_SCRIPT)
    set(_lists SOURCES HEADERS LIBS)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    message(STATUS "add exe: ${NAME}")
    message(STATUS "${NAME} : ${CMAKE_CURRENT_BINARY_DIR}")
    message(STATUS "${NAME} : ${CMAKE_CURRENT_LIST_DIR}")
    message(STATUS "${NAME} : LINK_SCRIPT = '${var_LINK_SCRIPT}'")
    message(STATUS "${NAME} : SOURCES = '${var_SOURCES}'")
    message(STATUS "${NAME} : HEADERS = '${var_HEADERS}'")
    message(STATUS "${NAME} : BIN = '${var_BIN}'")
    message(STATUS "${NAME} : HEX = '${var_HEX}'")

    add_custom_target("${NAME}")

    if (var_BIN)
        riscv_make_bin("${NAME}" "${NAME}")
    endif ()
    if (var_HEX)
        riscv_make_hex("${NAME}" "${NAME}")
    endif ()
endfunction()
