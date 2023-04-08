#include	<hv/UdpClient.h>
#include    <mutex>
#include    "json.hpp"

#include    "headers.h"

using JSON = nlohmann::json;
using namespace hv;
struct Msginfo_u
{
    INT32   payload_len;    //���س���
    INT32   key_len;        //���Źؼ��ʳ���
    INT32   phone_len;      //�ֻ������б���
    char buf[1024] = {0};
};
struct Msginfo_d
{
    INT32   payload_len;    //���س���
    INT32   phone_len;      //�ֻ����볤��
    INT32   mess_len;       //���ų���
    char buf[1024] = { 0 };          //��Ч����
};
struct PayLoad
{
    std::string mess_key;//�ؼ���
    std::string inside_number;  //�ֻ�����
    std::string outside_number;
public:
    JSON& to_json()
    {
        JSON j;
        j["phone"] = this->outside_number;
        j["message"]=this->mess_key;
        return j;
    }
};

class MessageClient
{
public:
    MessageClient(int port_p=2009, std::string remote="127.0.0.1") {
        port = port_p;
        remote_ip = remote;
        int sockfd = cli.createsocket(port, remote_ip.c_str());
        cli.onMessage = [&](const SocketChannelPtr& channel, Buffer* buf) {
            std::lock_guard<std::mutex> guard(_lock);
            printf("< %.*s\n", (int)buf->size(), (char*)buf->data());
            Msginfo_d* re = (Msginfo_d*)buf->data();
            PayLoad pay;
            pay.inside_number = std::string(re->buf, re->phone_len);
            pay.mess_key = std::string(re->buf + re->phone_len, re->mess_len);
            recv_msg.push_back(pay);
        };
        cli.start();
    }
	~MessageClient();

    std::vector<PayLoad>  get_message(std::vector<redis_phone> phone_pair,std::string key) {
        std::lock_guard<std::mutex> guard(_uploaded_lock);
        int i = 0;
        for (auto par : phone_pair) {
            std::string hash_Str = par.inside_number + par.outside_number+key;
            if (uploaded.count(hash_str(hash_Str)))
                i++;
            else {
                uploaded.insert({ hash_str(hash_Str), hash_Str });
            }
        }
        if (i == phone_pair.size()) {
            return ret_message(phone_pair,key);
        }
        std::vector<PayLoad>  ret;
        /*UdpClient   cli;
        int sockfd = cli.createsocket(port, remote_ip.c_str());
        if (sockfd < 0) {
            return ret;
        }
        cli.start();*/
        Msginfo_u   msg_u = { 0 };
        key.append(1, '\0');
        msg_u.key_len = key.length();

        int offset = 0,phone_len=0;
        strcpy_s(msg_u.buf+offset,key.length(), key.c_str());
        offset += key.length();

        for (auto one : phone_pair) {
            std::string phone_str=one.inside_number;
            phone_str.append(1, '\0');

            strcpy_s(msg_u.buf + offset, phone_str.length(), phone_str.c_str());
            offset += phone_str.length();
            phone_len+= phone_str.length();
        }
        msg_u.phone_len = phone_len;
        msg_u.payload_len = msg_u.key_len + msg_u.phone_len + sizeof(INT32) * 3;
        
        cli.sendto((void*)&msg_u,msg_u.payload_len+sizeof(INT32)*3);    //udp����
        
        while (ret.size() == 0) {
            for (auto in1 : recv_msg) {
                for (auto in2 : phone_pair) {
                    if (in1.inside_number.find(in2.inside_number.c_str()) != std::string::npos && in1.mess_key.find(key.c_str()) != std::string::npos)
                    {
                        in1.outside_number = in2.outside_number;
                        ret.push_back(in1);
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
        return  ret;
    }

    //ͨ��
    std::vector<PayLoad> ret_message(std::vector<redis_phone> phone_pair, std::string key) {
        std::lock_guard<std::mutex> guard(_lock);
        std::vector<PayLoad>  ret;
        for (auto in1 : recv_msg) {
            for (auto in2 : phone_pair) {
                if (in1.inside_number.find(in2.inside_number.c_str()) != std::string::npos && in1.mess_key.find(key.c_str()) != std::string::npos)
                {
                    in1.outside_number = in2.outside_number;
                    ret.push_back(in1);
                }
            }
        }
        return  ret;
    }
private:
    UdpClient cli;
    std::mutex  _lock;
    int port;
    std::string remote_ip;
    std::vector<PayLoad>  recv_msg;


    std::hash<std::string>  hash_str;
    std::mutex  _uploaded_lock;
    std::map<size_t, std::string>    uploaded;  //�����汣���Ѿ��ϴ��� �����Ѿ�getmessage�ĺ���
    //ǰ����һ��hash,�Ǻ���string���ɵ�hash,����string���ڲ�������ⲿ����+key
};


MessageClient::~MessageClient()
{
}