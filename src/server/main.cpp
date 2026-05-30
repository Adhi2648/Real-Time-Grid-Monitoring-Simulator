#include <iostream>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "GridMonitoringService.h"
#include "../common/Env.h"

int main()
{
    Env::loadDotEnv();

    const std::string serverAddress = Env::value(QStringLiteral("GRID_SERVER_ADDRESS"), QStringLiteral("0.0.0.0:50051")).toStdString();

    GridMonitoringService service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    if (!server) {
        std::cerr << "Failed to start gRPC server on " << serverAddress << std::endl;
        return 1;
    }

    std::cout << "gRPC server listening on " << serverAddress << std::endl;
    server->Wait();
    return 0;
}
