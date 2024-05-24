#include "db.h"
#include <muduo/base/Logging.h>
using namespace muduo;

//数据库配置信息
static string server = "127.0.0.1";
static string user ="root";
static string password ="123456";
static string dbname = "chat";



MySql::MySql()
{
    _conn = mysql_init(nullptr);

}

MySql::~MySql()
{
    if(_conn!=nullptr)
    {
        mysql_close(_conn);
    }
}


bool MySql::connect()
{
    MYSQL* p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306,nullptr, 0);
    if(p!=nullptr)
    {
        mysql_query(_conn, "set names gbk");
        LOG_INFO<<"connect mysql success!";
    }else{
        LOG_INFO<<"connect mysql fail!";
    }
    //返回问题
    return p;
}

//更新操作
bool MySql::update(string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG_INFO << __FILE__ <<":"<<__FILE__<<":"
                 <<sql<<"更新失败";
    }
    return true;
}

//查询操作  __FILE__ 宏代表的是当前源文件所在路径
MYSQL_RES* MySql::query(string sql)
{
    if(mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO <<__FILE__<<":"<<__FILE__<<":"
                 <<sql<<"查询失败";
        return nullptr;
    }

    //从服务器存储查询到的数据到客户端，以便后续使用
    return mysql_use_result(_conn); 
}

//获取连接
MYSQL* MySql::getConnection()
{
    return _conn;
}