#include    <iostream>

//API--
//post 127.0.0.1:8080/recv_mess
/*request-body-json :
      {
          "hash": "xxxxxxxxx",
              "list" : [
                  "6285711430788", "6285733751273"
              ] ,
              "key" : "wathapp"
      }*/
//API作用--返回json中list手机号列表接收到的短信内容:

//如果list表中存在一项是第一次提交任务(向短信服务器提交接收短信任务) : "/recv_mess"必定返回一个已经收到短信的号码    调用流程1
//如果list表中全部项都不是第一次提交任务 : "recv_mess"返回所有从短信服务器中相关的短信                           调用流程2
struct message_info {
    std::string outside_number; //外部号码 
    std::string inside_number;  //内部号码
    std::string message;        //短信内容
}
struct redis_phone
{
    std::string outside_number; //外部号码 
    std::string inside_number;  //内部号码
};
//程序内维护了一个udp接收回调函数
void udp_recv_callback(char*buf) {
    //解析udp数据----
    //每条udp只允许一个号码和一条短信 即
    struct {
        std::string inside_number;
        std::string message;
    }this_struct;
    this_struct* im = (this_struct*)buf;
    //...
    AllMessage.push_back(*im);
}

//程序内维护了一个任务表 使用哈希表  任务表由提交手机号码+key组成
//hash(手机号+关键词)  & 手机号+关键词
std::map<size_t, std::string> task;

//流程1 程序类维护了一个从短信服务器接收到短信的列表
std::vector<message_info> AllMessage;   //这是所有接收到的短信 全局变量
//如何判断"相关的短信":提交的外部号码 + key 一一对应
std::vector<message_info>   get_message(std::vector<redis_phone> _list, std::string key) {
    int i = 0;
    for (auto _li : _list) {
        if (task.count(hash(_li + key)))
            i++;
        else
            task.push_back(hash(_li + key), _li + key);
    }
    if (i == _list.size())
        return ret_message(_list, key);//调用流程2
	std::vector<message_info> ret;
    while (ret.size() == 0) {
        for (auto i : AllMessage) {
            for (int i = 0; i < json["list"].size(); i++) {
                if (i.outside_number == json["list"][i] && i.message.find(json["key"]) != std::string::npos)
                    ret.push_back(i);
            }
        }
    }
	return  ret;
}
//流程2 
std::vector<message_info>   ret_message(std::vector<redis_phone> _list, std::string key) {
    std::vector<message_info> ret;
    while (ret.size() == 0) {
        for (auto i : AllMessage) {
            for (int i = 0; i < json["list"].size(); i++) {
                if (i.outside_number == json["list"][i] && i.message.find(json["key"]) != std::string::npos)
                    ret.push_back(i);
            }
        }
    }
    return  ret;
}