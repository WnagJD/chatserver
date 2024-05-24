#include "offlinemessagemodel.hpp"

 //存储离线信息
void OfflineMsgModel::insert(int userid, string msg)
{
    //构建sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into offlinemessage values(%d, '%s')", userid, msg.c_str());
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

//删除用户离线信息
void OfflineMsgModel::remove(int userid)
{
    char sql[1024]={0};
    sprintf(sql,"delete from offlinemessage where userid = %d", userid);
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }

}

//查询用户的离线信息
vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024]={0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);
    vector<string> vec;
    MySql mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
       
    }
    return vec;
}