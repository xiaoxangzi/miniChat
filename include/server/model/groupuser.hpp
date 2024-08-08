#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"
#include <string>
struct GroupUser:User
{
public:
    void setRole(std::string role) {this->role = role;}
    std::string getRole(){return this->role;}
private:
    std::string role;
};




#endif