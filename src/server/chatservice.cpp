#include "/home/caht/chat_project/include/server/chatservice.hpp"
#include "/home/caht/chat_project/include/public.hpp"
#include "/home/caht/chat_project/include/server/model/user.hpp"

#include <cstdint>
#include <functional>
#include <muduo/base/Logging.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>


using namespace std::placeholders;

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, _1, _2, _3)});
    _msgHandlerMap.insert({RED_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUPER_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    if (_redis.connect()) { // 链接服务器
        //注册一个回调，接收信息。从_redis中得到两个变量，channel和message
        _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage, this, _1, _2));
    }

}

// 获取消息对应的Handler
MsgHandler ChatService::getHandler(int msgid){
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())
    {
        return [=](const muduo::net::TcpConnectionPtr &conn,
                        json &js, muduo::Timestamp )
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }else {
    return _msgHandlerMap[msgid];
    }
    
}

// 用户登入   id   pwd
void ChatService::login(const muduo::net::TcpConnectionPtr &conn,
                        json &js, muduo::Timestamp time)
{   
    int id = js["id"].get<int>();
    std::string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online") {
            //该用户以及登录不允许重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2; // 错误标识，有错误
            response["errmsg"] = "该账号已经登录，请重新输入新账号或注销该账号！";
            conn->send(response.dump()); // 想响应的消息发送回去，send 支持string 和自制的
        }else {
            // 用户登录成功，
            {
                // 涉及多线程，一定要考虑线程安全
                std::lock_guard<std::mutex> lock(_connMutex);
                // 用户上线，用户下线
                _userConnMap.insert({id, conn}); // 这个map会被多个线程调用， 因为onmessage就会被多个线程(工作线程)调用，
            }
            // 在redis上订阅与id同名的通道
            _redis.subscribe(id);

            // 登入后更改用户状态
            user.setState("online");
            _userModel.updataState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // 错误标识，无错误
            response["id"] = user.getId();
            response["name"] = user.getName();
            response["time"] = time.toString();


            // 检查该用户是否有离线信息
            std::vector<std::string> offlineMsgVec = _offlineMsgmodel.query(id);
            if (!offlineMsgVec.empty())
            {
                // 存在离线信息
                response["offlinemsg"] = offlineMsgVec;  // vector与这个json可以直接序列化
                // 将数据库中读出来的离线消息删除
                _offlineMsgmodel.remove(id);
            }

            // 登录成功返回好友信息
            std::vector<User> friendUserVec = _friendmodel.query(id);
            if (!friendUserVec.empty()) 
            {
                std::vector<std::string> vec;
                for (User &user : friendUserVec) {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec.push_back(js.dump());  // 序列化成string
                }
                response["friends"] = vec;
            }
            std::vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty()) {
                std::vector<std::string> groupVec;
                for (Group &group : groupuserVec) {
                    json groupJs;
                    groupJs["groupid"] = group.getId();
                    groupJs["groupname"] = group.getName();
                    groupJs["groupdesc"] = group.getDesc();
                    
                    std::vector<std::string> userV;
                    for (GroupUser &user : group.getUser()) {
                        json groupUserJs;
                        groupUserJs["id"] = user.getId();
                        groupUserJs["name"] = user.getName();
                        groupUserJs["state"] = user.getState();
                        groupUserJs["role"] = user.getRole();
                        userV.push_back(groupUserJs.dump()); // 序列化
                    }
                    groupJs["users"] = userV;
                    groupVec.push_back(groupJs.dump());
                }
                response["groups"] = groupVec;
            }

            conn->send(response.dump()); // 想响应的消息发送回去，send 支持string 和自制的
        }
    }else{
        // 用户不存在或者密码错误，登入失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1; // 错误标识，有错误
        response["errmsg"] = "账号密码错误！！";

        conn->send(response.dump()); // 想响应的消息发送回去，send 支持string 和自制的
    }

}

// 处理注销业务
void ChatService::logout(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp){
    int userid = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        // 查找toid是否连接服务器
        auto it = _userConnMap.find(userid);  //it 放着连接的指针指针
        if (it != _userConnMap.end()) {
            _userConnMap.erase(it);
        }
    }

    // 用户注销，需要取消redis中订阅的通道
    _redis.unsubscribe(userid);

    // 更新用户表中的状态信息
    User user;
    user.setId(userid);
    user.setState("offline");
    _userModel.updataState(user);
}

// {"msgid":1,"id":2,"password":"123456"}
// 处理注册业务
void ChatService::reg(const muduo::net::TcpConnectionPtr &conn,
                            json &js, muduo::Timestamp)
{
    std::string name = js["name"];
    std::string pwd = js["password"];
    
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user); // 传入的是引用 insert函数会给user写入id
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = RED_MSG_ACK;
        response["errno"] = 0; // 错误标识，无错误
        response["id"] = user.getId();
        conn->send(response.dump()); // 想响应的消息发送回去，send 支持string 和自制的
    }else 
    {
        //注册失败
        json response;
        response["msgid"] = RED_MSG_ACK;
        response["errno"] = 1; // 错误标识，有错误
        conn->send(response.dump()); // 想响应的消息发送回去，send 支持string(传实例) 和buffer(传指针)的
    }
    
}
// 客户端异常退出
void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr &conn){
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);

        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
            if (it->second == conn) {
                // 从 map中删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    
    //
    _redis.unsubscribe(user.getId());
    // 更新用户表中的状态信息
    if (user.getId() != -1) {
        user.setState("offline");
        _userModel.updataState(user);
    }
    
}


void ChatService::oneChat(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp time)
{
    int toid = js["toid"].get<int>();
    // int ;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        // 查找toid是否连接服务器
        auto it = _userConnMap.find(toid);  //it 放着连接的指针指针
        if (it != _userConnMap.end()) {
            // toid在线,服务器直接将消息推送给toid
            it->second->send(js.dump());        // buffer 为序列化后的信息
            return;
        }
    }
    // 得到toid用户的状态
    User user = _userModel.query(toid);
    if (user.getState() == "online") {
        _redis.publish(toid, js.dump());
        return;
    }

    // toid 不在线，存储离线数据
    _offlineMsgmodel.insert(toid, js.dump());

}

// 重置服务器状态
void ChatService::reset()
{
    // 把online的用户改为offline
    _userModel.resetState();
}
// 添加好友操作 ： msgid id frinedid 
void ChatService::addFriend(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp)
{

    int userid = js["id"].get<int>();
    int frinedid = js["friendid"].get<int>();
    // 存储好友信息
    _friendmodel.insert(userid, frinedid);
}


// 创建群业务 msg id groupname groupdesc 
void ChatService::createGroup(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();
    std::string groupName = js["groupname"];
    std::string groupDesc = js["groupdesc"];

    Group group;
    group.setName(groupName);
    group.setDesc(groupDesc);
    if(_groupModel.createGroup(group))
    {
        _groupModel.addGrouper(userid, group.getId(), "creator");
    }

}

// 加入群业务  msg  id groupid
void ChatService::addGroup(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGrouper(userid, groupid, "normal");
}
// 群组聊天业务
void ChatService::groupChat(const muduo::net::TcpConnectionPtr &conn,json &js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::vector<int> useridVec = _groupModel.queryGroupUser(userid, groupid); // 群内成员

    std::lock_guard<std::mutex> lock(_connMutex);
    for (int id : useridVec) 
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end()) 
        {
            // 在线--同一台服务端->直接发送
            it->second->send(js.dump());
        }else
        {
            // 在线--不同的服务端->发送给redis
            User user = _userModel.query(id);
            if (user.getState() == "online") {
                _redis.publish(id, js.dump());
                return;
            }
            // 离线
            _offlineMsgmodel.insert(id, js.dump());
        }
    }
}

// 从redis消息队列中获取订阅的消息  
void ChatService::handlerRedisSubscribeMessage(int userid, std::string msg){

    std::lock_guard<std::mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end()) {
        it->second->send(msg);
        return;
    }
    
    _offlineMsgmodel.insert(userid, msg);
}

