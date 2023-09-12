CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -g

SERVER_OBJS = main.cpp src/buffer/Buffer.cpp src/http/HttpRequest.cpp src/http/HttpResponse.cpp \
              		src/log/Log.cpp src/pool/SqlConnectionPool.cpp \
              		src/timer/HeapTimer.cpp src/server/Epoller.cpp src/server/HttpConnection.cpp \
              		src/server/Util.cpp src/server/WebServer.cpp

server: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $(SERVER_OBJS) -o server -pthread -lmysqlclient

clean:
	rm -rf ./server

.PHONY:clean ALL

