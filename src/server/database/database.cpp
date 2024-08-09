#include "database.h"

#include <muduo/base/Logging.h>

// 数据库配置信息
static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "123456";
static std::string dbname = "chat";


// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
    mysql_close(_conn);
}
// 连接数据库
/*
    问题：使用mysql_real_connect 连接数据库，无法连接。
    现象：mysql_real_connect输出为空指针。
    分析：user、server、password、dbname都正确，但是登入不进。ubuntu当时可以登入，然后又开始找代码的问题，最后发现Ubuntu当时是root权限
    改为普通权限后也登不进去，然后报错，通过该报错遭到解决方法。
    

*/
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
    password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // c/c++代码默认的编码字符是ASCII码，如果不设置，从MySQL上里下来的中文显示“？”
        mysql_query(_conn, "set names gbk");
    }
    return p;
}
// 更新操作
bool MySQL::update(std::string sql)
{
    if (mysql_query(_conn, sql.c_str()))// mysql_query 传入数据
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        << sql << "更新失败!";
        return false;
    }else {
        LOG_INFO << sql << "更新成功！"; // sql需要输入的字符串
        return true;
    }
}
// 查询操作
MYSQL_RES* MySQL::query(std::string sql)
{
    if (mysql_query(_conn, sql.c_str())) // mysql_query 传入数据
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

MYSQL* MySQL::getConnection()
{

    return _conn;
}
