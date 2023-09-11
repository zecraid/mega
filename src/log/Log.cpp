#include "Log.h"

// 构造函数
Log::Log() {
    fp_ = nullptr;
    deque_ = nullptr;
    writeThread_ = nullptr;
    lineCount_ = 0;
    toDay_ = 0;
    isAsync_ = false;
}

Log::~Log() {
    if(isAsync_){
        while (!deque_->empty()){
            deque_->flush(); // 唤醒消费者，处理掉剩下的任务
        }
        deque_->close(); // 关闭队列
        writeThread_->join(); // 等待当前线程完成手中的任务
        if(fp_){
            std::lock_guard<std::mutex> locker(mtx_);
            flush(); //清空缓冲区中的数据
            fclose(fp_);
        }
    }
}

void Log::flush() {
    if(isAsync_){ // 只有异步日志才会用到deque
        deque_->flush();
    }
    fflush(fp_); // 清空输入缓冲区
}

// 懒汉模式 局部静态变量法（这种方法不需要加锁和解锁操作）
Log* Log::Instance() {
    static Log log;
    return &log;
}

// 异步日志的写线程函数
void Log::flushLogThread() {
    Log::Instance()->asyncWrite_();
}

// 写线程真正的执行函数
void Log::asyncWrite_() {
    std::string str = "";
    while (deque_->pop(str)){
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

void Log::init(int level, const char *path, const char *suffix, int maxQueueCapacity) {
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;
    if(maxQueueCapacity){ // 采用异步方式记录日志
        isAsync_ = true;
        if(!deque_){ // 如果队列不存在就创建一个
            std::unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>);
            deque_ = std::move(newQueue);  // 左值变右值,掏空newDeque

            // 创建写日志线程，执行flushLogThread函数
            std::unique_ptr<std::thread> newThread(new std::thread(flushLogThread));
            writeThread_ = std::move(newThread);
        }
    } else {
        isAsync_ = false;
    }
    lineCount_ = 0;

    // 获取日志文件存储路径：fileName = ./log/2023_07_27.log
    time_t timer = time(nullptr);
    struct tm * sysTime = localtime(&timer);
    struct tm t = * sysTime;
    char fileName[LOG_NAME_LEN];
    memset(fileName,0,sizeof(fileName));
    snprintf(fileName,LOG_NAME_LEN - 1,"%s/%04d_%02d_%02d%s",
             path_,t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        std::lock_guard<std::mutex> locker(mtx_);
        buff_.retrieveAll();
        if(fp_){ // 重新打开
            flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "r"); //打开文件读取并Append写入
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "r");
        }
        // 计算文件行数
        if(fp_ != nullptr){
            char ch;
            while ((ch = fgetc(fp_)) != EOF) {
                if (ch == '\n') {
                    lineCount_++;
                }
            }
            if (ch != '\n' && lineCount_ > 0) {
                lineCount_++;
            }
            fclose(fp_);
        } else {
            lineCount_ = 0;
        }
        fp_ = fopen(fileName,"a");
        assert(fp_ != nullptr);
    }
}

void Log::write(int level, const char * file, int line, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm * sysTime = localtime(&tSec);
    struct tm t = * sysTime;
    va_list vaList;

    // 日志日期  日志行数  如果不是今天或者行数超了
    if(toDay_ != t.tm_mday || (lineCount_ && (lineCount_ >= MAX_LINES))){
        std::unique_lock<std::mutex> locker(mtx_);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if(toDay_ != t.tm_mday) //时间不匹配，则替换为最新的日志文件名
        {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }else{ // 行数超过了,newFile = ./log/2023_07_27-1.log
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_  / MAX_LINES), suffix_);
            lineCount_ = 0;
        }

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }

    // 在buffer内生成一条对应的日志信息
    {
        std::unique_lock<std::mutex> locker(mtx_);
        lineCount_ ++;

        int n = snprintf(buff_.beginWritePtr(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff_.moveWritePos(n);

        appendLogLevelTitle_(level);

        int j = snprintf(buff_.beginWritePtr(), 128, "[%s:%d]", file, line);
        buff_.moveWritePos(j);

        va_start(vaList, format);
        int m = vsnprintf(buff_.beginWritePtr(), buff_.writableBytes(), format, vaList);
        va_end(vaList);

        buff_.moveWritePos(m);
        buff_.append("\n\0", 2);

        if(isAsync_ && deque_ && !deque_->full()){ // 异步方式（加入阻塞队列中，等待写线程读取日志信息）
            deque_->push_back(buff_.retrieveAllToStr());
        } else { // 同步方式
            std::string log_line(buff_.peek(), buff_.readableBytes());
            printf("%s",log_line.c_str());
            fputs(buff_.peek(), fp_); // 同步就直接写入文件
        }
        buff_.retrieveAll();
    }
}

void Log::appendLogLevelTitle_(int level) {
    switch(level) {
        case 0:
            buff_.append("[DEBUG]", 7);
            break;
        case 1:
            buff_.append("[INFO] ", 7);
            break;
        case 2:
            buff_.append("[WARN] ", 7);
            break;
        case 3:
            buff_.append("[ERROR]", 7);
            break;
        default:
            buff_.append("[INFO] ", 7);
            break;
    }
}

int Log::getLevel() {
    std::lock_guard<std::mutex> locker(mtx_);
    return level_;
}

void Log::setLevel(int level) {
    std::lock_guard<std::mutex> locker(mtx_);
    level_ = level;
}

const char *Log::getLevelStr() const {
    switch (level_) {
        case 0: return "DEBUG";
        case 1: return "INFO";
        case 2: return "WARN";
        case 3: return "ERROR";
        default: return "UNKNOWN";
    }
}