#include <iostream>
#include <algorithm>
#include <memory>
#include "src/server/WebServer.h"

int main(){
    std::unique_ptr<WebServer> server = std::make_unique<WebServer>("0.0.0.0",8888);
    server->init(0, 0, "localhost", 3306, "root", "123456", "webserver", 12, "./resources");
    server->start();
}