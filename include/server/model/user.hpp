#ifndef USER_H
#define USER_H

#include <string>

// User 表的映射类
class User
{
public:
    User(int id = -1, std::string name = "", std::string pwd = "", std::string state = "offline")
    :id(id), name(name), password(pwd), state(state){}
    void setId(int id) {this->id = id;}
    void setName(std::string name) {this->name = name;}
    void setPwd(std::string pwd) {this->password = pwd;}
    void setState(std::string state) {this->state = state;}

    int getId() {return this->id;}
    std::string getName() {return this->name ;}
    std::string getPwd() {return this->password;}
    std::string getState() {return this->state;}


private:
    int id;
    std::string name;
    std::string password;
    std::string state;

};

#endif