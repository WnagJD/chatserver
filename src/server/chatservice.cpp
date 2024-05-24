#include "chatservice.hpp"
#include <muduo/base/Logging.h>
#include "public.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"

using namespace std;
using namespace muduo;

ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1,_2,_3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1,_2,_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1,_2,_3)});

    //连接redis
    if(_redis.connect())
    {
        //设置上报函数
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubsribeMessage, this , _1, _2));
    }
    
    

}

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

//登录业务 id password
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    // LOG_INFO<<"do login service!!";
    int id = js["id"].get<int>();
    string password = js["password"];
    User user;
    user = _userModel.query(id);
    if(user.getId()!=-1 && user.getPwd() == password)//1.判断用户id存在 2.判断密码表示登录密码是否正确
    {
        if(user.getState() == "online")
        {
            //该用户已经登录,不允许重复登录
            json response;
            response["msgid"]=LOGIN_MSG_ACk;
            response["errno"]=1;
            response["errmsg"]="this account is using, input another!";
            conn->send(response.dump());
        }else
        {
            //成功登录
            {
                //将连接插入到_userConnMap
                lock_guard<mutex>lock(_connMutex);
                _userConnMap.insert({id, conn}); 
            }

            //id登陆成功后，向redis订阅通道Channel(id)
            _redis.subscribe(id);

            json response;
            user.setState("online");
            _userModel.updateState(user);
            response["msgid"]= LOGIN_MSG_ACk;
            response["errno"]=0;
            response["id"]= user.getId();
            response["name"]= user.getName();

            //返回好友信息列表   将容器直接赋值给json对象时,只能是基础的数据结构,不能是自定义的数据结构
            vector<User> users = _friendModel.query(id);
            if(!users.empty())
            {
                vector<string> vec;
                for(User user : users)
                {
                    json js;
                    js["id"]=user.getId();
                    js["name"]=user.getName();
                    js["state"]=user.getState();
                    vec.push_back(js.dump());
                }
                //转成string类型的容器
                response["friends"] = vec;
            }

            //返回群组信息列表
            vector<Group> groups = _groupModel.queryGroups(id);
            if(!groups.empty())
            {
                //groups:[{groupid, groupname,groupdesc,[{id, name, state, role},{xxx},{xxx},{xxx}]}]
                vector<string> groupvec;
                for(Group group : groups)
                {
                    json grpjs;
                    grpjs["groupid"]= group.getId();
                    grpjs["groupname"] = group.getName();
                    grpjs["groupdesc"] = group.getDes();

                    vector<string> uservec;
                    vector<GroupUser> groupusers = group.getUser();
                    for(GroupUser users : groupusers)
                    {
                        json js;
                        js["id"]= users.getId();
                        js["name"] = users.getName();
                        js["state"] = users.getState();
                        js["role"] = users.getRole();
                        uservec.push_back(js.dump());
                    }
                    grpjs["users"]=uservec;
                    groupvec.push_back(grpjs.dump());
                }
                response["groups"] = groupvec;

            }
            
            vector<string> vec;
            vec = _offlineMsgModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"]=vec;
                //读取用户的离线消息后,把该用户的所有的离线消息删除
                _offlineMsgModel.remove(id);
            }

            conn->send(response.dump());

        }
    }
    else
    {
        //该用户不存在
        if(user.getId() == -1)
        {
            json response;
            response["msgid"]= LOGIN_MSG_ACk;
            response["errno"]=1;
            response["errmsg"]="this account not exist, enter  valid id again!";
            conn->send(response.dump());
        }
        else //密码不正确
        {
            json response;
            response["msgid"]= LOGIN_MSG_ACk;
            response["errno"]=1;
            response["errmsg"]="password error!";
            conn->send(response.dump());
        }
    }
    
}

//注册业务 使用name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    // LOG_INFO<<"do reg service!!";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if(state)
    {
        //注册成功
        json response;
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=0;
        response["id"]=user.getId();
        conn->send(response.dump());
    }else{
        //注册失败
        json response;
        response["msgid"]= REG_MSG_ACK;
        response["errno"]= 1;
        conn->send(response.dump());
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())
    {
        //返回一个默认的处理器，执行空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR<<"msgid"<<msgid<<"can not find handler!";
        };
    }else{
        return _msgHandlerMap[msgid];
    }

}

void ChatService::reset()
{
    //重置用户的状态信息为offline  
    _userModel.resetState(); //涉及到数据库中数据的操作都放在model层解决操作接口层)
}

//处理客户端异常退出(针对的是业务逻辑上的异常退出,检测到对端结束的处理方法)
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin(); it!=_userConnMap.end(); it++)
        {
            if(it->second == conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }

        }
    }//作用域

    //在redis中取消订阅
    _redis.unsubscribe(user.getId());

    //更新用户的状态信息
    if(user.getId()!=-1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }

}

//用户退出业务
void ChatService::loginout(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    
    //在redis中取消订阅
    _redis.unsubscribe(id);
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
        if(it !=_userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }


    User user(id,"","","offline");
    _userModel.updateState(user);

}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {   lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it!=_userConnMap.end())
        {
            //toid在线   //是否对端保持连接(当前对端断开连接时,客户端是主动断开,实际上的连接已经断开,
            //但是由于mutex的原因,_userConnMap中的连接信息没有断开，此时还可以发送数据吗,或则这些信息应该用于离线使用)
            //是否需要检测客户端是否断开
            it->second->send(js.dump());
            return;
        }
    }

    //查询toid是否在线
    User user = _userModel.query(toid);
    if(user.getState()=="online")
    {
        
        //toid在线
        if(_redis.publish(toid,js.dump()))
        {
            cout<<"进入publish"<<endl;
        }
        return;
    }



    //toid离线
    _offlineMsgModel.insert(toid,js.dump());
}

//添加好友关系  userid friendid
void ChatService::addFriend(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendModel.insert(userid, friendid);
}

//创建群组业务 groupname groupdesc
void ChatService::createGroup(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];
    Group group;
    group.setDes(groupdesc);
    group.setName(groupname);
    if(_groupModel.creatGroup(group))
    {
        _groupModel.addGroup(userid, group.getId(),"creator");
    }

}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid,"normal");
}

//群聊天业务
void ChatService::groupChat(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> vec;
    vec = _groupModel.queryGroupusers(userid, groupid);
    for(int id:vec)
    {   
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
        if(it!=_userConnMap.end())
        {
            it->second->send(js.dump());
        }else if(_userModel.query(id).getState()=="online")
        {
            _redis.publish(id, js.dump());
        }
        else
        {
            _offlineMsgModel.insert(id,js.dump());
        }
    }
}

void ChatService::handleRedisSubsribeMessage(int channel, string message)
{
    lock_guard<mutex>lock(_connMutex);
    auto it = _userConnMap.find(channel);
    if(it!=_userConnMap.end())
    {
        it->second->send(message);
        return;
    }

    //存储离线消息
    _offlineMsgModel.insert(channel, message);

}