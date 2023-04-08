#pragma once
#include    <string>
#include <locale>
#include <codecvt>
struct redis_phone
{
    std::string	 outside_number;	//�ⲿ����
    std::string	inside_number;	//�ڲ�����
};

std::wstring utf8ToWstring(const std::string& str)
{
    std::wstring_convert< std::codecvt_utf8<wchar_t> > strCnv;
    return strCnv.from_bytes(str);
}
std::string wstringToUtf8(const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t> > strCnv;
    return strCnv.to_bytes(str);
}