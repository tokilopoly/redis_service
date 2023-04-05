#include "hv/requests.h"
#include "hv/axios.h"
using namespace hv;
#include "hv/hthread.h" // import hv_gettid
#include    <thread>
#include    <mutex>
#include <condition_variable>
std::mutex  _lock;
std::condition_variable cv;
static std::string Recv(HttpClient* cli, std::string hash, std::string phone) {
    HttpRequest req;
    req.method = HTTP_GET;
    req.url = "http://127.0.0.1:8080/recv_mess";
    req.SetParam("hash", hash);
    req.SetParam("phone", phone);

    req.headers["Connection"] = "keep-alive";
    req.timeout = 60;
    HttpResponse resp;
    int ret = cli->send(&req, &resp);
    if (ret != 0) {
        return "null";
    }
    else {
        //printf("%d %s\r\n", resp.status_code, resp.status_message());
        return resp.body.c_str();
    }
    return "null";
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
        return "null";
    }
    else {
        //printf("%d %s\r\n", resp.status_code, resp.status_message());
        return resp.body.c_str();
    }
    return "null";
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
        //printf("request failed!\n");
        return "null";
    }
    else {
        //printf("%d %s\r\n", resp.status_code, resp.status_message());
        return resp.body.c_str();
    }
    return "null";
}
void    Thread_func(std::string token) {
    HttpClient sync_client;
    std::string hash = login(&sync_client, token);

    auto s = hash.find_last_of("\"");
    std::string hash22 = hash.substr(hash.find(":") + 2, s - hash.find(":") - 2);
    std::string ccs = Get_Recv(&sync_client, hash22);
    std::string ccc = Recv(&sync_client, hash22, "1111");
    _lock.lock();
    std::cout << token << "  " << hash << std::endl;
    std::cout << token << "  " << hash22 << std::endl;
    std::cout << token << "  thread:can_recv " << ccs << std::endl << std::endl;
    std::cout << token << "  thread:recv_mess " << ccc << std::endl << std::endl;
    std::cout << token << "  ----------------------------------------" << std::endl;
    _lock.unlock();
}
int main(int argc, char* argv[]) {
    std::vector<std::shared_ptr<std::thread>> thread_poolt;
    for (int i = 0; i < 10; i++) {
        std::string user = "user" + std::to_string(i);
        std::shared_ptr<std::thread> t(new std::thread(Thread_func, user));
        thread_poolt.push_back(t);
    }
    for (auto& t : thread_poolt) {
        t->join();
    }
    printf("finished!\n");
    getchar();
    return 0;
}
