cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME MDE25)


# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Core/status.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Core/uart_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Core/pwm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Core/sense.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Core/control.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Core/telemetry.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Core/CAN/can_server.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Download can_ids.h and can_parser_types.h header files
file(DOWNLOAD
    "https://raw.githubusercontent.com/ZeniteSolar/CAN_IDS/refs/heads/master/can_ids.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Core/CAN/can_ids.h"
    SHOW_PROGRESS
    TLS_VERIFY ON
)

file(DOWNLOAD
    "https://raw.githubusercontent.com/ZeniteSolar/CAN_IDS/refs/heads/master/can_parser_types.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Core/CAN/can_parser_types.h"
    SHOW_PROGRESS
    TLS_VERIFY ON
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
