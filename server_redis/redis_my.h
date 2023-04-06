#include <string>
#include <vector>
#include <utility>
#include <sw/redis++/redis++.h>
#include <iostream>
#include    "str.h"
#include	"json.hpp"
#include	"to_base64.h"

using JSON = nlohmann::json;
using namespace sw::redis;

#define	SERVED_KEY	"Served"
#define	REAL_PHONE_KEY	"Real_Phone"
class RedisManager {
public:
	enum STATUS_RedisManager
	{
		STATUS_ERROR = 0,
		STATUS_TRUE = 1,
		STATUS_FALSE = 2
	};
	RedisManager(const std::string& host, int port = 6379):_lock() {
		connection_options_.host = host;
		connection_options_.port = port;
		connection_options_.db = 0;
		pool_options_.size = 3;  // Pool size, i.e. max number of connections.
		pool_options_.wait_timeout = std::chrono::milliseconds(100);
	}

	bool Connect() {
		try {
			redis_me = std::make_unique<Redis>(connection_options_);
		}
		catch (const ReplyError& err) {
			printf("RedisHandler-- ReplyError：%s \n", err.what());
			return false;
		}
		catch (const TimeoutError& err) {
			printf("RedisHandler-- TimeoutError%s \n", err.what());
			return false;
		}
		catch (const ClosedError& err) {
			printf("RedisHandler-- ClosedError%s \n", err.what());
			return false;
		}
		catch (const IoError& err) {
			printf("RedisHandler-- IoError%s \n", err.what());
			return false;
		}

		catch (const Error& err) {
			std::cerr << "Failed to connect to Redis server: " << err.what() << '\n';
			return false;
		}
		return true;
	}

	void Disconnect() {
		redis_me.reset(nullptr);
	}

	void	init_check_user() {
		try
		{
			std::map<std::string, std::string> user_;
			redis_me->hgetall(user_db_name, std::inserter(user_, user_.begin()));
			for (auto in : user_) {
				JSON	js = JSON::parse(in.second);
				Check_User::USER_INFO_hash_time	init_strc;
				init_strc.hash = js["hash"];
				init_strc.user_info.t = js["time_t"];
				init_strc.user_info.token = in.first;
				check_user_.init_program_now_user(init_strc);
			}
		}
		catch (const std::exception&)
		{

		}
	}
	STATUS_RedisManager    exist_real_phone(std::string phone) {
		try
		{
			std::map<std::string, std::string> hashs;
			redis_me->hgetall(phone_db_name, std::inserter(hashs, hashs.begin()));
			for (auto& in : hashs) {
				JSON	json = JSON::parse(in.second);
				if (json[REAL_PHONE_KEY] == phone)
					return STATUS_TRUE;
			}
			return STATUS_FALSE;
		}
		catch (const std::exception&)
		{
			return STATUS_ERROR;
		}
	}

	STATUS_RedisManager    exist_user(std::string token) {
		try
		{
			if (check_user_.exist_user_intoken(token))
				return STATUS_TRUE;
			else
				return STATUS_FALSE;
		}
		catch (const std::exception&)
		{
			return STATUS_ERROR;
		}
	}
	
	STATUS_RedisManager	Add_Served(std::string phone, std::string token) {
		try {
			std::lock_guard<std::mutex> guard(_lock);
			OptionalString strValue = redis_me->hget(phone_db_name, phone);
			JSON    data = JSON::parse(strValue->c_str());
			data[SERVED_KEY].push_back(token);
			// 使用 std::lock_guard 获取互斥量
			redis_me->hset(phone_db_name, phone, data.dump());
		}
		catch (const Error& err) {
			std::cerr << "Failed to set key-value pair in Redis database: " << err.what() << '\n';
			return STATUS_ERROR;
		}
		return STATUS_TRUE;
	}
	
	STATUS_RedisManager	    has_Served(std::string phone, std::string token) {
		try {
			OptionalString strValue = redis_me->hget(phone_db_name, phone);
			JSON    data = JSON::parse(strValue->c_str());
			for (int i = 0; i < data[SERVED_KEY].size(); i++) {
				if (data[SERVED_KEY][i] == token)
					return  STATUS_TRUE;
			}
		}
		catch (const Error& err) {
			std::cerr << "Failed to set key-value pair in Redis database: " << err.what() << '\n';
			return STATUS_ERROR;
		}
		return  STATUS_FALSE;
	}

	std::vector<redis_phone> get_noServed_item(std::string token, int items_count = 0) {
		std::vector<redis_phone> ret;
		std::map<std::string, std::string> hashTerm;
		int  count = 1;
		try {
			redis_me->hgetall(phone_db_name, std::inserter(hashTerm, hashTerm.end()));
			for (auto& kv : hashTerm) {
				redis_phone	re_;
				re_.phone = kv.first;
				JSON    data = JSON::parse(kv.second);
				if (data[SERVED_KEY].size() == 0) {
					re_.real_phone = data[REAL_PHONE_KEY];
					ret.push_back(re_);
					Add_Served(re_.phone, token);
					count++;
				}
				else {
					bool	has_ = true;
					for (int j = 0; j < data[SERVED_KEY].size(); j++) {
						if (data[SERVED_KEY][j] == token) {
							has_ = false;
						}
					}
					if (has_) {
						re_.real_phone = data[REAL_PHONE_KEY];
						ret.push_back(re_);
						Add_Served(re_.phone, token);
						count++;
					}
				}
				if (items_count != 0) {
					if (items_count < count)
						break;
				}
			}
		}
		catch (const Error& err) {
			std::cerr << "Failed to set key-value pair in Redis database: " << err.what() << '\n';
			return ret;
		}
		return ret;
	}

	std::string	Alloc_Hash_in_token(std::string token) {
		auto hash = check_user_.Alloc_hash(token);
		//把hash存到users键中
		try
		{
			JSON	js;
			js["time_t"] = hash.user_info.t;
			js["hash"] = hash.hash;
			// 使用 std::lock_guard 获取互斥量
			std::lock_guard<std::mutex> guard(_lock);
			redis_me->hset(user_db_name, token, js.dump());
		}
		catch (const std::exception&)
		{
			return "error";
		}
		return hash.hash;
	}

	std::string	hash_get_token(std::string hash) {
		auto c = check_user_.get_user_info(hash);
		return	c.token;
	}
	int		check_user_exist_inhash(std::string hash) {
		try
		{
			if (check_user_.exist_user_inhash(hash))
				return	STATUS_TRUE;
		}
		catch (const std::exception&)
		{
			return STATUS_ERROR;
		}
		return STATUS_FALSE;
	}

	bool    _clear_all() {
		try {
			redis_me->del(user_db_name);
			redis_me->del(phone_db_name);
		}
		catch (const Error& err) {
			std::cerr << "Failed to set key-value pair in Redis database: " << err.what() << '\n';
			return false;
		}
		return true;
	}
	bool	_add_user_and_phone() {
		try {
			for (int i = 10000; i < 10100; i++) {
				std::string phone = std::to_string(i);
				std::string real_phone = phone + phone + std::to_string(i%10000);
				JSON json;
				json[REAL_PHONE_KEY] = real_phone;
				json[SERVED_KEY] = JSON::array();
				redis_me->hset(phone_db_name, phone, json.dump());
			}
			for (int i = 0; i < 10; i++) {
				std::string user = "user" + std::to_string(i);
				//user = base64_encode(user.c_str(), user.length());
				auto user_info = check_user_.Alloc_hash(user);
				JSON js;
				js["hash"] = user_info.hash;
				js["time_t"] = user_info.user_info.t;
				redis_me->hset(user_db_name, user, js.dump());
			}
		}
		catch (const Error& err) {
			std::cerr << "Failed to set key-value pair in Redis database: " << err.what() << '\n';
			return false;
		}
		return true;
	}


	Check_User::USER_INFO& get_user_info(std::string hash) {
		return check_user_.get_user_info(hash);
	}

	bool	check_hash_exist(std::string hash) {}
	bool	check_token_exist(std::string hash) {}
private:
	std::mutex	_lock;

	ConnectionOptions connection_options_;
	ConnectionPoolOptions pool_options_;

	std::unique_ptr<Redis> redis_me;
	const std::string user_db_name = "users";
	const std::string	phone_db_name = "phone";

	Check_User	check_user_;
};
