#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

using json=nlohmann::json;



ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenaddres, const string& nameArg)
                :_server(loop, listenaddres, nameArg), loop(loop)
{
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1,_2,_3));
    _server.setThreadNum(4);
}

void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }

}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer*buf, Timestamp time)
{
    string b = buf->retrieveAllAsString();
    json js = json::parse(b);
    ChatService* p = ChatService::instance();
    auto handler = p->getHandler(js["msgid"].get<int>());
    handler(conn,js, time);



    //注册信息类型对应的处理函数(回调函数)

}

void ChatServer::start()
{
    _server.start();
}
