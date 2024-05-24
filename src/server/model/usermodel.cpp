#include "usermodel.hpp"
#include "db.h"

//User表的增加方法
bool UserModel::insert(User& user)
{
    //构建SQL语句
    char sql[1024]={0};
    sprintf(sql,"insert into user(name, password, state) values('%s', '%s', '%s')", user.getName().c_str(), user.getPwd().c_str(), 
            user.getState().c_str());
    
    MySql mysql;
    bool stat = mysql.connect();

    if(stat)
    {
        if(mysql.update(sql))
        {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}


//根据用户号码查询用户信息
User UserModel::query(int id)
{
    //构建SQL语句
    char sql[1024]={0};
    sprintf(sql,"select * from user where id = %d", id);

    //建立MySql连接
    MySql mysql;
    if(mysql.connect())
    {
        MYSQL_RES * res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }

    }

    return User();
}

bool UserModel::updateState(User &user)
{
    //构建sql语句
    char sql[1024] ={0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(),user.getId());

    MySql mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;   
        }
    }
    return false;

}


void UserModel::resetState()
{
    char sql[1024]="update user set state = 'offline' where state = 'online'";
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}
    
