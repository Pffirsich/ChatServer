#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

// user表的ORM类，对照user表设计一个类出来，映射表的字段
// user表有4列：userid INT 用户id 约束NOT NULL
class User{
public:
    // public中提供一些构造的方法 还有set get之类
    User(int id=-1, string name="", string pwd = "", string state = "offline"){
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    void setID(int id){this->id = id;}
    void setName(string name){this->name = name;}
    void setPwd(string pwd){this->password = pwd;}
    void setState(string state){this->state = state;}

    int getID(){return this->id;}
    string getName(){return this->name;}
    string getPwd(){return this->password;}
    string getState(){return this->state;}
    
private:
    int id;
    string name;
    string password;
    string state;
};

#endif