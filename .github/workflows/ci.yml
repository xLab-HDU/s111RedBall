name: CI Build

on: [push, pull_request]

defaults:
  run:
    shell: bash

jobs:
  build:
    name: ${{ matrix.platform.name }} ${{ matrix.config.name }}
    runs-on: ${{ matrix.platform.os }}

    strategy:
      fail-fast: false
      matrix:
        platform:
        - { name: Windows MinGW, os: windows-2022, msystem: 'MINGW64' }
        - { name: Linux GCC,      os: ubuntu-latest }
        - { name: Linux Clang,    os: ubuntu-latest, flags: -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ }
        - { name: macOS,          os: macos-latest }
        config:
        - { name: Shared, flags: -DBUILD_SHARED_LIBS=TRUE }
        - { name: Static, flags: -DBUILD_SHARED_LIBS=FALSE }

    steps:
    - uses: actions/checkout@v4

    - name: Setup MinGW via MSYS2
      if: matrix.platform.os == 'windows-2022'
      uses: msys2/setup-msys2@v2
      with:
        msystem: ${{ matrix.platform.msystem }}
        update: true
        install: >
          mingw-w64-x86_64-gcc
          mingw-w64-x86_64-cmake
          mingw-w64-x86_64-ninja
    - name: Install Linux Dependencies
      if: runner.os == 'Linux'
      run: sudo apt-get update && sudo apt-get install libxrandr-dev libxcursor-dev libxi-dev libudev-dev libflac-dev libvorbis-dev libgl1-mesa-dev libegl1-mesa-dev libfreetype-dev

    - name: Configure
      run: |
        cmake -B build ${{ matrix.platform.flags }} ${{ matrix.config.flags }} -G Ninja
        
    - name: Build
      run: cmake --build build --config Release
