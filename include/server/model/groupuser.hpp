#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"

class GroupUser:public User
{
    private:
        string role;
    public:
        void setRole(string rloe){this->role = role;}
        string getRole(){return role;}

};



#endif