#ifndef PUBLIC_H
#define PUBLIC_H

/*
    server和client的公共文件
*/
enum EnMsgType{
    LOGIN_MSG = 1,          // 登录消息
    LOGIN_MSG_ACK,          // 登录反馈消息

    LOGOUT_MSG,             // 用户退出消息


    RED_MSG,                // 注册消息
    RED_MSG_ACK,            // 注册响应消息

    ONE_CHAT_MSG,           // 聊天消息
    ADD_FRIEND_MSG,         // 添加好友信息   

    CREATE_GROUP_MSG,       // 创建群组消息
    CREATE_GROUP_MSG_ACK,   // 创建群组消息响应
    
    ADD_GROUPER_MSG,        // 添加群组成员消息
    GROUP_CHAT_MSG,         // 群组聊天消息


};


#endif