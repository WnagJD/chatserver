#include <iostream>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;


class ChatServer
{
    public:
        ChatServer(EventLoop *loop, const InetAddress& listenaddr, const string& nameArg):
            _loop(loop), _server(loop, listenaddr, nameArg)
            {
                //给服务器注册用户连接和断开的回调函数
                _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

                //给服务器注册读写事件的回调函数
                // _server.setMessageCallback(bind(onMessage, this, _1,_2,_3));
                _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

                _server.setThreadNum(2);

            }
       
        
        void start()
            {
                _server.start();
            }
        
    private:
        TcpServer _server;
        EventLoop * _loop;

        void onConnection(const TcpConnectionPtr& conn)
        {
            if(conn->connected())
            {
                cout<<conn->peerAddress().toIpPort()<<"->"<< conn->localAddress().toIpPort() <<"state:online"<<endl;

            }
            else{
                cout<<conn->peerAddress().toIpPort()<<"->"<< conn->localAddress().toIpPort() <<"state:offline"<<endl;
                conn->shutdown();
                //_loop_quit();
            }
        }

        void onMessage(const TcpConnectionPtr& conn,Buffer *buffer,Timestamp time)
        {
            string buf = buffer->retrieveAllAsString();
            cout<<"recv data:"<<buf<<"time:"<<time.toFormattedString()<<endl;
            conn->send(buf);
        }
    
};



int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();


}

