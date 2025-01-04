fips_begin_lib(roms)
    fipsutil_embed(mz800-roms.yml mz800-roms.h)
    fips_files(dummy.c)
fips_end_lib()
