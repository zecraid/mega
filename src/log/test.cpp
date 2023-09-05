#include "Log.h"

int main(){
    Log::Instance()->init(0, "./log", ".log", 0);
    LOG_DEBUG("test debug");
    LOG_INFO("test info");
    LOG_WARN("test warn");
    LOG_ERROR("test error");
}