fips_begin_lib(roms)
    fips_generate(FROM mz800-roms.yml TYPE dump HEADER mz800-roms.h)
    fips_files(dummy.c)
fips_end_lib()
