#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include <string>
#include <vector>
#include "db.h"
using namespace std;

class OfflineMsgModel
{
    public:
        //存储离线信息
        void insert(int userid, string msg);

        //删除用户离线信息
        void remove(int userid);

        //查询用户的离线信息
        vector<string> query(int userid);

};



#endif