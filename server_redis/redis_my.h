#ifndef _REDIS_H_
#define _REDIS_H_

#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include	"json.hpp"
#include    "str.h"

using JSON = nlohmann::json;
#include    <hiredis-1.1.0/hiredis.h>


class Redis
{
public:

    Redis() {}

    ~Redis()
    {
        this->_connect = NULL;
        this->_reply = NULL;
    }

    bool connect(std::string host, int port)
    {
        this->_connect = redisConnect(host.c_str(), port);
        if (this->_connect != NULL && this->_connect->err)
        {
            printf("connect error: %s\n", this->_connect->errstr);
            return 0;
        }
        return 1;
    }

    std::vector<redis_phone>    getAll_noServed(std::string token,int cursor = 0) {
        std::vector<redis_phone> ret;
        redisReply* reply = (redisReply*)redisCommand(this->_connect, "HGETALL phone");
        if (reply == NULL) {
            std::cout << "Failed to execute Redis command" << std::endl;
            return ret;
        }
        if (reply->type == REDIS_REPLY_ARRAY) {
            for (int i = 0; i < reply->elements; i += 2) {
                JSON    data = JSON::parse(reply->element[i + 1]->str);
                if (data["Served"].size() == 0) {
                    redis_phone re;
                    re.real_phone = data["real_phone"];
                    re.phone = reply->element[i]->str;
                    ret.push_back(re);
                    add_phone_Served(re.phone, token);
                    continue;
                }
                for (int j = 0; j < data["Served"].size(); j++) {
                    if (data["Served"][j] == token)
                        continue;
                    else {
                        redis_phone re;
                        re.real_phone = data["real_phone"];
                        re.phone = reply->element[i]->str;
                        ret.push_back(re);
                        add_phone_Served(re.phone, token);
                    }
                }
            }
        }
        freeReplyObject(reply);
        return ret;
    }

    bool    has_Served(std::string phone, std::string token) {
        redisReply* reply = (redisReply*)redisCommand(this->_connect, "HGET phone %s", phone.c_str());
        JSON    data = JSON::parse(reply->str);
        freeReplyObject(reply);
        for (int i = 0; i < data["Served"].size(); i++) {
            if (data["Served"][i] == token)
                return  true;
        }
        return  false;
    }
    void    add_phone_Served(std::string phone, std::string token) {
        redisReply* reply = (redisReply*)redisCommand(this->_connect, "HGET phone %s", phone.c_str());
        JSON    data= JSON::parse(reply->str);
        data["Served"].push_back(token);
        freeReplyObject(reply);
        reply = (redisReply*)redisCommand(this->_connect, "HSET phone %s %s", phone.c_str(), data.dump().c_str());
        freeReplyObject(reply);
    }
    void    add_phone(std::string phone,std::string real_phone) {
        JSON    data ;
        data["Served"] = JSON::array();
        data["real_phone"] = real_phone;
        redisReply* reply = (redisReply*)redisCommand(this->_connect, "HSET phone %s %s", phone.c_str(), data.dump().c_str());
        freeReplyObject(reply);
    }
    bool    exist_phone(std::string phone) {
        redisReply* reply = (redisReply*)redisCommand(this->_connect, "HEXISTS phone %s", phone.c_str());
        int ret = reply->integer;
        freeReplyObject(reply);
        return  ret;
    }
    bool    exist_user(std::string token) {
       redisReply* reply = (redisReply*)redisCommand(this->_connect, "SISMEMBER %s %s", user_db_name.c_str(),token.c_str());
       int ret = reply->integer;
       freeReplyObject(reply);
       return ret;
    }
    void    add_user(std::string user) {
        redisCommand(this->_connect, "sadd %s %s",  user_db_name.c_str(),user.c_str());
    }

    void    clear_all() {
        redisReply* reply = (redisReply*)redisCommand(this->_connect, "DEL users");
        if (reply == NULL || reply->type != REDIS_REPLY_INTEGER) {
            std::cerr << "DEL error: " << this->_connect->errstr << std::endl;
        }
        freeReplyObject(reply);
        
        reply = (redisReply*)redisCommand(this->_connect, "DEL phone");
        if (reply == NULL || reply->type != REDIS_REPLY_INTEGER) {
            std::cerr << "DEL error: " << this->_connect->errstr << std::endl;
        }
        freeReplyObject(reply);
    }
private:
    std::string user_db_name = "users";
    redisContext* _connect;
    redisReply* _reply;
};

#endif 
