#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <string>
#include <thread>
#include <functional>

class Redis{
public:
    Redis();
    ~Redis();

    bool connect();

    void publish(int channel, std::string message);

    void subscribe(int channel);

    void unsubscribe(int channel);

    void observer_channel_message();

    void init_notify_handler(std::function<void(int, std::string)> fn);


private:

    redisContext* _publish_context;


    redisContext* _subcribe_context;

    std::function<void(int, std::string)> _notify_message_handler;

};


#endif