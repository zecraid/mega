#!/bin/bash

# 启动 Web 服务器，并将输出重定向到日志文件
nohup ./server > server.log 2>&1 &

# 记录服务器的进程ID（PID）到文件中，以便稍后关闭
echo $! > server.pid

echo "Web服务器已启动。PID: $!"
