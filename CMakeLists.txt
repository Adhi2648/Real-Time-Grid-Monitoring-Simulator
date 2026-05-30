cmake_minimum_required(VERSION 3.24)
project(GridMonitoringSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto/grid_telemetry.proto)
set(GENERATED_PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_PROTO_DIR})

set(GENERATED_PROTO_SRCS ${GENERATED_PROTO_DIR}/grid_telemetry.pb.cc)
set(GENERATED_PROTO_HDRS ${GENERATED_PROTO_DIR}/grid_telemetry.pb.h)
set(GENERATED_GRPC_SRCS ${GENERATED_PROTO_DIR}/grid_telemetry.grpc.pb.cc)
set(GENERATED_GRPC_HDRS ${GENERATED_PROTO_DIR}/grid_telemetry.grpc.pb.h)

add_custom_command(
    OUTPUT ${GENERATED_PROTO_SRCS} ${GENERATED_PROTO_HDRS} ${GENERATED_GRPC_SRCS} ${GENERATED_GRPC_HDRS}
    COMMAND $<TARGET_FILE:protobuf::protoc>
    ARGS
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
        --cpp_out=${GENERATED_PROTO_DIR}
        --grpc_out=${GENERATED_PROTO_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating gRPC sources from grid_telemetry.proto"
    VERBATIM
)

add_library(grid_proto STATIC
    ${GENERATED_PROTO_SRCS}
    ${GENERATED_GRPC_SRCS}
)

target_include_directories(grid_proto
    PUBLIC
        ${GENERATED_PROTO_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(grid_proto
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
)

add_library(grid_common STATIC
    src/common/Env.cpp
)

target_include_directories(grid_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(grid_common PUBLIC Qt6::Core)

add_executable(grid_monitor_app
    src/client/main.cpp
    src/client/MainWindow.cpp
    src/client/TelemetryClient.cpp
    src/client/MainWindow.h
    src/core/DataProcessor.cpp
    src/core/DataProcessor.h
    src/db/DatabaseManager.cpp
    src/db/DatabaseManager.h
)

target_include_directories(grid_monitor_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(grid_monitor_app PRIVATE
    grid_common
    grid_proto
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

add_executable(grid_monitor_server
    src/server/main.cpp
    src/server/GridMonitoringService.cpp
    src/server/GridMonitoringService.h
)

target_include_directories(grid_monitor_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(grid_monitor_server PRIVATE grid_common grid_proto Qt6::Core)

add_executable(sensor_node_simulator
    src/sensor/main.cpp
)

target_include_directories(sensor_node_simulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(sensor_node_simulator PRIVATE grid_common grid_proto Qt6::Core)
