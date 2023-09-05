#include "Log.h"

int main(){
    Log::Instance()->init(0, "./log", ".log", 0);
    LOG_DEBUG("test debug\n");
    LOG_INFO("test info\n");
    LOG_WARN("test warn\n");
    LOG_ERROR("test error\n");
}