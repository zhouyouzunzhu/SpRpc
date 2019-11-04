#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <iostream>
#include <vector>
#include <cstdarg>

class String
{
public:
	String();
	String(const String& str);
	String(const std::string& str);
	String(const std::wstring& str);
	String(const char* format, ...);
	virtual ~String();

	String& arg(const String& str);
	String& arg(const char* str);
	String& arg(const int& v, const int& dec = 10);
	String& arg(const long& v, const int& dec = 10);
	String& arg(const float& v, const int& res = 2);
	String& arg(const double& v, const int& res = 2);

	int toInt()const;
	float toFloat()const;
	std::string toString()const;
	std::wstring toWString()const;
	String toUtf8()const;

	char& at(const size_t& index);
	String& append(const String& str);
	String& append(const char* str);
	String& append(const char& ch);
	String& trim();
	String& erase(const size_t& beg, const size_t& len = 1);
	String& reverse();
	String& toUpperCase();
	String& toLowerCase();

	String operator+(const String& str)const;
	String operator+(const char* str)const;
	String operator+(const char& ch)const;
	String& operator=(const String& str);
	String& operator=(const char* str);
	String& operator=(const char& ch);
	String& operator+=(const String& str);
	String& operator+=(const char* str);
	String& operator+=(const char& ch);
	bool operator==(const String& str)const;
	bool operator!=(const String& str)const;
	bool operator<(const String& str)const;
	bool operator>(const String& str)const;
	explicit operator size_t()const;
	char& operator[](const size_t& index);

	size_t find(const char* src, const size_t& begin = 0)const;
	size_t find(const String& src, const size_t& begin = 0)const;
	size_t rfind(const char* src)const;
	size_t rfind(const String& src)const;

	bool beginWith(const char* str)const;
	bool beginWith(const String& str)const;
	bool endWith(const char* str)const;
	bool endWith(const String& str)const;

	String replace(const char* src, const char* tar)const;
	String replace(const String& src, const char* tar)const;
	String replace(const char* src, const String& tar)const;
	String replace(const String& src, const String& tar)const;

	String substr(const size_t& bgn, const size_t& end = -1)const;
	std::vector<String> split(const String& key)const;
	const char* data()const;
	const size_t& length()const;
	bool isEmpty()const;

	friend std::ostream& operator<<(std::ostream&, const String&);
	friend std::istream& operator>>(std::istream&, String&);
	friend struct std::hash<String>;
	static String Join(const std::vector<String>& strs, const String& joinSign = ", ");
private:
	static void replace(const String* in, String* out, const char* src, const char* tar, const size_t& srcLen, const size_t& tarLen);

private:
	size_t m_sArgIndex;
	size_t m_sLen;
	char *m_pData;
};

std::ostream& operator<<(std::ostream& os, const String& str);
std::istream& operator>>(std::istream& is, String& str);

namespace std
{
	template<> struct hash<String>
	{
		size_t operator()(const String& str)const{
			return std::hash<std::string>{}(str.m_pData);
		}
	};
}


