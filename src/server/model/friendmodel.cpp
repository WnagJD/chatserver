#include "friendmodel.hpp"
#include "db.h"

//添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    char sql[1024]={0};
    sprintf(sql,"insert into friend values(%d,%d)", userid, friendid);
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }

}

//返回好友列表
vector<User> FriendModel::query(int userid)
{
    char sql[1024]={0};
    sprintf(sql,"select a.id, a.name, a.state from user a inner join friend b on b.friendid = a.id where userid = %d", userid);
    vector<User> users;
    MySql mysql;
    if(mysql.connect())
    {
        MYSQL_RES * res = mysql.query(sql);
        MYSQL_ROW row;
        while((row=mysql_fetch_row(res))!=nullptr)
        {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setState(row[2]);
            users.push_back(user);
        }
        mysql_free_result(res);
    }
    return users;
}