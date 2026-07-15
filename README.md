# HIP Host-Runtime Memory Allocator

This is a host-only HIP runtime test. It compiles with the platform's native C++ compiler (GCC's `g++` frontend on Linux and Microsoft `cl.exe` on Windows) and never enables the HIP language or invokes `hipcc`.

The executable initializes HIP, selects a device (device `0` by default), then calls `hipMalloc` for 1 GiB blocks until the runtime reports an error. It reports the HIP error name, numeric code, description, successful allocation count, and aggregate allocated memory. Every successfully allocated block is released before exit.

## Prerequisites

- A working AMD GPU driver and HIP runtime installation.
- CMake 3.16 or newer.
- GCC on Linux or the Visual Studio C++ build tools on Windows.
- HIP SDL 6.4.2 and 7.1.1

The included [cmake/FindHIP.cmake](cmake/FindHIP.cmake) locates `hip/hip_runtime_api.h`. the `amdhip64` host runtime library and `hipcc` compiler. It searches `HIP_ROOT_DIR`, `HIP_PATH`, and `ROCM_PATH` (cache variables or environment variables), plus standard ROCm locations.

## Configure and build

### Linux

If HIP is installed in `/opt/rocm`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DHIP_ROOT_DIR=/opt/rocm
cmake --build build
```

The finder also discovers versioned per-user installations such as
`$HOME/rocm/7.14.0`. For that layout, either configure without an explicit
HIP root or point CMake at the parent directory:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DHIP_ROOT_DIR="/opt/rocm/7.1" -G "Ninja"
cmake --build build
```

### Windows

Important: Use Ninja as generator.

```bat
cmake -S . -B build -G "Ninja" -DHIP_ROOT_DIR="C:/opt/rocm/7.1" 
cmake --build build --config Release
```

## Run

The optional argument is the HIP device ID; the default is `0`.

```sh
./build/hip_mem_alloc
./build/hip_mem_alloc 1
```

For a multi-configuration Windows build:

```bat
build\Release\hip_mem_alloc.exe 0
```

Out-of-memory is the expected completion condition and returns success after cleanup. Other HIP allocation errors or failures while freeing memory return a non-zero exit code.