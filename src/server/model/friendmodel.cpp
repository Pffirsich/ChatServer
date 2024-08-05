#include "friendmodel.hpp"
#include "db.h"

// 添加好友关系
void FriendModel::insert(int userid, int friendid){
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values('%d', '%d')", userid, friendid);

    MySQL mysql;
    // 2.连接数据库，更新sql语句
    if(mysql.connect())  mysql.update(sql);
}

// 返回用户好友列表(正常应该存在客户端本地，为了简化业务 写在服务器端) 
vector<User> FriendModel::query(int userid){ // 通过一个用户id找到它好友的详细信息 包括id name和state
    char sql[1024] = {0};
    // 进行user表和friend表的联合查询
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid=a.id where b.userid = %d", userid);

    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES * res = mysql.query(sql);
        if(res!=nullptr){ // 查成功了
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){   // 循环 可能存在多条消息 都放入vec中
                User user;
                user.setID(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}