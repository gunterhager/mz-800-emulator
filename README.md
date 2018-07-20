![MZ-800](misc/cgrom_dump.png)

# mz-800-emulator

SHARP MZ-800 Emulator using Andre Weissflog's https://github.com/floooh/chips emulator infrastructure.

The emulator doesn't come with ROMs included since I think they're still copyrighted material.

NOTE: This project has just been started, so don't expect anything useful yet.

Build and run (exact versions of tools don't matter):

```bash
> python --version
Python 2.7.10
> cmake --version
cmake version 3.10.0
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
