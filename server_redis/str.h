#include <string>
#include <map>

struct redis_phone
{
	std::string	 real_phone;	//外部号码
	std::string	phone;	//内部号码
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
		return	c+b; //返回hash
	}
	bool	Is_User(std::string hash) {
		return Now_user.count(hash);
	}
	USER_INFO& get_USER_INFO(std::string hash) {
		return Now_user.at(hash);
	}
private:
	std::map<std::string , USER_INFO>	Now_user;  //分配一个hash 和一个token token是登录用
	std::hash<std::string> hash_int;			//通过token获取一个hash
	std::hash<std::time_t>	hash_timet;			//通过系统时间获取一个hash
};

IsUser::IsUser()
{
}

IsUser::~IsUser()
{
}