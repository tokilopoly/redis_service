#include "hv/requests.h"
#include "hv/axios.h"
using namespace hv;
#include "hv/hthread.h" // import hv_gettid


static std::string Recv(HttpClient* cli, std::string hash, std::string phone) {
    HttpRequest req;
    req.method = HTTP_GET;
    req.url = "http://127.0.0.1:8080/recv";
    req.SetParam("hash", hash);
    req.SetParam("phone", phone);

    req.headers["Connection"] = "keep-alive";
    req.timeout = 60;
    HttpResponse resp;
    int ret = cli->send(&req, &resp);
    if (ret != 0) {
        return "";
    }
    else {
        printf("%d %s\r\n", resp.status_code, resp.status_message());
        return resp.body.c_str();
    }
    return "";
}
static std::string Get_Recv(HttpClient* cli, std::string hash) {
    HttpRequest req;
    req.method = HTTP_GET;
    req.url = "http://127.0.0.1:8080/can_recv";
    req.SetParam("hash", hash);
    req.headers["Connection"] = "keep-alive";
    req.timeout = 60;
    HttpResponse resp;
    int ret = cli->send(&req, &resp);
    if (ret != 0) {
        return "";
    }
    else {
        printf("%d %s\r\n", resp.status_code, resp.status_message());
        return resp.body.c_str();
    }
    return "";
}
static std::string login(HttpClient* cli, std::string token) {
    HttpRequest req;
    req.method = HTTP_GET;
    req.url = "http://127.0.0.1:8080/login";
    req.SetParam("token", token);
    req.headers["Connection"] = "keep-alive";
    req.timeout = 60;
    HttpResponse resp;
    int ret = cli->send(&req, &resp);
    if (ret != 0) {
        printf("request failed!\n");
        return "";
    }
    else {
        printf("%d %s\r\n", resp.status_code, resp.status_message());
        return resp.body.c_str();
    }
    return "";
}
int main(int argc, char* argv[]) {
    std::string token = argv[1];
    int req_cnt = 2;
    HttpClient sync_client;
    HttpClient async_client;
    int resp_cnt = 0;
    std::string hash=login(&sync_client, token);
    std::cout << hash << std::endl;

    auto s=    hash.find_last_of("\"");
    std::string hash22 = hash.substr(hash.find(":")+2, s - hash.find(":")-2);
    std::cout << hash22 << std::endl;

    getchar();
    for (int i = 0; i < req_cnt; ++i) {
        std::string ccs=Get_Recv(&sync_client, hash22);
        std::cout << ccs << std::endl << std::endl;
        std::string ccc = Recv(&sync_client, hash22, "1111");
        std::cout << ccc << std::endl << std::endl;
    }

    while (resp_cnt < req_cnt) hv_delay(100);
    printf("finished!\n");
    getchar();
    return 0;
}
