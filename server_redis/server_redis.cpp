#include "hv/HttpServer.h"
#include    "redis_my.h"    
#include	"json.hpp"
#include	<thread>
#include <codecvt>
#include <mutex>
using namespace hv;
using JSON = nlohmann::json;


int main(int argc, char** argv) {
	HV_MEMCHECK;
#pragma region MyRegion
	int port = 0;
	if (argc > 1) {
		port = atoi(argv[1]);
	}
	if (port == 0) port = 8080;
	HttpService router;
#pragma endregion
#pragma region REDIS
	Redis* my_redis = new Redis();
	IsUser	check_user;

	if (!my_redis->connect("127.0.0.1", 6379))
	{
		printf("connect error!\n");
		return 0;
	}

	/*my_redis->getAll_noServed("user");

	auto c=my_redis->has_Served("1101002", "user3");*/
#pragma endregion
	// curl -v http://ip:port/user/123
	router.GET("/can_recv", [&](const HttpContextPtr& ctx) {
		JSON resp;
		resp["hash"] = ctx->param("hash");
		resp["no_served"] = JSON::array();
		if (!check_user.Is_User(resp["hash"])) {
			resp["invalid"] = false;
			return ctx->send(resp.dump());
		}
		auto user_info = check_user.get_USER_INFO(resp["hash"]);
		resp["invalid"] = true;
		auto all = my_redis->getAll_noServed(user_info.token);
		for (auto a : all) {
			resp["no_served"].push_back(a.real_phone);
		}
		return ctx->send(resp.dump());
		});

	router.GET("/recv", [&](const HttpContextPtr& ctx) {
		std::mutex	mutex_me;
		mutex_me.lock();
		std::string hash = ctx->param("hash");
		std::string phone = ctx->param("phone");
		JSON resp;
		resp["hash"] = hash;
		resp["phone"] = phone;
		resp["no_served"] = JSON::array();
		if (!check_user.Is_User(hash)) {
			resp["invalid"] = false;
			return ctx->send(resp.dump());
		}
		auto user_info = check_user.get_USER_INFO(hash);
		resp["invalid"] = true;
		for (int i = 0; i < 4; i++) {
			Sleep(1000);
		}
		std::wstring duanxin_content = L"中文something...";
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
		std::string utf8_str = cvt.to_bytes(duanxin_content);
		resp["content"] = utf8_str;
		mutex_me.unlock();
		return ctx->send(resp.dump(4));
		});

	router.GET("/login", [&](const HttpContextPtr& ctx) {
		std::string token = ctx->param("token");
		JSON resp;
		if (my_redis->exist_user(token))
		{
			std::string hash = check_user.Alloc_hash(token);
			resp["hash"] = hash;
		}
		else {
			resp["hash"] = NULL;
		}
		return ctx->send(resp.dump());
		});



	HttpServer server;
	server.service = &router;
	server.port = port;


	// uncomment to test multi-processes
	// server.setProcessNum(4);
	// uncomment to test multi-threads
	server.setThreadNum(8);

	server.start();

	while (getchar() != '\n');
	return 0;
}
