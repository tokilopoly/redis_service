#include <string>
#include <map>

struct redis_phone
{
	std::string	 real_phone;	//外部号码
	std::string	phone;	//内部号码
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
				// 找到了要删除的元素
				auto to_remove = it; // 保存元素
				++it; // 先移动迭代器，以防止失效
				Now_user.erase(to_remove); // 删除元素
			}
			else {
				++it; // 移动迭代器
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
		return ret; //返回hash
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
	
	void	init_program_now_user(USER_INFO_hash_time user_info_hash_time) {//程序在初始化的时候在把redis中所有的用户拿到
		_lock.lock();
		USER_INFO	user{ user_info_hash_time.user_info.token,user_info_hash_time.user_info.t };
		Now_user.insert(std::pair<std::string,USER_INFO>(user_info_hash_time.hash,user));
		_lock.unlock();
	}
private:
	std::map<std::string , USER_INFO>	Now_user;  //分配一个hash 和一个token token是登录用
	std::hash<std::string> hash_int;			//通过token获取一个hash
	std::hash<std::time_t>	hash_timet;			//通过系统时间获取一个hash
	std::mutex	_lock;	//对now_user操作的时候 一定上锁
};

Check_User::Check_User()
{
}

Check_User::~Check_User()
{
}