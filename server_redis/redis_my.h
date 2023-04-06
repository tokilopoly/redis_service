#include <string>
#include <vector>
#include <utility>
#include <sw/redis++/redis++.h>
#include <iostream>
#include    "str.h"
#include	"json.hpp"
#include	"to_base64.h"
#include	"headers.h"

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
	RedisManager(const std::string& host, int port = 6379) :_lock(), _check_lock(), _redis_lock() {
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

	void	init_phone_realphone() {
		std::map<std::string, std::string> hashs;
		redis_me->hgetall(phone_db_name, std::inserter(hashs, hashs.begin()));
		for (auto& in : hashs) {
			JSON	json = JSON::parse(in.second);
			outPhone_inphone.insert(std::pair<std::string, std::string>(json[REAL_PHONE_KEY], in.first));
		}
	}

	bool	exist_real_phone(std::string inside_number) {	//传进来一个外部号码
		if (outPhone_inphone.count(inside_number) == true)
			return true;
		return false;
	}

	std::string	get_realphone(std::string inside_number) {	//传进来一个外部号码 返回一个内部号码
		if (exist_real_phone(inside_number))
			return	outPhone_inphone.at(inside_number);
		return	"";
	}
	//STATUS_RedisManager    exist_real_phone(std::string inside_number) {
	//	try
	//	{
	//		std::lock_guard<std::mutex> guard(_redis_lock);
	//		//std::lock_guard<std::mutex> guard(_lock);
	//		std::map<std::string, std::string> hashs;
	//		redis_me->hgetall(phone_db_name, std::inserter(hashs, hashs.begin()));
	//		for (auto& in : hashs) {
	//			JSON	json = JSON::parse(in.second);
	//			if (json[REAL_PHONE_KEY] == inside_number)
	//				return STATUS_TRUE;
	//		}
	//		return STATUS_FALSE;
	//	}
	//	catch (const std::exception&)
	//	{
	//		return STATUS_ERROR;
	//	}
	//}

	bool    exist_user(std::string token) {
		std::lock_guard<std::mutex> guard(_check_lock);
		if (check_user_.exist_user_intoken(token))
			return true;
		else
			return false;
	}

	bool	Add_Served(std::string inside_number, std::string token) {
		try {
			// 使用 std::lock_guard 获取互斥量
			std::lock_guard<std::mutex> guard(_redis_lock);
			OptionalString strValue = redis_me->hget(phone_db_name, inside_number);
			JSON    data = JSON::parse(strValue->c_str());
			data[SERVED_KEY].push_back(token);
			redis_me->hset(phone_db_name, inside_number, data.dump());
		}
		catch (const Error& err) {
			std::cerr << "Failed to set key-value pair in Redis database: " << err.what() << '\n';
			return false;
		}
		return true;
	}

	STATUS_RedisManager	    has_Served(std::string inside_number, std::string token) {
		try {
			std::lock_guard<std::mutex> guard(_redis_lock);
			OptionalString strValue = redis_me->hget(phone_db_name, inside_number);
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
		std::lock_guard<std::mutex> guard(_lock);//同时刻只允许一个线程获取没服务的号码
		std::vector<redis_phone> ret;
		std::map<std::string, std::string> hashTerm;
		int  count = 1;
		try {
			redis_me->hgetall(phone_db_name, std::inserter(hashTerm, hashTerm.end()));
			for (auto& kv : hashTerm) {
				redis_phone	re_;
				re_.inside_number = kv.first;
				JSON    data = JSON::parse(kv.second);
				if (data[SERVED_KEY].size() == 0) {
					re_.outside_number = data[REAL_PHONE_KEY];
					ret.push_back(re_);
					while(!Add_Served(re_.inside_number, token));
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
						re_.outside_number = data[REAL_PHONE_KEY];
						ret.push_back(re_);
						while (!Add_Served(re_.inside_number, token));
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
		// 使用 std::lock_guard 获取互斥量
		std::lock_guard<std::mutex> guard(_check_lock);
		std::lock_guard<std::mutex> guard2(_redis_lock);
		auto hash = check_user_.Alloc_hash(token);
		//把hash存到users键中
		try
		{
			JSON	js;
			js["time_t"] = hash.user_info.t;
			js["hash"] = hash.hash;
			redis_me->hset(user_db_name, token, js.dump());
		}
		catch (const std::exception&)
		{
			return "error";
		}
		return hash.hash;
	}

	std::string	hash_get_token(std::string hash) {
		std::lock_guard<std::mutex> guard(_check_lock);
		auto c = check_user_.get_user_info(hash);
		return	c.token;
	}

	bool	check_user_exist_inhash(std::string hash) {
		std::lock_guard<std::mutex> guard(_check_lock);
		if (check_user_.exist_user_inhash(hash))
			return	true;

		return false;
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
				std::string inside_number = std::to_string(510011142857857);
				std::string outside_number =  std::to_string(6285711430788);
				JSON json;
				json[REAL_PHONE_KEY] = outside_number;
				json[SERVED_KEY] = JSON::array();
				redis_me->hset(phone_db_name, inside_number, json.dump());
				
				inside_number= std::to_string(510013341977798);
				outside_number = std::to_string(6285733751273);
				json[REAL_PHONE_KEY] = outside_number;
				json[SERVED_KEY] = JSON::array();
				redis_me->hset(phone_db_name, inside_number, json.dump());

				inside_number = std::to_string(510015232399215);
				outside_number =  std::to_string(6285852854414);
				json[REAL_PHONE_KEY] = outside_number;
				json[SERVED_KEY] = JSON::array();
				redis_me->hset(phone_db_name, inside_number, json.dump());

				inside_number = std::to_string(510019767102854);
				outside_number = std::to_string(6285697691608);
				json[REAL_PHONE_KEY] = outside_number;
				json[SERVED_KEY] = JSON::array();
				redis_me->hset(phone_db_name, inside_number, json.dump());

			for (int i = 0; i < 4; i++) {
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
		std::lock_guard<std::mutex> guard(_check_lock);
		return check_user_.get_user_info(hash);
	}

	bool	check_hash_exist(std::string hash) {}
	bool	check_token_exist(std::string hash) {}
private:
	std::mutex	_lock;
	std::mutex	_redis_lock;
	std::mutex	_check_lock;

	ConnectionOptions connection_options_;
	ConnectionPoolOptions pool_options_;

	std::unique_ptr<Redis> redis_me;
	const std::string user_db_name = "users";
	const std::string	phone_db_name = "phone";

	Check_User	check_user_;
	std::map<std::string, std::string>	outPhone_inphone;	//这个结构储存所有的返回给客户端的未服务号码列表
	//真实号码 和 内部号码
};
