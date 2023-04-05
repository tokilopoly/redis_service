#include <iostream>
#include <string>
#include <sw/redis++/redis++.h>
#include <sw/redis++/connection.h>
#include "redis_my.h"
#include <locale>
#include <codecvt>
#include <hv/HttpServer.h>
#include "json.hpp"

using namespace sw::redis;
using namespace hv;
using JSON = nlohmann::json;


int main(int argc, char** argv) {
	HV_MEMCHECK;
	int port = 8080;
	if (argc > 1) {
		port = atoi(argv[1]);
	}
	HttpService router;
	RedisManager	Redis_me("127.0.0.1");
	Redis_me.Connect();

	Redis_me._clear_all();
	Redis_me._add_user_and_phone();
	Redis_me.init_check_user();	//从数据库把所有users用户信息保存到程序里

#pragma region REDIS
	router.GET("/can_recv", [&](const HttpContextPtr& ctx) {
		JSON resp;
		std::string hash = ctx->request->GetParam("hash");
		if (!hash.empty()) {
			resp["hash"] = hash;
			resp["no_served"] = JSON::array();
			if (Redis_me.check_user_exist_inhash(resp["hash"]) != RedisManager::STATUS_TRUE) {
				resp["invalid"] = false;
				return ctx->send(resp.dump());
			}

			std::string user_token = Redis_me.hash_get_token(hash);
			resp["invalid"] = true;
			auto all = Redis_me.get_noServed_item(user_token,10);
			for (auto a : all) {
				resp["no_served"].push_back(a.real_phone);
			}
		}
		return ctx->send(resp.dump());
		});

	router.GET("/recv_mess", [&](const HttpContextPtr& ctx) {
		std::string hash = ctx->request->GetParam("hash");
		std::string phone = ctx->request->GetParam("phone");
		JSON resp;
		if (hash.empty() || phone.empty()) {
			resp["hash"] = "null";
			resp["phone"] = "null";
			return ctx->send(resp.dump(4));
		}
		resp["hash"] = hash;
		resp["phone"] = phone;
		if (Redis_me.check_user_exist_inhash(hash) != RedisManager::STATUS_TRUE || Redis_me.exist_real_phone(phone) != RedisManager::STATUS_TRUE) {
			resp["invalid"] = false;
			return ctx->send(resp.dump());
		}
		auto user_info = Redis_me.get_user_info(hash);
		resp["invalid"] = true;
		for (int i = 0; i < 20; i++) {
			Sleep(1000);
		}
		std::wstring duanxin_content = L"中文something...";
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
		std::string utf8_str = cvt.to_bytes(duanxin_content);
		resp["content"] = utf8_str;
		return ctx->send(resp.dump(4));
		});

	router.GET("/login", [&](const HttpContextPtr& ctx) {
		std::string token = ctx->request->GetParam("token");
		JSON resp;
		if (token.empty()) {
			resp["token"] = "null";
			return ctx->send(resp.dump(4));
		}
		if (Redis_me.exist_user(token) == RedisManager::STATUS_TRUE)
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
	server.setThreadNum(2);

	server.start();

	while (getchar() != '\n');
	return 0;
}
