#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <functional>
#include <mutex>
#include "json.hpp"
#include "public.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"


using namespace muduo;
using namespace muduo::net;
using json=nlohmann::json;
using namespace std;
// using MsgHandler = std::function<void(const TcpConnectionPtr* conn, const json js, Timestamp time)>;
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;


//聊天服务器业务处理类
class ChatService
{
    public:
        ChatService();

        //获取单例对象的接口函数
        static ChatService* instance();

        //处理登录业务
        void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
        
        //处理注册业务
        void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

        //获取对应消息的处理器
        MsgHandler getHandler(int msgid);

        //处理客户端异常退出
        void clientCloseException(const TcpConnectionPtr& conn);

        //处理服务器异常退出
        void reset();

        //一对一聊天业务
        void oneChat(const TcpConnectionPtr& conn, json &js, Timestamp time);

        //添加好友关系
        void addFriend(const TcpConnectionPtr& conn, json &js, Timestamp time);

        //创建群组业务
        void createGroup(const TcpConnectionPtr& conn, json &js, Timestamp time);

        //加入群组业务
        void addGroup(const TcpConnectionPtr& conn, json &js, Timestamp time);

        //群聊天业务
        void groupChat(const TcpConnectionPtr& conn, json &js, Timestamp time);

        //用户退出业务
        void loginout(const TcpConnectionPtr& conn, json &js, Timestamp time);

        //从redis消息队列中获取消息
        void handleRedisSubsribeMessage(int, string);
    
    private:
        //消息id和对应的业务处理方法
        unordered_map<int,  MsgHandler> _msgHandlerMap;
        UserModel _userModel;
        OfflineMsgModel _offlineMsgModel;
        FriendModel _friendModel;
        GroupModel _groupModel;
        Redis _redis;

        //记录用户的连接信息(存储在线用户的通信连接)
        unordered_map<int, TcpConnectionPtr> _userConnMap;

        //互斥锁mutex,用于保证_userConnMap的线程安全
        mutex _connMutex;

};






#endif