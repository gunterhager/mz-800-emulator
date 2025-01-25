**ATTENTION:** This project is being sunset and will no longer be maintained. The MZ-800 emulator project is being moved over to:
https://github.com/gunterhager/mz800-emuz
The new project is using the zig language which is IMHO a much better choice than C for implementing emulators.

![MZ-800](misc/cgrom_dump.png)

# mz-800-emulator

SHARP MZ-800 Emulator using Andre Weissflog's https://github.com/floooh/chips emulator infrastructure.

**NOTE:** This project is work in progress, so don't expect anything very useful yet. It doesn't even boot into the monitor. However certain test programs can run successfully. A couple of them are included in the project.

You can drop MZF files onto the emulator window to load and run them.

Build and run (exact versions of tools don't matter):

```bash
> python --version
Python 2.*
> cmake --version
cmake version 3.*
> ./fips build
...
> ./fips list targets
...
> ./fips run [target]
...
```

To get optimized builds for performance testing:

```bash
# on OSX:
> ./fips set config osx-make-release
> ./fips build
> ./fips run [target]

# on Linux
> ./fips set config linux-make-release
> ./fips build
> ./fips run [target]

# on Windows
> fips set config win64-vstudio-release
> fips build
> fips run [target]
```

To open project in IDE:
```bash
# on OSX with Xcode:
> ./fips set config osx-xcode-debug
> ./fips gen
> ./fips open

# on Windows with Visual Studio:
> ./fips set config win64-vstudio-debug
> ./fips gen
> ./fips open

# experimental VSCode support on Win/OSX/Linux:
> ./fips set config [linux|osx|win64]-vscode-debug
> ./fips gen
> ./fips open
```

