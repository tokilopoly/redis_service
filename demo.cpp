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
//API����--����json��list�ֻ����б���յ��Ķ�������:

//���list���д���һ���ǵ�һ���ύ����(����ŷ������ύ���ն�������) : "/recv_mess"�ض�����һ���Ѿ��յ����ŵĺ���    ��������1
//���list����ȫ������ǵ�һ���ύ���� : "recv_mess"�������дӶ��ŷ���������صĶ���                           ��������2
struct message_info {
    std::string outside_number; //�ⲿ���� 
    std::string inside_number;  //�ڲ�����
    std::string message;        //��������
}
struct redis_phone
{
    std::string outside_number; //�ⲿ���� 
    std::string inside_number;  //�ڲ�����
};
//������ά����һ��udp���ջص�����
void udp_recv_callback(char*buf) {
    //����udp����----
    //ÿ��udpֻ����һ�������һ������ ��
    struct {
        std::string inside_number;
        std::string message;
    }this_struct;
    this_struct* im = (this_struct*)buf;
    //...
    AllMessage.push_back(*im);
}

//������ά����һ������� ʹ�ù�ϣ��  ��������ύ�ֻ�����+key���
//hash(�ֻ���+�ؼ���)  & �ֻ���+�ؼ���
std::map<size_t, std::string> task;

//����1 ������ά����һ���Ӷ��ŷ��������յ����ŵ��б�
std::vector<message_info> AllMessage;   //�������н��յ��Ķ��� ȫ�ֱ���
//����ж�"��صĶ���":�ύ���ⲿ���� + key һһ��Ӧ
std::vector<message_info>   get_message(std::vector<redis_phone> _list, std::string key) {
    int i = 0;
    for (auto _li : _list) {
        if (task.count(hash(_li + key)))
            i++;
        else
            task.push_back(hash(_li + key), _li + key);
    }
    if (i == _list.size())
        return ret_message(_list, key);//��������2
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
//����2 
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