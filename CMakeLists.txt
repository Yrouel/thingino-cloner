cmake_minimum_required(VERSION 3.10)
project(thingino-cloner VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

# Include directories
include_directories(${LIBUSB_INCLUDE_DIRS})
include_directories(include)

# Compiler flags
add_compile_options(-Wall -Wextra -Werror)
add_compile_options(${LIBUSB_CFLAGS_OTHER})

# Source files
set(SOURCES
    src/main.c
    src/usb/manager.c
    src/usb/device.c
    src/usb/protocol.c
    src/firmware/loader.c
    src/firmware/reader.c
    src/firmware/handshake.c
    src/firmware/flash_descriptor.c
    src/ddr/parser.c
    src/utils.c
    src/bootstrap.c
)

# Create executable
add_executable(thingino-cloner ${SOURCES})

# Link libraries
target_link_libraries(thingino-cloner ${LIBUSB_LIBRARIES})

# Installation
install(TARGETS thingino-cloner DESTINATION bin)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(thingino-cloner PRIVATE _WIN32)
elseif(APPLE)
    # macOS-specific settings
    target_compile_definitions(thingino-cloner PRIVATE __APPLE__)
else()
    # Linux-specific settings
    target_compile_definitions(thingino-cloner PRIVATE __linux__)
endif()