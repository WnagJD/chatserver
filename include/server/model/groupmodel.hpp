#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.hpp"

class GroupModel
{
    public:
        //创建群组
        bool creatGroup(Group& group);

        //加入群组
        void addGroup(int userid, int groupid, string role);

        //查询用户所在的群组信息
        vector<Group> queryGroups(int userid);

        //根据指定的groupid查询群组内的成员，除了userid自己,用于用户群聊时，发送信息给群内其他成员
        vector<int> queryGroupusers(int userid, int groupid);

};

#endif