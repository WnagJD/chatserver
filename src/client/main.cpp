#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <semaphore.h>
#include <chrono>
#include <ctime>

#include "public.hpp"
#include "user.hpp"
#include "group.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


using namespace std;
using namespace placeholders;
using json = nlohmann::json;

//记录当前用户的信息用于显示
User g_currentUser;

//记录当前登录用户的好友信息列表
vector<User> g_currentUserFriendList;

//记录当前用户的群组列表信息
vector<Group> g_currentUserGroupList;

//用于控制读写线程的信号量
sem_t rwsem;

//记录登录状态
bool g_isLoginSuccess;

//控制主菜单页面程序
bool isMainMenuRunning = false;

//读线程处理函数
void readTaskHandler(int clientfd);

//显示当前用户登录的基本信息
void showCurrentUserData();

//获取当前的系统时间
string getCurrentTime();

//业务程序界面
void mainMenu(int clientfd);

int main(int argc, char*argv[])
{
    if(argc<3)
    {
        cerr<<"command invalid! example:./ChatClient 127.0.0.1 6000"<<endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    sockaddr_in clientaddr;
    clientaddr.sin_family= AF_INET;
    clientaddr.sin_port = htons(port);
    //inet_addr:将IPV4的点分十进制字符串转换为网络字节序
    clientaddr.sin_addr.s_addr = inet_addr(ip);

    int socketfd = socket(PF_INET, SOCK_STREAM, 0);
    if(socketfd == -1)
    {
        cerr<<"create socket failure!"<<endl;
        exit(-1);
    }
    
    int ret = connect(socketfd,(sockaddr*)&clientaddr,sizeof(clientaddr));
    if(ret == -1)
    {
        cerr<<"connect server failure!"<<endl;
    }

    //初始化信号量
    sem_init(&rwsem, 0, 0);

    //启动读线程
    thread readTask(readTaskHandler, socketfd);
    readTask.detach();

    //进入主界面
    for(;;)
    {
        //显示首先的菜单选项
        cout<<"================================="<<endl;
        cout<<"1. login"<<endl;
        cout<<"2. register"<<endl;
        cout<<"3. quit"<<endl;
        cout<<"================================="<<endl;
        cout<<"choice:";
        int choice =0;
        cin>>choice;
        cin.get();//读掉缓冲区中残留的回车

        switch(choice)
        {
            case 1://login 业务
            {
                int id=0;
                char pwd[50]={0};
                cout<<"userid:";
                cin>>id;
                cin.get();
                cout<<"password:";
                cin.getline(pwd, 50);

                json js;
                js["msgid"]=LOGIN_MSG;
                js["id"]=id;
                js["password"]= string(pwd);

                string request = js.dump();

                g_isLoginSuccess = false;

                int ret = send(socketfd, request.c_str(),strlen(request.c_str())+1,0);
                if(ret == -1)
                {
                    cerr<<"send login msg error:"<<request<<endl;
                    exit(-1);
                }
                //等待信号量,等待读线程处理完毕
                sem_wait(&rwsem);

                if(g_isLoginSuccess)
                {
                    //进入聊天主界面
                    isMainMenuRunning = true;
                    mainMenu(socketfd);
                }

            }
            break;

            case 2://注册业务
            {
                char name[50]={0};
                char password[50]={0};
                cout<<"username:";
                cin.getline(name, 50);
                cout<<"password";
                cin.getline(password, 50);

                json js;
                js["msgid"]=REG_MSG;
                js["name"]=name;
                js["password"]=password;

                string request = js.dump();
                int ret = send(socketfd, request.c_str(), strlen(request.c_str())+1, 0);
                if(ret == -1)
                {
                    cerr<<"send reg msg error!"<<endl;
                }
                //等待信号量
                sem_wait(&rwsem);
            }
            break;

        case 3://退出业务
            {
                close(socketfd);
                sem_destroy(&rwsem);
                exit(0);

            }
            break;

        default : 
                cerr<<"invalid input!"<<endl;
                break;
        }
    }

    return 0;
}


//处理登录的响应逻辑
void doLoginResponse(json& responsejs)
{
   
    if(responsejs["errno"].get<int>()!=0)
    {
        
        cerr<<responsejs["errmsg"]<<endl;
        g_isLoginSuccess = false;
    }
    else
    {
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);
        // g_currentUser.setState(responsejs["state"]);
        
        //记录当前好友信息
        if(responsejs.contains("friends"))
        {
            g_currentUserFriendList.clear();
            vector<string> friendusres = responsejs["friends"];
            for(string &user : friendusres)
            {
                json js = json::parse(user);
                User users;
                users.setId(js["id"].get<int>());
                users.setName(js["name"]);
                users.setState(js["state"]);
                g_currentUserFriendList.push_back(users);
            }
            
        }
        
        if(responsejs.contains("groups"))
        {
            
            g_currentUserGroupList.clear();
            vector<string> grouplists = responsejs["groups"];
            
            for(string & str:grouplists)
            {
                Group grp;
                json grpjs = json::parse(str);
                grp.setId(grpjs["groupid"].get<int>());
                grp.setName(grpjs["groupname"]);
                grp.setDes(grpjs["groupdesc"]);
                
                vector<string> user = grpjs["users"];
                for( string & ur: user)
                {
                    GroupUser guser;
                    json js = json::parse(ur);
                    guser.setId(js["id"].get<int>());
                    guser.setName(js["name"]);
                    guser.setState(js["state"]);
                    guser.setRole(js["role"]);
                    grp.getUser().push_back(guser);
                }
                g_currentUserGroupList.push_back(grp);
                 
            }
        }
        
        //显示登录用户的基本信息
        showCurrentUserData();

        //显示当前用户的离线信息,个人聊天信息或者群组聊天信息
        if(responsejs.contains("offlinemsg"))
        {
            vector<string> offlinemgss = responsejs["offlinemsg"];
            for(string & str:offlinemgss)
            {
                json js = json::parse(str);
                if(js["mgsid"]==ONE_CHAT_MSG)
                {
                    cout<<js["time"].get<string>()<<"[" <<js["id"]<<"]"<<js["name"].get<string>()<<"said:"<<js["msg"].get<string>()<<endl;

                }else
                {
                    cout<<"群聊天["<<js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                            << " said: " << js["msg"].get<string>() << endl;
                }
            }

        }
        
        g_isLoginSuccess = true;
    }

}

//处理注册业务逻辑
void doRegResponse(json& responsejs)
{
    if(responsejs["errno"].get<int>()!=0)
    {
        cerr<<responsejs["errmsg"]<<endl;
    }
    else{
       cout<<"name register success! userid is ="<<responsejs["id"].get<int>()<<", do not forget it !";
    }
}

//子线程的接收函数
void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buf[1024]={0};
        int len = recv(clientfd, buf, 1024, 0);
        if(len == -1 || len ==0)
        {
            close(clientfd);
            exit(-1);
        }
        
        string str(buf);
        json js = json::parse(str);
        int msgid = js["msgid"].get<int>();
        
        switch(msgid)
        {
            case ONE_CHAT_MSG:
            {
                cout<<js["time"].get<string>()<<"["<<js["id"].get<int>()<<"]"<<js["name"].get<string>()
                <<" said: "<<js["msg"].get<string>()<<endl;
                break;
            }
            case GROUP_CHAT_MSG:
            {
                cout<<"群消息["<<js["groupid"].get<int>()<<"]: "<<js["time"].get<string>()<<"["<<js["id"].get<int>()<<"]"<<js["name"].get<string>()
                <<" said: "<<js["msg"].get<string>()<<endl;
                break;
            }
            case LOGIN_MSG_ACk: 
            {
                
                doLoginResponse(js);
                sem_post(&rwsem); //通知主线程,登录的结果
                break;
            }
            case REG_MSG_ACK:
            {
                doRegResponse(js);
                sem_post(&rwsem); //通知主线程，注册的结果
                break;
            }
        }
    }

}


void showCurrentUserData()
{
    cout<<"========================login user========================"<<endl;
    cout<<"current login user => id:"<<g_currentUser.getId()<<" name:"<<g_currentUser.getName()<<endl;
    cout<<"------------------------friend list------------------------"<<endl;
    if(!g_currentUserFriendList.empty())
    {
        for(User &user : g_currentUserFriendList)
        {
            cout<<user.getId()<<" "<<user.getName()<<" "<<user.getState()<<endl;
        }
    }
    cout<<"------------------------group list------------------------"<<endl;
    if(!g_currentUserGroupList.empty())
    {
        for(Group& group : g_currentUserGroupList)
        {
            cout<<group.getId()<<" "<<group.getName()<<endl;
            for(GroupUser& user : group.getUser())
            {
                cout<<user.getId()<<" "<<user.getName()<<" "<<user.getState()
                <<" "<< user.getRole()<<endl;
            }
        }
    }
    cout<<"=========================================================="<<endl;
}

//"help" command handler
void help(int fd =0, string str="");
//"chat" command handler
void chat(int, string);
//"addfriend" command handler
void addfriend(int, string);
//"creategroup" command handler
void creategroup(int, string);
//"addgroup" command handler
void addgroup(int, string);
//"groupchat" command handler
void groupchat(int, string);
//"login" command handler
void loginout(int, string);

//系统支持的客户端命令列表
unordered_map<string, string> commandMap{
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}
    };

//注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap
{
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}
};

//主聊天程序页面
void mainMenu(int clientfd)
{
    help();
    char buffer[1024]={0};
    while(isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuffer(buffer);
        string command;//存储命令
        int idx = commandbuffer.find(":");
        if(idx ==-1)
        {
            command = commandbuffer;
        }
        else
        {
            command = commandbuffer.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cerr<<"invalid input command!"<<endl;
            continue;;
        }

        cout<<commandbuffer.substr(idx+1, commandbuffer.size()-idx)<<endl;
        it->second(clientfd, commandbuffer.substr(idx+1, commandbuffer.size()-idx));
    }
}


//"help" command handler
void help(int fd, string str)
{
    cout<<"show command list>>>"<<endl;
    for(auto& it : commandMap)
    {
        cout<<it.first<<":"<<it.second<<endl;
    }
    cout<<endl;
}
//"chat" command handler
void chat(int clientfd, string command) 
{
    int idx = command.find(":");
    if(idx == -1)
    {
        cout<<"chat command invalid!"<<endl;
        return;
    }
    int userid = atoi(command.substr(0,idx).c_str());
    string message = command.substr(idx+1, command.size()-idx);
    json js;
    js["msgid"]=ONE_CHAT_MSG;
    js["id"]=g_currentUser.getId();
    js["name"]=g_currentUser.getName();
    js["toid"]=userid;
    js["time"]=getCurrentTime();
    js["msg"]=message;
    
    string str = js.dump();
    int len = send(clientfd, str.c_str(), strlen(str.c_str())+1, 0);
    if(len == -1)
    {
        cerr<<"send chat msg error ->"<<str<<endl;
    }
}
//"addfriend" command handler
void addfriend(int clientfd, string command)
{
    int friendid = atoi(command.c_str());
    json js;
    js["msgid"]=ADD_FRIEND_MSG;
    js["id"]=g_currentUser.getId();
    js["friendid"]=friendid;
    string buf = js.dump();
    int len = send(clientfd, buf.c_str(), strlen(buf.c_str())+1, 0);
    if(len ==-1)
    {
        cerr<<"send addfriend msg error ->"<<buf<<endl;
    }
}
//"creategroup" command handler
void creategroup(int clientfd, string command)
{
    int idx = command.find(":");
    if(idx == -1)
    {
        cerr<<"creategroup invaild error!"<<endl;
        return ;
    }
    string groupname = command.substr(0, idx);
    string groupdesc = command.substr(idx+1, command.size()-idx);//长度
    json js;
    js["msgid"]=CREATE_GROUP_MSG;
    js["id"]=g_currentUser.getId();
    js["groupname"]=groupname;
    js["groupdesc"]=groupdesc;

    string buf = js.dump();
    int len = send(clientfd, buf.c_str(), strlen(buf.c_str())+1, 0);
    if(len == -1)
    {
        cerr<<"send creategroup msg error ->"<<buf<<endl;
    }

}
//"addgroup" command handler
void addgroup(int clientfd, string command)
{
    int groupid = atoi(command.c_str());
    json js;
    js["msgid"]=ADD_GROUP_MSG;
    js["id"]=g_currentUser.getId();
    js["groupid"]=groupid;
    string buf = js.dump();

    int len = send(clientfd, buf.c_str(), strlen(buf.c_str())+1, 0);
    if(len == -1)
    {
        cerr<<"send addgroup msg error ->"<<buf<<endl;
    }

}
//"groupchat" command handler
void groupchat(int clientfd, string command)
{
    int idx = command.find(":");
    if(idx == -1)
    {
        cerr<<"groupchat invaild error!"<<endl;
        return ;
    }
    int groupid = atoi(command.substr(0, idx).c_str());
    string message = command.substr(idx+1, command.size()-idx);//长度
    json js;
    js["msgid"]=GROUP_CHAT_MSG;
    js["id"]=g_currentUser.getId();
    js["name"]=g_currentUser.getName();
    js["groupid"]=groupid;
    js["msg"]=message;
    js["time"]=getCurrentTime();

    string buf = js.dump();
    int len = send(clientfd, buf.c_str(), strlen(buf.c_str())+1, 0);
    if(len == -1)
    {
        cerr<<"groupchat msg error ->"<<buf<<endl;
    }
}
//"login" command handler
void loginout(int clientfd, string commnd)
{
    json js;
    js["msgid"]=LOGINOUT_MSG;
    js["id"]=g_currentUser.getId();
    string buf = js.dump();

    int len = send(clientfd, buf.c_str(), strlen(buf.c_str())+1, 0);
    if(len == -1)
    {
        cerr<<"send loginout msg error ->"<<buf<<endl;
    }
    else
    {
        isMainMenuRunning = false;
    }

}

//获取系统当前时间
string getCurrentTime()
{
    //获取到当前时间的时间戳
    auto it = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&it);
    char data[60]={0};
    sprintf(data, "%d-%02d-%02d-%02d-%02d-%02d", ptm->tm_year+1990, ptm->tm_mon+1, ptm->tm_mday,
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
        
    return std::string(data);

}