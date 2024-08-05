#include "usermodel.hpp"
#include "db.h"
#include <iostream>

// user表的增加方法
bool UserModel::insert(User &user){
    // 先组装sql语句，再输出sql语句。插入成功后获得这条sql语句的主键id
    // name要求是UNIQUE的(不能重复)，但这里不需要考察，这是业务层该考虑的。这里只需要实现插入命令即可
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
    user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    MySQL mysql;
    // 2.连接数据库，更新sql语句
    if(mysql.connect()){
        if(mysql.update(sql)){
            // 3.获取插入成功的用户数据生成的主键id
            user.setID(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 根据用户号码查询用户信息
User UserModel::query(int id){
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    MySQL mysql;
    // 2.连接数据库，更新sql语句
    if(mysql.connect()){
        MYSQL_RES * res = mysql.query(sql);
        // 3.从res里读数据
        if(res!=nullptr){ // 查成功了
            MYSQL_ROW row = mysql_fetch_row(res);  //从资源中把行全获取出来
            if(row!=nullptr){ //里面有数据
                User user;
                user.setID(atoi(row[0]));   // atoi把字符串转化成int
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);

                mysql_free_result(res);  // 释放指针资源
                return user;
            }
        }
    }
    return User();
}

// 更新用户的状态信息
bool UserModel::updateState(User user){
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getID());

    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}

// 重置用户的状态信息
void UserModel::resetState(){
    // 1.组装sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    MySQL mysql;
    // 2.连接数据库，更新sql语句
    if(mysql.connect()){mysql.update(sql); }
}