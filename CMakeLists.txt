cmake_minimum_required(VERSION 3.7)

project(SpaceScanner)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/avr-gcc.toolchain.cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(BINARY_NAME ${CMAKE_PROJECT_NAME})


include(assert_out_of_source_build)
assert_out_of_source_build(${CMAKE_PROJECT_NAME})

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Valid options are Debug (default) and Coverage.")


if(CMAKE_BUILD_TYPE MATCHES Release)
   set(CMAKE_C_FLAGS_RELEASE "-O3")
   set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(CMAKE_BUILD_TYPE MATCHES Release)

if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
   set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -save-temps -g -gdwarf-3 -gstrict-dwarf")
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

if(CMAKE_BUILD_TYPE MATCHES Debug)
   set(CMAKE_C_FLAGS_DEBUG "-O1 -save-temps -g -gdwarf-3 -gstrict-dwarf")
   set(CMAKE_CXX_FLAGS_DEBUG "-O1 -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES Debug)

add_definitions("-std=c11")
add_definitions("-fpack-struct")
add_definitions("-fshort-enums")
add_definitions("-Wall")
#add_definitions("-Werror")
add_definitions("-pedantic")
#add_definitions("-pedantic-errors")
add_definitions("-funsigned-char")
add_definitions("-funsigned-bitfields")
add_definitions("-ffunction-sections")
add_definitions("-fdiagnostics-color=auto")

# Debug should build firmware, Coverage builds tests

# set(AVR_UPLOADTOOL avrdude)
# set(AVR_PROGRAMMER dragon_isp)
set(AVR_PROGRAMMER atmelice_isp)
# set(AVR_PROGRAMMER arduino)
#set(AVR_PROGRAMMER usbtiny)
# set(AVR_UPLOADTOOL_PORT /dev/ttyUSB0)
set(AVR_UPLOADTOOL_PORT "usb")
# set(AVR_UPLOADTOOL_OPTIONS "-b 57600")
set(AVR_UPLOADTOOL_OPTIONS "-B 4MHz")

set(AVR_MCU atmega328p)
set(AVR_L_FUSE 0xFF)
set(AVR_H_FUSE 0xDA)
set(AVR_E_FUSE 0xFD)

set(MCU_SPEED 16000000)
#link_directories("/usr/avr/lib")

# Ugly, fix.
set (CMAKE_EXE_LINKER_FLAGS
    "-Wl,-L/usr/avr/lib -Wl,-T/usr/avr/lib/ldscripts/avr5.xn -Wl,-u,vfprintf -lprintf_flt")

include(avr-gcc.toolchain)


message(STATUS "Current uploadtool is: ${AVR_UPLOADTOOL}")
message(STATUS "Current programmer is: ${AVR_PROGRAMMER}")
message(STATUS "Current upload port is: ${AVR_UPLOADTOOL_PORT}")
message(STATUS "Current uploadtool options are: ${AVR_UPLOADTOOL_OPTIONS}")
message(STATUS "Current MCU is set to: ${AVR_MCU}")
message(STATUS "Current H_FUSE is set to: ${AVR_H_FUSE}")
message(STATUS "Current L_FUSE is set to: ${AVR_L_FUSE}")


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_definitions("-DF_CPU=${MCU_SPEED}")

# Add all the sub-builds
add_subdirectory(src)

# Build the main executable
#add_avr_executable(${BINARY_NAME}
#    $<TARGET_OBJECTS:main>
#    $<TARGET_OBJECTS:firmware>
#)
