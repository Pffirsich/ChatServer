#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
#include "user.hpp"
using namespace std;

// 维护好友信息的操作接口方法
class FriendModel{
public:
    // 添加好友关系
    void insert(int userid, int friendid);
    
    // 返回用户好友列表(正常应该存在客户端本地，为了简化业务 写在服务器端) 进行user表和friend表的联合查询
    vector<User> query(int userid);  // 通过一个用户id找到它的用户列表
};
#endif