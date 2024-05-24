#include "chatserver.hpp"
#include "chatservice.hpp"
#include <signal.h>

//服务器异常断开 (以Ctrl+C为例)
//实际业务中可能会遇到断电或者其他的异常，导致的服务器断开问题
void resetHandler(int sig)
{
    ChatService::instance()->reset();
    exit(0);
}


int main(int argc, char*argv[])
{
    if(argc<3)
    {
        cerr<<"command invalid! example: ./ChatClient 192.168.164,132 6000" <<endl;
        exit(-1);

    }
    struct sigaction act;
    act.sa_flags = 1;
    act.sa_handler = resetHandler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
    
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    EventLoop loop;
    InetAddress listenaddress(ip, port);
    ChatServer server(&loop, listenaddress, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}