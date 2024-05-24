#include "redis.hpp"
#include <thread>

Redis::Redis():_publish_context(nullptr),_subscribe_context(nullptr)
{
}

Redis::~Redis()
{
    if(_publish_context!=nullptr)
    {
        redisFree(_publish_context);
    }

    if(_subscribe_context!=nullptr)
    {
        redisFree(_subscribe_context);
    }

}


bool Redis::connect()
{
    //用于向redis发布的连接
    _publish_context = redisConnect("127.0.0.1", 6379);
    if(_publish_context==nullptr)
    {
        cerr<<"redis connect failed!"<<endl;
        return false;
    }
    //用于向redis注册的连接
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if(_subscribe_context==nullptr)
    {
        cerr<<"redia connect failed!"<<endl;
        return false;
    }

    //启动线程用于接受redis相应的消息
    //在单独的线程中，监听通道时间，有消息到达上报给业务层
    thread t([&](){
        observer_channel_message();
    });
    t.detach();
    cout<<"connect redis-server success!"<<endl;
    return true;

}

//向redis指定的channel通道发布消息
bool Redis::publish(int channel, string message)
{
    redisReply * reply = (redisReply*)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if(reply == nullptr)
    {
        cerr<<"publish command failed!"<<endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}


//向redis指定的通道channel订阅
bool Redis::subscribe(int channel)
{
    if(REDIS_ERR == redisAppendCommand(_subscribe_context,"SUBSCRIBE %d", channel))
    {
        cerr<<" subscribe command failed!"<<endl;
        return false;
    }

    int done =0;
    while(!done)
    {
        if(REDIS_ERR == redisBufferWrite(_subscribe_context, &done))
        {
            cerr<<"subscribe command failed!"<<endl;
            return false;
        }
    }

    return true;
}

//向redis指定的通道channel取消订阅
bool Redis::unsubscribe(int channel)
{
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context,"UNSUBSCRIBE %d", channel))
    {
        cerr<<" subscribe command failed!"<<endl;
        return false;
    }

    int done =0;
    while(!done)
    {
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        {
            cerr<<"subscribe command failed!"<<endl;
            return false;
        }
    }

    return true;
}

//在独立线程中接受订阅通道的消息
void Redis::observer_channel_message()
{
    redisReply * reply=nullptr;
    cout<<"进入notify1.0"<<endl;
    while(REDIS_OK == redisGetReply(this->_subscribe_context, (void **)reply))
    {
        cout<<"进入notify1.1"<<endl;
        if(reply!=nullptr)
        {
            cout<<"reply not nullptr"<<endl;
        }
        // if(reply->element[2]!=nullptr)
        // {
        //     cout<<"reply->element[2] nullptr"<<endl;
        // }
        // if(reply->element[2]->str!=nullptr)
        // {
        //     cout<<"reply->element[2]->str nullptr"<<endl;
        // }

        if(reply!=nullptr && reply->element[2]!=nullptr && reply->element[2]->str!=nullptr)
        {
            //调用通知信息处理函数
            cout<<"进入notify1.2"<<endl;
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    cerr<<">>>>>>>>>>>>>>>observer_channel_message quit <<<<<<<<<<<<<<<<<<<<<<"<<endl;

}

void Redis::init_notify_handler(function<void(int, string)> fn)
{
    this->_notify_message_handler = fn;
}