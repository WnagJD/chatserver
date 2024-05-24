#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>
#include "groupuser.hpp"
using namespace std;

class Group
{
    public:
        Group(int id=-1, string name = "", string des="")
        {
            this->id = id;
            this->name = name;
            this->des = des;
        }

        void setId(int id){this->id = id;}
        void setName(string name){this->name = name;}
        void setDes(string des){this->des = des;}

        int getId(){return id;}
        string getName(){return name;}
        string getDes(){return des;}
        vector<GroupUser>& getUser(){return users;}

    private:
        int id;
        string name;
        string des;
        //组成员信息
        vector<GroupUser> users;
};


#endif