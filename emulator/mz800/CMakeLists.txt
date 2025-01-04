find_package(XCTest REQUIRED)

fips_begin_app(MZ-800-Emulator windowed)
#    
    fips_files(mz800.c mz800.h ui_mz800.h)
    fips_files(mzf.h)
    fips_files(gdg_whid65040_032.h ui_gdg_whid65040_032.h)
    if (FIPS_IOS)
        fips_files(ios-info.plist)
    endif()
    fips_deps(roms common ui)
fips_end_app()
target_compile_definitions(MZ-800-Emulator PRIVATE CHIPS_USE_UI)

# add unit tests using XCTest (this does work currently)
if (FIPS_OSX AND OSX_XCTEST)
    set(RESOURCE_FILES
    XCTests/assets/TestHalt.mzf
    XCTests/assets/TestBorder.mzf
    XCTests/assets/TestCharacters.mzf
    XCTests/assets/TestMZ700Color.mzf
    XCTests/assets/TestMZ700VRAM.mzf
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
