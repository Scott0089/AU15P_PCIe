# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.2)

# Project name and language
project(AU15P_PCIe LANGUAGES C)

# Add Definitions

add_definitions(-DSDT)
add_definitions(-U__linux__)

# Set build type if not provided
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -O2 -fPIC")

# Add main executables source files
set(MAIN_SRCS
    main.c
)

file(GLOB SC_LIBS_SRCS
    sc_libs/*.c
)

# Link Subdirectory libraries
add_subdirectory(xlnx_libs)

# Add directories to the include path
include_directories(
    sc_libs
    xlnx_libs/gpio
    xlnx_libs/standalone
    xlnx_libs/hw_config
    xlnx_libs/tmrctr
    xlnx_libs/sysmon
    xlnx_libs/v_tpg
    xlnx_libs/video_common
)

# add_library(xdma STATIC ${XDMA_SRCS})
add_library(libsc_libs STATIC ${SC_LIBS_SRCS})

target_link_libraries(libsc_libs xlnx_libs m)

# Loop over main source files and create executables
foreach(MAIN_SRC ${MAIN_SRCS})
    # Extract the executable name from the source file
    get_filename_component(EXE_NAME ${MAIN_SRC} NAME_WE)

    # Add executable target
    add_executable(${EXE_NAME} ${MAIN_SRC})

    # Link libraries to the executable
    target_link_libraries(${EXE_NAME} xlnx_libs m pthread libsc_libs)

    # Set output directories for the executable
    set_target_properties(${EXE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endforeach()

# Add custom clean target to remove build files
add_custom_target(distclean COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/clean.cmake)