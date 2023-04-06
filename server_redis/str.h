#include <string>
#include <map>

struct redis_phone
{
	std::string	 real_phone;	//�ⲿ����
	std::string	phone;	//�ڲ�����
};


class Check_User
{
public:
	struct USER_INFO
	{
		std::string	token;
		time_t 	t;
	};
	struct USER_INFO_hash_time
	{
		std::string hash;
		USER_INFO	user_info;
	};
public:
	Check_User();
	~Check_User();
	USER_INFO_hash_time	Alloc_hash(std::string token) {
		_lock.lock();
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
		_lock.unlock();
		USER_INFO_hash_time	ret;
		ret.hash = c + b;
		ret.user_info.t = user.t;
		ret.user_info.token = token;
		return ret; //����hash
	}

	bool	exist_user_inhash(std::string hash) {
		return Now_user.count(hash);
	}
	bool	exist_user_intoken(std::string token) {
		for (const auto in : Now_user)
		{
			if (in.second.token == token)
				return	true;
		}
		return	false;
	}

	USER_INFO& get_user_info(std::string hash) {
		return Now_user.at(hash);
	}
	
	void	init_program_now_user(USER_INFO_hash_time user_info_hash_time) {//�����ڳ�ʼ����ʱ���ڰ�redis�����е��û��õ�
		_lock.lock();
		USER_INFO	user{ user_info_hash_time.user_info.token,user_info_hash_time.user_info.t };
		Now_user.insert(std::pair<std::string,USER_INFO>(user_info_hash_time.hash,user));
		_lock.unlock();
	}
private:
	std::map<std::string , USER_INFO>	Now_user;  //����һ��hash ��һ��token token�ǵ�¼��
	std::hash<std::string> hash_int;			//ͨ��token��ȡһ��hash
	std::hash<std::time_t>	hash_timet;			//ͨ��ϵͳʱ���ȡһ��hash
	std::mutex	_lock;	//��now_user������ʱ�� һ������
};

Check_User::Check_User()
{
}

Check_User::~Check_User()
{
}