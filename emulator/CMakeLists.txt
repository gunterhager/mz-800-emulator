cmake_minimum_required (VERSION 3.1)
find_package(XCTest REQUIRED)

# select the sokol-gfx backend depending on platform
if(FIPS_EMSCRIPTEN OR FIPS_RASPBERRYPI OR FIPS_ANDROID)
    add_definitions(-DSOKOL_GLES2)
elseif(FIPS_WINDOWS)
    add_definitions(-DSOKOL_D3D11)
elseif(FIPS_OSX)
    add_definitions(-DSOKOL_METAL)
else()
    add_definitions(-DSOKOL_GLCORE33)
endif()

if (FIPS_WINDOWS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# a lib of common code shared between all emulators
fips_begin_lib(common)
    fips_vs_warning_level(3)
    fips_dir(common)
    fips_files(gfx.c gfx.h)
    fips_files(fs.c fs.h)
    fips_files(sound.c sound.h)
    fips_files(clock.c clock.h)
    fips_files(args.c args.h)
    if (FIPS_OSX)
        fips_files(sokol.m)
        if (FIPS_IOS)
            fips_frameworks_osx(UIKit Metal MetalKit AudioToolbox)
        else()
            fips_frameworks_osx(Cocoa QuartzCore Metal MetalKit AudioToolbox)
        endif()
    else()
        fips_files(sokol.c)
        if (FIPS_LINUX)
            fips_libs(X11 GL m dl)
        endif()
    endif()
fips_end_lib()

fips_begin_app(MZ-800-Emulator windowed)
    fips_vs_warning_level(3)
    fips_files(mz800.c mz800.h)
    fips_files(mzf.h)
    fips_files(gdg_whid65040_032.h)
    if (FIPS_IOS)
        fips_files(ios-info.plist)
    endif()
    fips_dir(roms)
    fips_generate(FROM mz800-roms.yml TYPE dump SOURCE mz800-roms.c HEADER mz800-roms.h)
    fips_deps(common)
fips_end_app()

# add unit tests using XCTest
if (FIPS_OSX AND OSX_XCTEST)
    xctest_add_bundle(MZ-800-Emulator-Tests MZ-800-Emulator 
    XCTests/GDGTests.m
    XCTests/assets/WriteCharacter.mzf
    )
    xctest_add_test(XCTest.MZ-800-Emulator MZ-800-Emulator-Tests)
    set(RESOURCE_FILES 
    XCTests/assets/WriteCharacter.mzf
    )
    set_target_properties(MZ-800-Emulator-Tests PROPERTIES
    RESOURCE "${RESOURCE_FILES}"
    )
endif()
