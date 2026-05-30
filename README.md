# Real-Time Grid Monitoring Simulator

This repo contains a full C++/Qt/gRPC project for a power-grid monitoring simulator. It is set up as a three-part system:

- `grid_monitor_server`: central gRPC server that receives sensor telemetry and streams it to subscribers
- `sensor_node_simulator`: gRPC sensor client that generates telemetry and pushes it to the server
- `grid_monitor_app`: Qt dashboard that subscribes to the live stream and shows processed telemetry

The project also includes local `.env` support so database credentials and runtime settings stay out of GitHub.

## Stack

- C++20
- Qt 6 Widgets and Qt SQL
- gRPC and Protobuf
- Multi-threaded telemetry processing
- SQL Server through ODBC

## Layout

- `proto/` contains the telemetry service contract
- `src/common/` contains shared telemetry models and `.env` loading
- `src/server/` hosts the gRPC server
- `src/sensor/` contains the simulated PMU client
- `src/client/` contains the Qt dashboard and gRPC subscriber
- `src/core/` contains the background processing queue
- `src/db/` contains the SQL Server wrapper

## Local config

Copy `.env.example` to `.env` and edit values as needed. The `.env` file is ignored by Git.

## Build notes

The project is wired for CMake, Qt, gRPC, and Protobuf. The generated proto code is expected in the build tree, so your local environment needs the matching toolchain packages if you want to compile it.
