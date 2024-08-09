#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "json.hpp"
#include "groupmodel.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "frindmodel.hpp"
#include "group.hpp"
#include "redis.hpp"

#include <cstdint>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>

using json = nlohmann::json;

using MsgHandler = std::function<void (const muduo::net::TcpConnectionPtr &conn,
                            json &js, muduo::Timestamp time)>;

// 连天服务器业务类
class ChatService{

public:
    // 获取单例对象的接口函数
    static ChatService* instance(); // 创建一个ChatService* 
    // 处理登入业务
    void login(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp);
    // 处理注册业务
    void reg(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp);
    // 一对一聊天业务
    void oneChat(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp); // 网络层直接派发过来的，输入参数需要满足网络层要求
    // 添加好友业务
    void addFriend(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp);
    // 创建群业务
    void createGroup(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp);
    // 加入群业务
    void addGroup(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp);
    // 群组聊天业务
    void groupChat(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp);
    // 处理注销业务
    void logout(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp);

    void handlerRedisSubscribeMessage(int userid, std::string msg);


    // 获取对应的事件处理器
    MsgHandler getHandler(int msgid);
    // 重置服务器状态
    void reset();
    // 处理客户端异常退出
    void clientCloseException(const muduo::net::TcpConnectionPtr &conn);

private:
    ChatService();
    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgmodel;
    FriendModel _friendmodel;
    GroupModel _groupModel;

    std::unordered_map<int, MsgHandler> _msgHandlerMap; // 业务处理其不存在在运行的过程中区怎加业务，所以没有考虑线程安全
    
    // 存储在线用户的连接s
    // 这个map在运行的过程中，用户的行为会对其进行修改，所以一定要考虑线程安全。
    std::unordered_map<int, muduo::net::TcpConnectionPtr> _userConnMap; // 如果断开连接muduo::net::TcpConnectionPtr为空
    // 定义互斥锁，保证_userConnMap的线程安全。
    std::mutex _connMutex;

    // redis操作对象
    Redis _redis;

};



#endif