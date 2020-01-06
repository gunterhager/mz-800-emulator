cmake_minimum_required (VERSION 3.1)
find_package(XCTest REQUIRED)

fips_begin_app(MZ-800-Emulator windowed)
    fips_vs_warning_level(3)
    fips_files(mz800.c mz800.h)
    fips_files(mzf.h)
    fips_files(gdg_whid65040_032.h)
    if (FIPS_IOS)
        fips_files(ios-info.plist)
    endif()
    fips_deps(roms common)
fips_end_app()

# add unit tests using XCTest
if (FIPS_OSX AND OSX_XCTEST)
    set(RESOURCE_FILES
    XCTests/assets/TestHalt.mzf
    XCTests/assets/TestBorder.mzf
    XCTests/assets/TestCharacters.mzf
    )
    xctest_add_bundle(MZ-800-Emulator-Tests MZ-800-Emulator
    XCTests/GDGTests.m
    XCTests/MZFTests.m
    XCTests/MZFile.m
    XCTests/MZFile.h
    ${RESOURCE_FILES}
    )
    xctest_add_test(XCTest.MZ-800-Emulator MZ-800-Emulator-Tests)
    set_target_properties(MZ-800-Emulator-Tests PROPERTIES
    RESOURCE "${RESOURCE_FILES}"
    )
endif()
