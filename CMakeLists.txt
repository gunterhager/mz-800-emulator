#
# project: MZ-800-Emulator
#
cmake_minimum_required(VERSION 3.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
project(MZ-800-Emulator)

# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

fips_setup()
fips_import_fips_imgui_imgui()

# just suppress this pesky "missing field initializer warning" for now
if (FIPS_CLANG OR FIPS_GCC)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")
endif()

if (NOT FIPS_EMSCRIPTEN AND NOT FIPS_ANDROID)
    fips_ide_group(tests)
    add_subdirectory(tests)
endif()

fips_ide_group(emulator)
fips_add_subdirectory(emulator)
fips_ide_group(tools)
add_subdirectory(tools)
fips_finish()
