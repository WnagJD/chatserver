#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"


class UserModel
{
    public:
        //更新用户状态信息
        bool updateState(User& user);
        
        //增加User表
        bool insert(User& user);

        //根据用户号查询用户信息
        User query(int id);

        //重置用户的状态信息
        void resetState();

        //
};


#endif
