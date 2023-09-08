server:
	g++ main.cpp src/buffer/Buffer.cpp src/http/HttpRequest.cpp src/http/HttpResponse.cpp \
		src/log/Log.cpp src/pool/ThreadPool.cpp src/pool/SqlConnectionPool.cpp \
		src/timer/HeapTimer.cpp src/server/Acceptor.cpp src/server/Channel.cpp \
		src/server/Epoll.cpp src/server/EventLoop.cpp src/server/HttpConnection.pp \
		src/server/Socket.cpp src/server/Util.cpp -o server -g -pthread -lmysqlclient

clean:
	-rm -rf server

.PHONY:clean ALL
