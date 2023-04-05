#include <string>
#include <map>

struct redis_phone
{
	std::string	 real_phone;	//�ⲿ����
	std::string	phone;	//�ڲ�����
};


class IsUser
{
	struct USER_INFO
	{
		std::string	token;
		time_t 	t;
	};
public:
	IsUser();
	~IsUser();
	std::string	Alloc_hash(std::string token) {
		auto it = Now_user.begin();
		while (it != Now_user.end()) {
			if (it->second.token == token) {
				// �ҵ���Ҫɾ����Ԫ��
				auto to_remove = it; // ����Ԫ��
				++it; // ���ƶ����������Է�ֹʧЧ
				Now_user.erase(to_remove); // ɾ��Ԫ��
			}
			else {
				++it; // �ƶ�������
			}
		}
		USER_INFO	user;
		user.t = std::time(nullptr);
		user.token = token;
		auto c= std::to_string(hash_int(token));
		auto b= std::to_string(hash_timet(user.t));
		
		Now_user.insert(std::pair<std::string, USER_INFO>(c+b, user));
		return	c+b; //����hash
	}
	bool	Is_User(std::string hash) {
		return Now_user.count(hash);
	}
	USER_INFO& get_USER_INFO(std::string hash) {
		return Now_user.at(hash);
	}
private:
	std::map<std::string , USER_INFO>	Now_user;  //����һ��hash ��һ��token token�ǵ�¼��
	std::hash<std::string> hash_int;			//ͨ��token��ȡһ��hash
	std::hash<std::time_t>	hash_timet;			//ͨ��ϵͳʱ���ȡһ��hash
};

IsUser::IsUser()
{
}

IsUser::~IsUser()
{
}