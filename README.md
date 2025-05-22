# bsoncxx precompiled shared library
This project holds the precompiled bsoncxx lib (compiled on a linux machine with clangd and cmake).
Then, the created shared object was extracted along with the header files from the source project.

## Build

### Meson

Add the following `bsoncxx.wrap` to your `subprojects` directory:
```
[wrap-git]
url = git@github.com:makredzic/bsoncxx.git
revision = HEAD
```

In the **project root** meson build file, use `dependency('bsoncxx')` to fetch the `bsoncxx` dependency.
