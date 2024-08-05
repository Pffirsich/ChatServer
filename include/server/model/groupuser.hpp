#ifndef GROUPUSER_h
#define GROUPUSER_H
#include "user.hpp"

// 群组用户，多了一个role角色信息，从user类直接继承，复用user的其他信息
class GroupUser : public User{
public:
    void setRole(string role){this->role = role;}
    string getRole(){return this->role;}
private:
    string role;  // 添加了一个该派生类独有的成员变量 role 其他的id name等成员变脸继承自user类 也有
};


#endif