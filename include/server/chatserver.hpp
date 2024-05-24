#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <string>
#include <iostream>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

//聊天服务器网络类
class ChatServer
{
    public:
        ChatServer(EventLoop*loop, const InetAddress &listenaddress, const string& nameArg);
        void onConnection(const TcpConnectionPtr& conn);
        void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
        void start();
    
    private:
        TcpServer _server;
        EventLoop *loop;
};




#endif