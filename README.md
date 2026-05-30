# Real-Time Grid Monitoring Simulator

CMakeLists.txt is required for this project. It defines the Qt, gRPC, and Protobuf targets, generates the proto code, and builds the three executables.

This repo is set up as a three-part system:

- `grid_monitor_server`: central gRPC server that receives sensor telemetry and streams it to subscribers
- `sensor_node_simulator`: gRPC sensor client that generates telemetry and pushes it to the server
- `grid_monitor_app`: Qt dashboard that subscribes to the live stream and shows processed telemetry

The project also includes local `.env` support so database credentials and runtime settings stay out of GitHub.

## What you need to install

- Visual Studio Build Tools with the Desktop development with C++ workload
- CMake
- Qt 6 x64 for MSVC
- Protobuf and gRPC, or vcpkg if you want the easiest path on Windows
- SQL Server ODBC Driver 18
- SQL Server Developer or Express if you want the database logging path to connect locally

## Recommended setup on Windows

1. Install Visual Studio Build Tools and include the C++ workload.
2. Install Qt 6 and make sure the MSVC x64 kit matches the compiler you installed.
3. Install CMake.
4. Install gRPC and Protobuf. If you use vcpkg, install it once and then add the needed packages from there.
5. Install the Microsoft ODBC Driver 18 for SQL Server.
6. Clone this repository.
7. Copy `.env.example` to `.env` and set your local values.
8. Configure the project with CMake Tools in VS Code, or run CMake manually with the right Qt and package paths.

If you are using vcpkg, the common package install step looks like this:

```powershell
vcpkg install grpc protobuf
```

## Local config

Copy `.env.example` to `.env` and edit the values for your machine.

## How to build and run

If you are using vcpkg, configure with your toolchain file. Example:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\src\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build
```

If you are using VS Code, pick the MSVC kit in CMake Tools and run `CMake: Configure`, then `CMake: Build`.

Run the targets in this order:

1. `grid_monitor_server`
2. `sensor_node_simulator`
3. `grid_monitor_app`

## Project layout

- `proto/` contains the telemetry service contract
- `src/common/` contains shared telemetry models and `.env` loading
- `src/server/` hosts the gRPC server
- `src/sensor/` contains the simulated PMU client
- `src/client/` contains the Qt dashboard and gRPC subscriber
- `src/core/` contains the background processing queue
- `src/db/` contains the SQL Server wrapper
