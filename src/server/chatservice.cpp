/*
业务模块代码
将LOGIN_MSG和login方法绑定在一起
注意：不要在业务模块里写数据模块代码，要拆分这两个模块的代码
*/
#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>

using namespace muduo;
using namespace std;

// 获取单例对象的接口函数
// 静态方法在类外写的时候不需要再加static
ChatService *ChatService::instance(){
    static ChatService service;  // 单例对象
    return &service;
}

// 注册消息以及对应的handler回调操作
// 网络模块和业务模块解耦的核心
ChatService::ChatService(){
    // 用户基本业务管理相关事件处理回调注册
    //把LOGIN_MSG和login方法绑定在一起 检测到msgid是LOGIN_MSG，就执行login操作
    _msghandlerMap.insert({LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3)});
    _msghandlerMap.insert({REG_MSG, bind(&ChatService::reg, this, _1, _2, _3)});
    _msghandlerMap.insert({ONE_CHAT_MSG, bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msghandlerMap.insert({QUIT_MSG, bind(&ChatService::quit, this, _1, _2, _3)});
    _msghandlerMap.insert({ADD_FRIEND_MSG, bind(&ChatService::addFriend, this, _1, _2, _3)});

    // 群组业务管理相关事件处理回调注册
    _msghandlerMap.insert({CREATE_GROUP_MSG, bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msghandlerMap.insert({ADD_GROUP_MSG, bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msghandlerMap.insert({GROUP_CHAT_MSG, bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if(_redis.connect()){
        // 设置上报消息的回调
        _redis.init_notify_handler(bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 服务器异常，业务重置方法
void ChatService::reset(){
    // 把online状态的用户，设置成offline
    _userModel.resetState();
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid){
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = _msghandlerMap.find(msgid);  //不要使用find[]，因为使用[]没找到的话会自动将这对值添加进去
    if(it == _msghandlerMap.end()){
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp){
            //使用muduo库的打印 不要用cout
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";  //LOG_ERROR不需要用endl
        };
    }
    else{
        return _msghandlerMap[msgid];
    }
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time){
    // LOG_INFO<<" do login service!";
    int id = js["id"].get<int>();  // get<int>()是把字符串类型转化为整形
    string pwd = js["password"];
    User user = _userModel.query(id);
    if(user.getID()==id && user.getPwd()==pwd){
        if(user.getState()=="online"){
            // 该用户已经登录，不允许重复登录
            json responce;
            responce["msgid"] = LOGIN_MSG_ACK;
            responce["errno"] = 2;
            responce["errmsg"] = "this account is using, input another!";
            conn->send(responce.dump());
        }
        else{
            // 登录成功 记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});  // 涉及到多线程同时向这个表中增删改查 需要考虑线程安全
            }// 加{}用来确定作用域 出了}会自动解锁 线程安全不用进来加锁 出去解锁
            // id用户登录成功后，向redis订阅channel
            _redis.subscribe(id);

            // 登录成功 更新用户状态信息 state offline->online
            user.setState("online");
            _userModel.updateState(user);

            json responce;
            responce["msgid"] = LOGIN_MSG_ACK;
            responce["errno"] = 0;  // 如果检测errno为0 就表示这个响应没错 响应成功了
            responce["id"] = user.getID();
            responce["name"] = user.getName();
            // 查询该用户是否有离线消息，有的话要提供给该用户
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty()){
                responce["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }
            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty()){
                vector<string> vec2;
                for(User &user : userVec){
                    json js;
                    js["id"] = user.getID();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                responce["friends"] = vec2;
            }
            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if(!groupuserVec.empty()){
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for(Group &group : groupuserVec){
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for(GroupUser &user : group.getUsers()){
                        json js;
                        js["id"] = user.getID();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }
                responce["groups"] = groupV;
            }
            conn->send(responce.dump());
        }
        
    }
    else{
        // 该用户不存在/用户存在但是密码错误，登陆失败
        json responce;
        responce["msgid"] = LOGIN_MSG_ACK;
        responce["errno"] = 1;  // 如果检测errno为1 响应失败
        responce["errmsg"] = "id or password is invaild!";
        conn->send(responce.dump());
    }
}

// 处理注册业务 填name password 不用填id id是注册成功我们给返回的 也不需要填state 有默认值
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time){
    // LOG_INFO<<" do reg service!";
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if(state){
        // 注册成功 返回相应的json字符串给对端客户端
        json responce;
        responce["msgid"] = REG_MSG_ACK;
        responce["errno"] = 0;  // 如果检测errno为0 就表示这个响应没错 响应成功了
        responce["id"] = user.getID();
        conn->send(responce.dump());
    }
    else{
        // 注册失败
        json responce;
        responce["msgid"] = REG_MSG_ACK;
        responce["errno"] = 1;  // 如果检测errno为1 响应失败
        conn->send(responce.dump());
    }
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin(); it!=_userConnMap.end(); it++){
            if(it->second == conn){
                // 从map表删除用户的连接信息
                user.setID(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 在redis中取消订阅通道
    _redis.unsubscribe(user.getID());

    // 更新用户的状态信息
    if(user.getID()!=-1){
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end()){
            //toid在线，转发消息   服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线
    User user = _userModel.query(toid);
    if(user.getState()=="online"){
        _redis.publish(toid, js.dump());
        return;
    }
    //toid不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    // 存储好友信息
    _friendModel.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if(_groupModel.createGroup(group)){
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}
// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}
// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for(int id : useridVec){
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()){
            // 转发群消息
            it->second->send(js.dump());
        }
        else{
            // 查询toid是否在线
            User user = _userModel.query(id);
            if(user.getState()=="online"){
                _redis.publish(id, js.dump());
            }
            else{
                // 存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

// 登出业务
void ChatService::quit(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end()){
            _userConnMap.erase(it);
        }
    }
    // 用户注销，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    User user(userid, "", "","offline");
    _userModel.updateState(user);
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}