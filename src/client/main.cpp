#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <thread>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <ctime>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "/home/caht/chat_project/include/server/model/group.hpp"
#include "/home/caht/chat_project/include/server/model/user.hpp"
#include "/home/caht/chat_project/include/public.hpp"
#include "/home/caht/chat_project/thirdparty/json.hpp"

using json = nlohmann::json;
// 全局变量记录当前用户信息
User g_currentUser; 
// 记录当前登录用户的好友列表信息
std::vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
std::vector<Group> g_currentUserGroupList;

bool isMainMenuRuning = true;
// 显示当前登录成功用户的基本信息
void showCurrentUserData();

// 接收线程
void readTaskHandler(int clientfd);

// 获取系统时间
std::string getCurrentTime();
// 主聊天业务程序
void mainMenu(int clientfd);

// 聊天客户端实现，main线程用作发送线程，子线程用于接收线程。
int main(int argc, char **argv){
    if (argc < 3)
    {
        std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << std::endl;
        exit(-1);
    }

    char* ip = argv[1];
    std::uint16_t port = atoi(argv[2]); // 数字字符串转换成int类型的整数,里面传的是一个指针

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }

    // 填写client需要连接的server信息ip + port
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // client 和 server进行连接
    if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)) == -1)
    {
        std::cerr << "connect server error" << std::endl;
        close(clientfd);
        exit(-1);
    }

    // 连接成功，以及菜单 登录、注册、退出
    for (; ; ) 
    {
        
        // 显示首页面菜单  登录、注册、退出
        std::cout << "=======================" << std::endl;
        std::cout << "       1.login         " << std::endl;
        std::cout << "       2.register      " << std::endl;
        std::cout << "       3.quit          " << std::endl;
        std::cout << "=======================" << std::endl;
        std::cout << "choice:";
        int choice = 0;
        std::cin >> choice;   
        // 前面是一个整数，后面要输入一个字符串，在输入整数后需要对残留的回车进行读取，不然会干扰字符串的读取
        // 要重缓冲区读一个整数，之后要把回车给读掉。
        std::cin.get(); // 读掉缓冲区残留的回车 \n

        switch (choice) 
        {
        case 1: // login  登录
        {
            int id = 0;
            char pwd[50] = {0};
            std::cout << "userid:";
            std::cin >> id;
            std::cin.get();
            std::cout << "password:";
            std::cin.getline(pwd, 50); // 读取在一整行，避免空格断开

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            std::string request = js.dump(); // 序列化

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1) // 没有发送成功
            {
                std::cout << "send login msg error:" << request << std::endl;
            }
            else {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (len == -1) // 接收失败
                {
                    std::cerr << "recv login response error!" << std::endl;
                }else { // 登录成功
                    std::string bufstr(buffer);
                    json responseJs = json::parse(bufstr); // 出错
                    if (responseJs["errno"].get<int>() != 0){ // 用户重复登录
                        std::cerr << responseJs["errmsg"] << std::endl;
                    }else { // 登录成功
                        g_currentUser.setId(responseJs["id"].get<int>());
                        g_currentUser.setName(responseJs["name"]);

                        // 记录当前用户的好友信息
                        if (responseJs.contains("friends")) { // 找一下js中是否存在某个元素
                            // 初始化
                            g_currentUserFriendList.clear();
                            std::vector<std::string> vec = responseJs["friends"]; // 拿出好友信息，一行一个好友信息
                            
                            for (std::string &str : vec){
                                json js = json::parse(str); // 反序列化 string -> json
                                User user;
                                user.setId(js["id"].get<int>());
                                user.setName(js["name"]);
                                user.setState(js["state"]);
                                g_currentUserFriendList.push_back(user);
                            }
                        }

                        if (responseJs.contains("groups")) {

                            g_currentUserGroupList.clear();

                            std::vector<std::string> groupVec = responseJs["groups"]; // 拿出群数据
                            for (std::string &groupstr : groupVec) {
                                json groupjs = json::parse(groupstr);
                                Group group;
                                group.setId(groupjs["groupid"].get<int>());
                                group.setName(groupjs["groupname"]);
                                group.setDesc(groupjs["groupdesc"]);

                                std::vector<std::string> userVec = groupjs["users"];
                                for (std::string &userstr : userVec) {
                                    GroupUser user;
                                    json userjs = json::parse(userstr);
                                    user.setId(userjs["id"]);
                                    user.setName(userjs["name"]);
                                    user.setState(userjs["state"]);
                                    user.setRole(userjs["role"]);
                                    group.getUser().push_back(user);
                                }
                                g_currentUserGroupList.push_back(group);
                            }
                        }
                        // 显示当前用户的基础信息
                        showCurrentUserData();
                        // 显示当前用户的离线消息，个人聊天信息或者群组消息
                        if (responseJs.contains("offlinemsg")) {
                            std::vector<std::string> offlinemsgVec = responseJs["offlinemsg"]; // 拿出数据
                            for (std::string offlinemsgstr: offlinemsgVec) {
                                json js = json::parse(offlinemsgstr); // 反序列化

                                // user的在线消息
                                if (js["msgid"].get<int>() == ONE_CHAT_MSG) {
                                    std::cout << js["time"].get<std::string>() << "  [userid:" << js["id"].get<int>() << "]:" 
                                    << js["name"].get<std::string>() << "  said: " << js["msg"].get<std::string>() <<std::endl;
                                }else{ // 群的在线消息
                                    std::cout << "群消息[groupid:" << js["groupid"] <<"]:" <<  js["time"].get<std::string>() << "   [userid" << js["id"].get<int>() << "] " 
                                    << js["name"].get<std::string>() << "said: " << js["msg"].get<std::string>() <<std::endl;
                                }
                            }
                        }
                        // 登录成功，启动接收线程负责接收数据
                        static int readthreadnumber = 0;
                        if (readthreadnumber == 0){
                            std::thread readTask(readTaskHandler, clientfd); // 该线程只启动一次
                            readTask.detach();  // 和 pthread.detach 一样     释放线程
                            readthreadnumber++;
                        }

                        
                        // 进入聊天主页面
                        isMainMenuRuning = true;
                        mainMenu(clientfd);
                    }
                }
            }
        }
        break;
        case 2: // 注册业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            std::cout <<"name:";
            std::cin.getline(name, 50); // cin scanf  遇到空格和回车都会结束输入
            std::cout << "password:";
            std::cin.getline(pwd, 50);

            json js;
            js["msgid"] = RED_MSG;
            js["name"] = name;
            js["password"] = pwd;
            std::string request = js.dump(); // 序列化

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                std::cerr << "send reg.msg error" << request << std::endl;
            }
            else 
            {   // 注册消息存在响应
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (len == -1)
                {
                    std::cerr << "recv reg response error" << std::endl;
                }
                else {
                    json responseJs = json::parse(buffer); // 反序列化
                    if (responseJs["errno"].get<int>() != 0) // 往服务器内添加数据异常
                    {
                        std::cerr << name << "is laready exist, register error!" << std::endl;
                    }else { //注册成功
                        std::cout << "register success, userid is " << responseJs["id"]
                                    << ", do not forget it!" << std::endl;
                    }
                }
            }
        }
        break;
        case 3: // 退出业务
            close(clientfd);
            exit(-1);
        default:
            std::cerr << " invalid input!" << std::endl;
            break;
        }
    }
    return 0;
}


void showCurrentUserData(){
    std::cout << "========================login user========================" << std::endl;
    std::cout << "current login user => id:" << g_currentUser.getId() << "       name:" 
    << g_currentUser.getName() <<std::endl;
    std::cout << "-----------------------friend list------------------------" << std::endl;
    if (!g_currentUserFriendList.empty()) {
        for (User &friendUser : g_currentUserFriendList) {
            std::cout << friendUser.getId() << "  "<< friendUser.getName() << "    " << friendUser.getState() << std::endl;
        }
    }
    std::cout << "-----------------------group list-------------------------" << std::endl;
    if (!g_currentUserGroupList.empty()) {
        for (Group &userGroup : g_currentUserGroupList) {
            std::cout << userGroup.getId() << "  "<< userGroup.getName() << "  "<< userGroup.getDesc()<< std::endl;
            for (GroupUser &user : userGroup.getUser()) {
                std::cout <<"    "<< user.getId() << "  "<< user.getName() << "  "<< user.getState() 
                << "  "<< user.getRole() << std::endl;
            }
        }
    }
    std::cout << "==========================================================" << std::endl;
}   
 // 接收线程 
void readTaskHandler(int clientfd){
    while (true) {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (len <= 0) {
            close(clientfd);
            exit(-1);
        }

        // 
        json js = json::parse(buffer); // 反序列化
        int msgtype = js["msgid"].get<int>();
        // user的在线消息
        if (msgtype == ONE_CHAT_MSG) {
            std::cout << js["time"].get<std::string>() << "[" << js["id"].get<int>() << "]:" 
            << js["name"].get<std::string>() << "said: " << js["msg"].get<std::string>() <<std::endl;
            continue;
        }

        // 群的在线消息
        if (msgtype == GROUP_CHAT_MSG) {
            std::cout << "群消息[groupid:" << js["groupid"] <<"]:" <<  js["time"].get<std::string>() << "   [userid:" << js["id"].get<int>() << "]:" 
            << js["name"].get<std::string>() << "   said: " << js["msg"].get<std::string>() <<std::endl;
            continue;
        }

        // 注册群响应
        if (msgtype == CREATE_GROUP_MSG_ACK) {
            
        }

    }
}



std::string getCurrentTime(){
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    struct std::tm *ptm = localtime(&now_time_t);
    char date[60];

    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", 
        (int)ptm->tm_year + 1900,(int)ptm->tm_mon + 1, (int)ptm->tm_mday,
        (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    
    return std::string(date);
}

void help(int, std::string);
void chat(int, std::string);
void addfriend(int, std::string);
void creategroup(int, std::string);
void addgroup(int, std::string);
void groupchat(int, std::string);
void logout(int, std::string);

// 系统支持的客户端操作
std::unordered_map<std::string, std::string> commandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "组建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat::groupid:message"},
    {"logout", "注销,格式quit"}};
// 注册系统支持的客户端命令处理
std::unordered_map<std::string, std::function<void (int, std::string)>> commendHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"logout", logout}};



void mainMenu(int clientfd){
    help(0,"");
    char buffer[1024] = {0};
    bool flag = true;
    while (isMainMenuRuning) {
        std::cin.getline(buffer, 1024);
        std::string commandbuf{buffer};
        std::string command;
        int idx = commandbuf.find(":");
        if (idx == -1) { // 没有出现  :
            command = commandbuf;
        }else {
            command = commandbuf.substr(0,idx);
        }
        auto it = commendHandlerMap.find(command);
        if (it ==commendHandlerMap.end()) {
            std::cerr << "invalid input command!" << std::endl;
            continue;
        }
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }

}



void help(int i = 0, std::string str = ""){
    std::cout << "show commend list >>>" << std::endl;
    for (auto &p : commandMap) {
        std::cout << p.first << ":" << p.second << std::endl;
    }
    std::cout << std::endl;
}

void chat(int clientfd, std::string str){
    int idx = str.find(":"); // friendid:message
    if (idx == -1){
        std::cerr << "chat command invalid!" << std::endl;
        return ;
    }
    int frinedid = atoi(str.substr(0,idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = frinedid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(),strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        std::cerr << "send addfriend msg error ->" << buffer <<std::endl;
        
    }
}

void addfriend(int clientfd, std::string str){
    int friendid = atoi(str.c_str()); // str: friendid
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    std::string buffer = js.dump(); // 序列化
    
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        std::cerr << "send addfriend msg error ->" << buffer <<std::endl;
    }
}

void creategroup(int clientfd, std::string str){ // groupname:groupdesc
    int idx = str.find(":"); 
    if (idx == -1){
        std::cerr << "creategroup command invalid!" << std::endl;
        return ;
    }

    std::string groupName = str.substr(0, idx);
    std::string groupDesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupName;
    js["groupdesc"] = groupDesc;
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        std::cerr << "send creategroup msg error ->" << buffer <<std::endl;
    }
}
void addgroup(int clientfd, std::string str){ // groupid
    int groupid = atoi(str.c_str()); 
    json js;
    js["msgid"] = ADD_GROUPER_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    std::string buffer = js.dump(); // 序列化
    
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        std::cerr << "send addgroup msg error ->" << buffer <<std::endl;
    }
}

void groupchat(int clientfd, std::string str){ // groupid:message
    int idx = str.find(":"); 
    if (idx == -1){
        std::cerr << "groupchat command invalid!" << std::endl;
        return ;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        std::cerr << "send groupchat msg error ->" << buffer <<std::endl;
    }
}

void logout(int clientfd, std::string str){
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = g_currentUser.getId();
    std::string buffer = js.dump(); // 序列化
    
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        std::cerr << "send addfriend msg error ->" << buffer <<std::endl;
    }else {
        isMainMenuRuning = false;
    }
    
}