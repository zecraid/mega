#include <iostream>
#include "src/server/WebServer.h"

int main(){
    WebServer * server = new WebServer();
    server->init(8888,0, 0, "localhost", 3306, "root", "123456", "webserver", 12, "./resources");
    server->start();
}