#!/bin/bash

# 检查是否存在 PID 文件
if [ -e server.pid ]; then
    # 从PID文件中读取服务器的进程ID
    pid=$(cat server.pid)

    # 尝试关闭服务器进程
    kill $pid

    # 检查是否成功杀死进程
    if [ $? -eq 0 ]; then
        echo "Web服务器已关闭。PID: $pid"
    else
        echo "无法关闭Web服务器。"
    fi

    # 删除PID文件
    rm server.pid
else
    echo "PID文件不存在，Web服务器可能未启动。"
fi
