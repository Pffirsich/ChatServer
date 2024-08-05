#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include<string>
using namespace std;

//json序列化实例1 底层结构：无序的链式哈希表
string func1(){
    json js;
    js["msg.type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "lisi";
    js["msg"] = "hello world!";
    string sendBuf = js.dump();
    //js.dump()可以把数据序列化之后的json字符串赋给sendBuf
    //转成字符串之后就可以通过网络进行发送
    return sendBuf;
    //c_str()就是将C++的string转化为C的字符串数组，c_str()生成一个char *指针，指向字符串的首地址
    //为后面网络传输字符流做铺垫
}
//json序列化实例2 json的值可以还是一个json
string func2(){
    json js;
    //添加数组
    js["id"] = {1,2,3,4,5};
    js["name"] = "zhang san";
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";
    //上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
    return js.dump();
}

//json容器序列化 
string func3(){
    json js;
    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;
    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m;
    string sendBuf = js.dump();
    return sendBuf;
}
int main(){
    // string recvBuf = func1();
    // json jsbuf = json::parse(recvBuf);//json字符串反序列化成数据对象
    // cout<<jsbuf["msg_type"]<<endl;
    // cout<<jsbuf["from"]<<endl;

    // string recvBuf = func2();
    // json jsBuf = json::parse(recvBuf);
    // cout<<jsBuf["msg"]<<endl;
    // auto m = jsBuf["msg"];
    // cout<<m["zhang san"]<<endl;
    
    string recvbuf = func3();
    json jsbuf = json::parse(recvbuf);
    vector<int> v = jsbuf["list"];
    for(auto &a : v){
        cout<<a<<" ";
    }
    cout<<endl;
    map<int, string> mymap = jsbuf["path"];
    for(auto &c : mymap){
        cout<<c.first<<" "<<c.second<<endl;
    }
    return 0;
}
