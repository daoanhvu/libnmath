# Nautilus 3D Modeling Tool

## Install libraries and build tools on MacOs
```bash
brew install cmake glew glfw glm
```

## Build Using Cmake
Build with tests
```bash
cmake -S . -B build -DTESTS=ON
cmake --build build
```
Build without tests
```bash
cmake -S . -B build -DTESTS=ON
cmake --build build
```

Run tests:
```bash
ctest --test-dir build
```