#include "HttpRequest.h"

// 网页名称，和一般的前端跳转不同，这里需要将请求信息放到后端来验证一遍再上传
const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML {
        "/index", "/register", "/login", "/welcome", "/video", "/picture", "/loginfail", "/registerfail", "/test"
};

// 登录/注册
const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
        {"/login.html", 1}, {"/register.html", 0}
};

// 初始化操作，一些清零操作
void HttpRequest::init() {
    state_ = REQUEST_LINE;  // 初始状态
    method_ = path_ = version_= body_ = "";
    header_.clear();
    post_.clear();
}

// 解析处理
bool HttpRequest::parse(Buffer* buff){
    const char END[] = "\r\n";
    if(buff->readableBytes() == 0)   // 没有可读的字节
        return false;
    // 读取数据开始
    while(buff->readableBytes() && state_ != FINISH){
        // 从buff中的读指针开始到读指针结束，这块区域是未读取得数据并去处"\r\n"，返回有效数据得行末指针
        const char* lineend = std::search(buff->peek(), buff->beginWritePtrConst(), END, END+2);
        std::string line(buff->peek(), lineend);
        switch (state_)
        {
            case REQUEST_LINE:// 解析错误
                if(!parseRequestLine_(line))
                {
                    return false;
                }
                parsePath_();   // 解析路径
                break;
            case HEADERS:
                parseHeader_(line);
                if(buff->readableBytes() <= 2) {  // 说明是get请求，后面为\r\n
                    state_ = FINISH;   // 提前结束
                }
                break;
            case BODY:
                parseBody_(line);
                break;
            default:
                break;
        }
        if(lineend == buff->beginWritePtr()) {  // 读完了
            buff->retrieveAll();
            break;
        }
        buff->moveReadPosToEnd(lineend + 2);        // 跳过回车换行
    }
    LOG_DEBUG("%s %s\n", method_.c_str(), path_.c_str());
    return true;
}

bool HttpRequest::parseRequestLine_(const std::string& line)
{
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;  // 用来匹配patten得到结果
    // 在匹配规则中，以括号()的方式来划分组别 一共三个括号 [0]表示整体
    if(std::regex_match(line, subMatch, patten)){ // 匹配指定字符串整体是否符合
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error\n");
    return false;
}

// 解析路径，统一一下path名称,方便后面解析资源
void HttpRequest::parsePath_(){
    if(path_ == "/"){
        path_ = "/index.html";
    }
    else{
        if(DEFAULT_HTML.find(path_) != DEFAULT_HTML.end()) {
            path_ += ".html";
        }
    }
}

// 解析请求头
void HttpRequest::parseHeader_(const std::string& line){
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line, subMatch, patten)){
        header_[subMatch[1]] = subMatch[2];
    } else { //匹配失败说明首部行匹配完了，状态变化
        state_ = BODY;
    }
}

// 解析请求体
void HttpRequest::parseBody_(const std::string& line){
    body_ = line;
    parsePost_();
    state_ = FINISH; // 状态转换为下一个状态
    LOG_DEBUG("Body:%s, len:%d\n", line.c_str(), line.size());
}

// 16进制转化为10进制
int HttpRequest::converHex(char ch){
    if(ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if(ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return ch;
}

// 处理post请求
void HttpRequest::parsePost_(){
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded"){
        parseFromUrlencoded_();  // POST请求体示例
        if(DEFAULT_HTML_TAG.count(path_)){
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            if(tag == 1){ //用户登录
                if(userLogin(post_["username"], post_["password"])){
                    path_ = "/welcome.html";
                } else {
                    path_ = "/loginfail.html";
                }
            } else if(tag == 0){ //用户注册
                if(userRegister(post_["username"], post_["password"])){
                    path_ = "/welcome.html";
                } else {
                    path_ = "/registerfail.html";
                }
            } else { //未知情况
                path_ = "/error.html";
            }
        }
    }
}

// 从url中解析编码
void HttpRequest::parseFromUrlencoded_(){
    if(body_.size() == 0) { return; }

    std::string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++){
        char ch = body_[i];
        switch (ch)
        {
            case '=':
                key = body_.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                num = converHex(body_[i+1]) * 16 + converHex(body_[i+2]);
                body_[i + 2] = num % 10 + '0';
                body_[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s\n", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    }
    assert(j <= i);
    if(post_.count(key) == 0 && j < i){
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

// 用户登录检测
bool HttpRequest::userLogin(const std::string &name, const std::string &pwd) {
    if(name == "" || pwd == "") { return false; }
    LOG_INFO("Login name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SQLConnectionRAII(&sql, SQLConnectionPool::instance());
    assert(sql);

    bool flag = false;
    char order[256] = { 0 };
    MYSQL_RES *res = nullptr;

    /* 查询用户及密码 */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s\n", order);

    if(mysql_query(sql, order)) {
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s\n", row[0], row[1]);
        std::string password(row[1]);
        if(pwd == password) { flag = true; }
        else {
            flag = false;
            LOG_INFO("password error\n");
        }
    }
    mysql_free_result(res);
    return flag;
}

// 用户注册
bool HttpRequest::userRegister(const std::string &name, const std::string &pwd) {
    MYSQL* sql;
    SQLConnectionRAII(&sql, SQLConnectionPool::instance());

    bool flag = false;
    char order[256] = { 0 };
    MYSQL_RES *res = nullptr;

    // 查询数据库中是否已有用户
    bzero(order, 256);
    snprintf(order, 256, "SELECT username FROM user WHERE username='%s' LIMIT 1", name.c_str());
    if(mysql_query(sql, order)) {
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        std::string username(row[0]);
        if(name == username) {
            LOG_INFO("user used\n");
            mysql_free_result(res);
            return false;
        }
    }

    // 进行注册行为
    bzero(order, 256);
    snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
    LOG_DEBUG( "%s", order);
    if(mysql_query(sql, order)) {
        LOG_DEBUG( "Insert error\n");
        flag = false;
    }
    flag = true;
    LOG_INFO( "userRegister success\n");
    return flag;
}

std::string HttpRequest::path() const{
    return path_;
}

std::string& HttpRequest::path(){
    return path_;
}
std::string HttpRequest::method() const {
    return method_;
}

std::string HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::getPost(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::getPost(const char* key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

bool HttpRequest::isKeepAlive() const {
    if(header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}