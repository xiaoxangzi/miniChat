#include "chatserver.hpp"
#include "chatservice.hpp"
#include <muduo/base/StringPiece.h>

#include <iostream>
// #include <mysql/mysql_com.h>
#include <signal.h>

// {"msgid":1,"id":1,"password":"123456"}  登入1
// {"msgid":1,"id":2,"password":"123456"}  登入2
// {"msgid":5,"id":1,"from":"zhang san","to":2,"msg":"hello!"}
// {"msgid":5,"id":2,"from":"zhang san","to":1,"msg":"hello111!"}
// {"msgid":6,"userid":1,"frinedid":2}      添加好友
// sql语句 SELECT a.id, a.name, a.state FROM user a INNER JOIN friend b ON b.friendid = a.id WHERE b.userid = 1;

void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0); 
}

int main(int argc, char **argv){
    signal(SIGINT, resetHandler);
    if (argc < 3)
    {
        std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << std::endl;
        exit(-1);
    }
    char* ip = argv[1];
    std::uint16_t port = atoi(argv[2]); // 数字字符串转换成int类型的整数,里面传的是一个指针
    muduo::StringArg IP(ip);
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr{IP, port};
    ChatServer server{&loop, addr,"ChatServer"}; // 创建了一个TCP服务端muduo网络

    server.start();
    // 等待回调连接
    // 连接之后等待信息

    loop.loop();

    return 0;
}