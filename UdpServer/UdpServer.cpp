/*
 * UdpServer_test.cpp
 *
 * @build   make evpp
 * @server  bin/UdpServer_test 1234
 * @client  bin/UdpClient_test 1234
 *
 */

#include <iostream>
#include    <hv/UdpServer.h>
struct Msginfo_u
{
    INT32   payload_len;    //负载长度
    INT32   key_len;        //短信关键词长度
    INT32   phone_len;      //手机号码列表长度
    char buf[1024] = { 0 };
};
struct Msginfo_d
{
    INT32   payload_len;    //负载长度
    INT32   phone_len;      //手机号码长度
    INT32   mess_len;       //短信长度
    char    buf[1024] = {0};          //有效负载
};
using namespace hv;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s port\n", argv[0]);
        return -10;
    }
    int port = atoi(argv[1]);

    UdpServer srv;
    int bindfd = srv.createsocket(port);
    if (bindfd < 0) {
        return -20;
    }
    printf("server bind on port %d, bindfd=%d ...\n", port, bindfd);
    srv.onMessage = [](const SocketChannelPtr& channel, Buffer* buf) {
        Msginfo_u* msg_u = (Msginfo_u*)buf->data();
        printf("length-%d \n", msg_u->payload_len);
        printf("key_len-%d \n", msg_u->key_len);
        for (int i = 0; i < msg_u->phone_len+msg_u->key_len; i++) {
            printf("%c", msg_u->buf[i]);
        }

        
        
        
        //先把负载提出来
        std::string pay_str (msg_u->buf,msg_u->key_len+msg_u->phone_len);
        auto end_ = pay_str.find('\0');
        auto start_ = 0;
        std::string key= pay_str.substr(start_, end_ - start_);
        std::vector<std::string> phone_list;
        while (end_ != std::string::npos) {
            start_ = end_;
            end_ = pay_str.find('\0', end_+1);
            if (end_ == std::string::npos)
                break;
            std::string s = pay_str.substr(start_+1, end_ - start_);
            phone_list.push_back(s);
        }

        for (auto p : phone_list) {
            int offset = 0;
            Msginfo_d   msg_d = { 0 };
            msg_d.phone_len = 0x10;
            strcpy_s(msg_d.buf+offset, 0x10, p.c_str());
            offset += 0x10;
            char  send[] = "Hello Wath somethings...";
            int buf_len = strlen(send);
            memcpy_s(msg_d.buf + offset, 1024 - offset, send, buf_len);
            offset = offset + buf_len;
            msg_d.mess_len = buf_len;
            msg_d.payload_len = offset + sizeof(INT32) * 3;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            // echo
            channel->write((void*)&msg_d, msg_d.payload_len);
        }
    };
    srv.start();

    while (getchar());

    return 0;
}