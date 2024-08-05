/*
网络模块代码
使用了muduo库，得到了一个基于事件驱动的I/O多路复用epoll+线程池的网络
基于Reactor模型的，1个主Reactor是I/O线程，负责处理新用户连接，3个子Reactor是工作线程负责处理已连接用户的读写事件
*/
#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"
#include <functional>
#include <string>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg),
      _loop(loop)
{
    // 注册连接回调
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1)); 
    //onConnection的第一个参数就是setConnectionCallback的第一个参数 回调函数绑定
    // 注册消息回调
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置线程数量
    _server.setThreadNum(4);
}
// 启动服务
void ChatServer::start()
{
    _server.start();
}

// 上报连接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 客户端断开连接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn); // 客户端异常关闭
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 数据的反序列化
    json js = json::parse(buf);

    /* 达到的目的：完全解耦网络模块的代码和业务模块的代码 */

    // 通过js["msgid"]获取业务handler
    // 使用get强转成int类型
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    // 通过事件回调将网络模块代码和业务模块代码完全拆分开
    msgHandler(conn, js, time);
}