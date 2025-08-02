# Red Ball
This repository is a Red Ball game implemented in C++ using SFML and Box2D, built with CMake.

[![Build Status](https://github.com/xLab-HDU/RedBall/actions/workflows/ci.yml/badge.svg)](https://github.com/xLab-HDU/RedBall/actions?workflow=ci build)

## How to Use

```sh
# configure the project
cmake -B build

# build the project
cmake --build build

# test the project
cd build
ctest --output-on-failure

# run the project
./bin/main.exe

# package the project
cpack -G ZIP
# or
cpack -G NSIS
```

## License

The source code is dual licensed under Public Domain and MIT -- choose whichever you prefer.
