#include <string>
#include <locale>
#include <codecvt>

// convert string to wstring
inline std::wstring to_wide_string(const std::string& input)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(input);
}
// convert wstring to string 
inline std::string to_byte_string(const std::wstring& input)
{
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(input);
}
std::string wstringToUtf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t> > strCnv;
	return strCnv.to_bytes(str);
}

std::wstring utf8ToWstring(const std::string& str)
{
	std::wstring_convert< std::codecvt_utf8<wchar_t> > strCnv;
	return strCnv.from_bytes(str);
}