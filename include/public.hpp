#ifndef PUBLIC_H
#define PUBLIC_H
enum EnMsgType{
    LOGIN_MSG=1,
    LOGIN_MSG_ACk,
    REG_MSG,
    REG_MSG_ACK,
    ONE_CHAT_MSG, //一对一聊天业务
    ADD_FRIEND_MSG,//添加好友关系
    CREATE_GROUP_MSG,//创建群组
    ADD_GROUP_MSG,//加入群组
    GROUP_CHAT_MSG,//群聊天
    LOGINOUT_MSG //用户退出

};

#endif