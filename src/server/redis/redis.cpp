#include "/home/caht/chat_project/include/server/redis/redis.hpp"
#include <cstdlib>
#include <hiredis/hiredis.h>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>
#include <thread>


Redis::Redis():_publish_context(nullptr), _subcribe_context(nullptr){}

Redis::~Redis(){
    if (_publish_context != nullptr) {
        redisFree(_publish_context);
    }
    if (_subcribe_context != nullptr) {
        redisFree(_subcribe_context);
    }
}

bool Redis::connect(){
    // 负责publish发布消息的上下文链接
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (_publish_context == nullptr) {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }

    _subcribe_context = redisConnect("127.0.0.1", 6379);
    if (_subcribe_context == nullptr) {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }

    std::thread t{
        [&] (){
            observer_channel_message();
            }
    };
    t.detach();

    std::cout << "connect redis-server success!" <<std::endl;
    return true;
}


void Redis::publish(int channel, std::string message){
    redisReply* reply = (redisReply* )redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());

    if (reply == nullptr) {
        std::cerr << "publish command failed!" <<std::endl;
        return;
    }
    freeReplyObject(reply);

    return;
}



void Redis::subscribe(int channel){

    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel)) { // 没有连接上
        std::cerr << "subscribe command failed!" << std::endl;
        return;
    }

    int done = 0;

    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done)) {
            std::cerr << "subscribe command failed!" << std::endl;
            return;
        }
    }
    return;
}

void Redis::unsubscribe(int channel){
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel)) { // 没有连接上
        std::cerr << "unsubscribe command failed!" << std::endl;
        return;
    }
    // redisBufferWrite可以循环发送缓冲区，知道缓冲区数据发送完毕(done被置为1)
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done)) {
            std::cerr << "subscribe command failed!" << std::endl;
            return;
        }
    }
    return;
}

void Redis::observer_channel_message(){
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply)) {
        // 订阅接收到的消息带有三个元素 ：message channel 消息
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    std::cerr << ">>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<" << std::endl;
}
void Redis::init_notify_handler(std::function<void(int, std::string)> fn){
    this->_notify_message_handler = fn;
}

