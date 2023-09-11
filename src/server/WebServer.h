#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Epoller.h"
#include "../timer/HeapTimer.h"
#include "../log/Log.h"
#include "../pool/SqlConnectionPool.h"
#include "../pool/ThreadPool.h"
#include "../http/"

class WebServer {

};


#endif //!__WEBSERVER_H__
