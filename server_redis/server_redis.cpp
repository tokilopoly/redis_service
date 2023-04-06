#include <iostream>
#include <string>
#include <sw/redis++/redis++.h>
#include <sw/redis++/connection.h>
#include "redis_my.h"
#include <locale>
#include <codecvt>
#include <hv/HttpServer.h>
#include "json.hpp"
#include	"udpClient.h"
using namespace sw::redis;
using namespace hv;
using JSON = nlohmann::json;


int main(int argc, char** argv) {
	HV_MEMCHECK;
	
	MessageClient	mess_client;
	std::vector<std::string> inside_number;
	int port = 8080;
	if (argc > 1) {
		port = atoi(argv[1]);
	}

	HttpService router;
	RedisManager	Redis_me("127.0.0.1");
	Redis_me.Connect();

	Redis_me._clear_all();			//测试,先把用户 和 手机号写入数据库
	Redis_me._add_user_and_phone();
	Redis_me.init_check_user();	//从数据库把所有users用户信息保存到程序里
	Redis_me.init_phone_realphone();//从数据库把所有号码对应真实号码保存到程序里

#pragma region REDIS
	router.GET("/can_recv", [&](const HttpContextPtr& ctx) {
		JSON resp;
		std::string hash = ctx->request->GetParam("hash");
		if (!hash.empty()) {
			resp["hash"] = hash;
			resp["no_served"] = JSON::array();
			if (Redis_me.check_user_exist_inhash(hash) ==false) {
				resp["invalid"] = false;
				return ctx->send(resp.dump());
			}
			std::string user_token = Redis_me.hash_get_token(hash);
			resp["invalid"] = true;
			auto all = Redis_me.get_noServed_item(user_token, 10);
			for (auto a : all) {
				resp["no_served"].push_back(a.outside_number);
			}
		}
		return ctx->send(resp.dump());
		});

	router.POST("/recv_mess", [&](const HttpContextPtr& ctx) {
		JSON request_json;
		JSON response_json;
		try
		{
			request_json=JSON::parse(ctx->request->body);
		}
		catch (JSON::exception& e)
		{
			response_json["error"] = "not json";	//json解析失败
			return ctx->send(response_json.dump());
		}
		try
		{
			std::string s = request_json.dump();
			//hash或者 list为空
			if (request_json["hash"].get<std::string>().empty() || request_json["list"].size() == 0 || \
				request_json["key"].get<std::string>().empty()) {
				response_json["error"] = "hash or phone_list or key == empty";
				return ctx->send(response_json.dump());
			}
		}
		catch (JSON::exception& e)
		{
			response_json["error"] = "hash or phone_list or key == empty";
			return ctx->send(response_json.dump());
		}
		
		//检查hash有效性
		if (Redis_me.check_user_exist_inhash(request_json["hash"].get<std::string>()) ==false) {
			response_json["error"] = "not hash";
			return ctx->send(response_json.dump());
		}
		std::vector<redis_phone>	phone_pair;	//把外部号码对应的内部号码保存到这个结构中

		//检查phone_list中phone是否真实存在
		bool	_noExist = false;
		std::vector<std::string> _noExist_list;
		for (auto i : request_json["list"].get<std::vector<std::string>>()) {
			std::string in_num = Redis_me.get_realphone(i);
			if (in_num.empty()) {
				_noExist = true;
				_noExist_list.push_back(i);
			}
			else 
				phone_pair.push_back({i,in_num});
		}
		if (_noExist) {
			response_json["error_"] = JSON::array();
			response_json["error"] ="no exist";
			for (auto o : _noExist_list)
				response_json["error_"].push_back(o);
			return ctx->send(response_json.dump());
		}

		auto user_info = Redis_me.get_user_info(request_json["hash"].get<std::string>());
		bool	_noServed = false;
		std::vector<std::string> _noReverd_list;
		for (auto i : phone_pair) {
			if (Redis_me.has_Served(i.inside_number, user_info.token) != RedisManager::STATUS_TRUE)
			{
				_noServed = true;
				_noReverd_list.push_back(i.outside_number);
			}
		}
		if (_noServed) {
			response_json["error_"] = JSON::array();
			response_json["error"] = "not served";
			for (auto o : _noReverd_list)
				response_json["error_"].push_back(o);
			return ctx->send(response_json.dump());
		}

		std::thread	t([&] {
			auto pay=mess_client.get_message(phone_pair, request_json["key"].get<std::string>());
			response_json["content"] = JSON::array();
			for(auto &pa:pay){
				JSON	js;
				js["phone"] = pa.outside_number;
				js["message"] = pa.mess_key;
				response_json["content"].push_back(js);//pa.to_json()
			}
			});
		response_json["hash"] = request_json["hash"];
		t.join();
		return 	ctx->send(response_json.dump(4));
		});

	router.GET("/login", [&](const HttpContextPtr& ctx) {
		std::string token = ctx->request->GetParam("token");
		JSON resp;
		if (token.empty()) {
			resp["token"] = "null";
			return ctx->send(resp.dump(4));
		}
		if (Redis_me.exist_user(token) ==true)
		{
			std::string hash = Redis_me.Alloc_Hash_in_token(token);
			resp["hash"] = hash;
		}
		else {
			resp["hash"] = "0";
		}
		return ctx->send(resp.dump());
		});





	HttpServer server;
	server.service = &router;
	server.port = port;
	// uncomment to test multi-processes
	//server.setProcessNum(4);
	// uncomment to test multi-threads
	server.setThreadNum(4);
	server.start();

	while (getchar());
	return 0;
}
