server:
	g++ main.cpp src/buffer/Buffer.cpp src/http/HttpRequest.cpp src/http/HttpResponse.cpp \
		src/log/Log.cpp src/pool/SqlConnectionPool.cpp \
		src/timer/HeapTimer.cpp src/server/Epoller.cpp src/server/HttpConnection.cpp \
		src/server/Util.cpp src/server/WebServer.cpp \
		 -o server -g -pthread -lmysqlclient

clean:
	-rm -rf server

.PHONY:clean ALL
