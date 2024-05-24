#include "json.hpp"
using json = nlohmann::json;
#include <string>
#include <iostream>
#include <vector>
#include <map>
using namespace std;

//json 序列化普通类型
string fun1()
{
    json js;
    js["msg_type"]=2;
    js["id"]="1234";
    js["name"]="frist name";
    // cout<<js<<endl;
    string sendbuf = js.dump(); //将json对象->json字符串(序列化)
    return sendbuf;
}

//json 序列化数组
string fun2()
{
    json js;
    js["id"]={1,2,3,4};
    js["msg"]["requets"]="need file";
    js["msg"]["response"]="no file";
    string s1=js.dump();
    return s1;
    // cout<<js<<endl;
    // cout<<s1<<endl;
    // cout<<s1.c_str()<<endl;
}

//json直接序列化容器
string fun3()
{
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    json js1;
    js1["vector"]=v;
    // cout<<js1<<endl;
    map<int, string> m;
    m.insert({1,"first"});
    m.insert({2, "second"});
    m.insert({3,"thrid"});

    // json js2;
    js1["map"]=m;
    // cout<<js2<<endl;
    // string sendbuf= js1.dump();
    // cout<<js1<<endl;
    // cout<<sendbuf.c_str()<<endl;
    string sendbuf = js1.dump();
    return sendbuf;
}

int main()
{
    string recvbuf = fun1();
    json recvjs = json::parse(recvbuf);
    cout<<recvjs["id"]<<endl;
    recvbuf = fun2();
    recvjs = json::parse(recvbuf);
    cout<<recvjs["msg"]<<endl;
    recvbuf = fun3();
    recvjs = json::parse(recvbuf);
    vector<int> v = recvjs["vector"];
    map<int,string> m = recvjs["map"];
    for(int&c: v)
    {
        cout<<c<<endl;
    }
    for(auto&m : m)
    {
        cout<<m.first<<" "<<m.second<<endl;
    }


    // fun3();

    return 0;

}