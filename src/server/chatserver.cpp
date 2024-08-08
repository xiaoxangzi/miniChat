#include "/home/caht/chat_project/include/server/chatserver.hpp"
#include "/home/caht/chat_project/thirdparty/json.hpp"
#include "/home/caht/chat_project/include/server/chatservice.hpp"

#include <functional>
#include <muduo/net/Buffer.h>
#include <string>
using namespace std::placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(muduo::net::EventLoop* loop,
            const muduo::net::InetAddress& listenAddr,
            const muduo::string& nameArg)
            :_server(loop, listenAddr, nameArg),_loop(loop)
{
    _server.setConnectionCallback(
        std::bind(
            &ChatServer::onConnection,this,_1));
    
    _server.setMessageCallback(
        std::bind(
            &ChatServer::onMessage, this, _1,_2,_3));

    _server.setThreadNum(4);
};

void ChatServer::start(){
        _server.start();
}

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& conn){
    // 客户端断开连接  会给服务端发送一个智能指针，这个指针是在连接时创建的，用于纯粹该链接的信息。
    if(!conn->connected()){
        ChatService::instance()->clientCloseException(conn); // 用户断开 断开用户的conn会返回，将用户状态设置成offline
        conn->shutdown();
    }
}
// 
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString(); // 读出数据 转成字符串
    // 数据的反序列化
    json js = json::parse(buf);
    // 输入信息与格式不匹配问题

    // 达到的目的：完全解耦网络模块的代码和业务模块的代码————事件回调
    // 
    // 事件分发
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);
}
 


