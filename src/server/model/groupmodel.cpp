#include "groupmodel.hpp"
#include "db.h"

//创建群组
bool GroupModel::creatGroup(Group& group)
{
    char sql[1024]={0};
    sprintf(sql,"insert into allgroup(groupname, groupdesc) values('%s','%s')", group.getName().c_str(), group.getDes().c_str());
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
        group.setId(mysql_insert_id(mysql.getConnection()));
        return true;
    }
    return false;

}

//加入群组
void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024]={0};
    sprintf(sql,"insert into groupuser values(%d,%d,'%s')", groupid, userid, role.c_str());
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }

}
//查询用户所在的群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024]={0};
    sprintf(sql,"select a.id, a.groupname,a.groupdesc from allgroup a inner join groupuser b on b.groupid = a.id where b.userid = %d", userid);
    vector<Group> groups;
    MySql mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        while((row=mysql_fetch_row(res))!=nullptr)
        {
            Group  group;
            group.setId(atoi(row[0]));
            group.setName(row[1]);
            group.setDes(row[2]);
            groups.push_back(group);
        }
        mysql_free_result(res);

    }

    for(Group& group:groups)
    {
        sprintf(sql,"select a.id, a.name,a.state,b.grouprole from user a inner join groupuser b on b.userid = a.id where b.groupid=%d", group.getId());
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        while((row=mysql_fetch_row(res))!=nullptr)
        {
            GroupUser user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setName(row[2]);
            user.setRole(row[3]);
            group.getUser().push_back(user);
        }
        mysql_free_result(res);
    }

    return groups;

}
//根据指定的groupid查询群组内的成员，除了userid自己,用于用户群聊时，发送信息给群内其他成员
vector<int> GroupModel::queryGroupusers(int userid, int groupid)
{
    char sql[1024]={0};
    sprintf(sql,"select userid from groupuser where groupid=%d and userid !=%d", groupid, userid);
    vector<int> userids;
    MySql mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        while((row=mysql_fetch_row(res))!=nullptr)
        {
            userids.push_back(atoi(row[0]));
        }
        mysql_free_result(res);
    }
    return userids;
}