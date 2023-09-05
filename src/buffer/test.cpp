#include "Buffer.h"

int main(){
    Buffer buff;
    buff.append("hello");
    buff.append("world");
    printf("%s",buff.retrieveAllToStr().c_str());
    return 0;
}