include(FindPackageHandleStandardArgs)

set(HIP_ROOT_DIR "${HIP_ROOT_DIR}" CACHE PATH "HIP SDK root")

set(_HIP_HINTS)

foreach(v HIP_ROOT_DIR HIP_PATH ROCM_PATH)
    if(DEFINED ${v} AND NOT "${${v}}" STREQUAL "")
        list(APPEND _HIP_HINTS "${${v}}")
    endif()

    if(DEFINED ENV{${v}} AND NOT "$ENV{${v}}" STREQUAL "")
        list(APPEND _HIP_HINTS "$ENV{${v}}")
    endif()
endforeach()

if(WIN32)
    list(APPEND _HIP_HINTS
        "C:/Program Files/AMD/ROCm"
        "C:/Program Files/AMD/HIP"
        "C:/opt/rocm"
        "C:/ROCm"
        "/opt/rocm"
    )
else()
    list(APPEND _HIP_HINTS /opt/rocm)
endif()

list(REMOVE_DUPLICATES _HIP_HINTS)

find_path(HIP_INCLUDE_DIR
    NAMES hip/hip_runtime_api.h
    HINTS ${_HIP_HINTS}
    PATH_SUFFIXES include
)

find_library(HIP_RUNTIME_LIBRARY
    NAMES amdhip64
    HINTS ${_HIP_HINTS}
    PATH_SUFFIXES lib lib64 bin
)

find_program(HIP_CLANG_COMPILER
    NAMES
        hipcc
    HINTS ${_HIP_HINTS}
    PATH_SUFFIXES
        bin
        llvm/bin
)

find_package_handle_standard_args(HIP
    REQUIRED_VARS
        HIP_INCLUDE_DIR
        HIP_RUNTIME_LIBRARY
        HIP_CLANG_COMPILER
)

if(HIP_FOUND)

    set(HIP_LIBRARIES "${HIP_RUNTIME_LIBRARY}")

    if(NOT TARGET HIP::runtime)
        add_library(HIP::runtime UNKNOWN IMPORTED)

        set_target_properties(HIP::runtime PROPERTIES
            IMPORTED_LOCATION "${HIP_RUNTIME_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${HIP_INCLUDE_DIR}"
            INTERFACE_COMPILE_DEFINITIONS
                __HIP_PLATFORM_AMD__=1
        )
    endif()
        message(STATUS "HIP INCLUDES: ${HIP_INCLUDE_DIR}")
        message(STATUS "HIP LIBRARY: ${HIP_RUNTIME_LIBRARY}")
        message(STATUS "HIP COMPILER: ${HIP_CLANG_COMPILER}")
else()
    message(FATAL_ERROR "HIP Not found")
endif()

mark_as_advanced(
    HIP_INCLUDE_DIR
    HIP_RUNTIME_LIBRARY
    HIP_CLANG_COMPILER
)