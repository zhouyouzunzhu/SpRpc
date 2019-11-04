#include "mstr.h"
#include <queue>
#include <cstring>

#ifdef WIN32
#include <ShlObj.h>
#endif

#define BUF_SIZE 4096
char buffer[BUF_SIZE] = { 0 };
char M_buf1[0xff] = { 0 };
char M_buf2[0xff] = { 0 };

String::String()
{
	m_pData = new char[1];
	m_pData[0] = '\0';
	m_sArgIndex = 0;
	m_sLen = 0;
}
String::String(const char* format, ...)
{
	va_list va;
	va_start(va, format);
	std::vsprintf(buffer, format, va);
	va_end(va);
	m_sArgIndex = 0;
	m_sLen = std::strlen(buffer);
	m_pData = new char[m_sLen + 1];
	strcpy(m_pData, buffer);
}
String::String(const String &str)
{
	m_sArgIndex = 0;
	m_sLen = str.m_sLen;
	m_pData = new char[m_sLen + 1];
	std::strcpy(m_pData, str.m_pData);
}
String::String(const std::string& str)
{
	m_sArgIndex = 0;
	m_sLen = str.size();
	m_pData = new char[m_sLen + 1];
	m_pData[m_sLen] = '\0';
	std::strcpy(m_pData, str.c_str());
}
String::String(const std::wstring& str)
{
	std::string curLocale = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = str.c_str();
	size_t _Dsize = 2 * str.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	wcstombs(_Dest, _Source, _Dsize);
	m_pData = _Dest;
	m_sLen = _Dsize - 1;
	setlocale(LC_ALL, curLocale.c_str());
}
String::~String(){ delete[] m_pData; }

String& String::arg(const String& str)
{
	sprintf(M_buf1, "{%u}", m_sArgIndex++);
	replace(this, this, M_buf1, str.m_pData, std::strlen(M_buf1), str.m_sLen);
	return *this;
}
String& String::arg(const char* str)
{
	sprintf(M_buf1, "{%u}", m_sArgIndex++);
	replace(this, this, M_buf1, str, std::strlen(M_buf1), std::strlen(str));
	return *this;
}
String& String::arg(const int& v, const int& dec)
{
	sprintf(M_buf1, "%%%c", (dec == 16 ? 'X' : (dec == 8 ? 'o' : 'd')));
	sprintf(M_buf2, M_buf1, v);
	sprintf(M_buf1, "{%u}", m_sArgIndex++);
	replace(this, this, M_buf1, M_buf2, std::strlen(M_buf1), std::strlen(M_buf2));
	return *this;
}
String& String::arg(const long& v, const int& dec)
{
	sprintf(M_buf1, "%%%c", (dec == 16 ? 'X' : (dec == 8 ? 'o' : 'd')));
	sprintf(M_buf2, M_buf1, v);
	sprintf(M_buf1, "{%u}", m_sArgIndex++);
	replace(this, this, M_buf1, M_buf2, std::strlen(M_buf1), std::strlen(M_buf2));
	return *this;
}
String& String::arg(const float& v, const int& res)
{
	sprintf(M_buf1, "%%.%df", res);
	sprintf(M_buf2, M_buf1, v);
	sprintf(M_buf1, "{%u}", m_sArgIndex++);
	replace(this, this, M_buf1, M_buf2, std::strlen(M_buf1), std::strlen(M_buf2));
	return *this;
}
String& String::arg(const double& v, const int& res)
{
	sprintf(M_buf1, "%%.%df", res);
	sprintf(M_buf2, M_buf1, v);
	sprintf(M_buf1, "{%u}", m_sArgIndex++);
	replace(this, this, M_buf1, M_buf2, std::strlen(M_buf1), std::strlen(M_buf2));
	return *this;
}

int String::toInt()const
{
	int num = 0;
	size_t i = 0;
	String str = *this;
	char sign = 'd';
	bool isFu = false;
	str.toLowerCase();

	while (i < str.m_sLen)
	{
		if (str.m_pData[i] == '0')
		{
			if (str.m_pData[i + 1] == 'x') { if (i - 1 >= 0 && str.m_pData[i - 1] == '-') isFu = true; sign = 'x'; i += 2; break; }
			if (str.m_pData[i + 1] > '0' && str.m_pData[i + 1] < '8'){ if (i - 1 >= 0 && str.m_pData[i - 1] == '-') isFu = true; sign = '0'; ++i; break; }
		}
		else if (str.m_pData[i] == '#') { if (i - 1 >= 0 && str.m_pData[i - 1] == '-') isFu = true; sign = 'x'; ++i; break; }
		else if (str.m_pData[i] >= '0' && str.m_pData[i] <= '9') { if (i - 1 >= 0 && str.m_pData[i - 1] == '-') isFu = true; break; }
		++i;
	}

	switch (sign)
	{
	case 'd':
		for (; i < m_sLen; ++i)
		{
			if (str.m_pData[i] >= '0' && str.m_pData[i] <= '9') num = num * 10 + (str.m_pData[i] - '0');
			else break;
		}
		break;
	case '0':
		for (; i < m_sLen; ++i)
		{
			if (str.m_pData[i] >= '0' && str.m_pData[i] <= '7') num = num * 8 + (str.m_pData[i] - '0');
			else break;
		}
		break;
	case 'x':
		for (; i < m_sLen; ++i)
		{
			if (str.m_pData[i] >= '0' && str.m_pData[i] <= '9') num = num * 16 + (str.m_pData[i] - '0');
			else if (str.m_pData[i] >= 'a' && str.m_pData[i] <= 'f') num = num * 16 + 10 + (str.m_pData[i] - 'a');
			else break;
		}
		break;
	}
	
	return num * (isFu ? -1 : 1);
}
float String::toFloat()const
{
	size_t index = 0;
	float value = 0.0f;
	for (size_t i = 0; i <= m_sLen; ++i)
	{
		if (m_pData[i] == '-')
		{
			if (i + 1 < m_sLen && (m_pData[i + 1] >= '0' && m_pData[i + 1] <= '9'))
				M_buf1[index++] = m_pData[i];
		}
		else if (((m_pData[i] >= '0' && m_pData[i] <= '9') || m_pData[i] == '.') && index + 1 < 0xf)
			M_buf1[index++] = m_pData[i];
		else if (index != 0)
		{
			M_buf1[index] = '\0';
			value = static_cast<float>(std::atof(M_buf1));
			break;
		}
	}
	return value;
}
std::string String::toString()const{ return m_pData; }
std::wstring String::toWString()const{
	setlocale(LC_ALL, "chs");
	size_t _Dsize = m_sLen + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest, m_pData, _Dsize);
	std::wstring ws = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, "C");
	return ws;
}
String String::toUtf8()const
{
#ifdef WIN32
	static char *buf = NULL;
	if (buf)
	{
		free(buf);
		buf = NULL;
	}
	wchar_t *unicode_buf;
	int nRetLen = MultiByteToWideChar(CP_ACP, 0, m_pData, -1, NULL, 0);
	unicode_buf = (wchar_t*)malloc((nRetLen + 1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, m_pData, -1, unicode_buf, nRetLen);
	nRetLen = WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, NULL, 0, NULL, NULL);
	buf = (char*)malloc(nRetLen + 1);
	WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, buf, nRetLen, NULL, NULL);
	free(unicode_buf);
	return buf;
#else
	return *this;
#endif
}

char& String::at(const size_t& index)
{
	if (m_sLen == 0)
	{
		if (m_pData != nullptr)
			delete[] m_pData;
		m_pData = new char[2];
		m_pData[0] = '\0';
		m_pData[1] = '\0';
		m_sLen = 1;
	}
	return m_pData[(index >= m_sLen ? m_sLen - 1 : index)];
}
String& String::append(const String& str)
{
	m_sLen = m_sLen + str.m_sLen;
	char* tp = m_pData;
	m_pData = new char[m_sLen + 1];
	std::strcpy(m_pData, tp);
	std::strcat(m_pData, str.m_pData);
	delete[] tp;
	return *this;
}
String& String::append(const char* str)
{
	m_sLen = m_sLen + std::strlen(str);
	char* tp = m_pData;
	m_pData = new char[m_sLen + 1];
	std::strcpy(m_pData, tp);
	std::strcat(m_pData, str);
	delete[] tp;
	return *this;
}
String& String::append(const char& ch)
{
	m_sLen = m_sLen + 1;
	char* tp = m_pData;
	m_pData = new char[m_sLen + 1];
	std::strcpy(m_pData, tp); 
	m_pData[m_sLen - 1] = ch;
	delete[] tp;
	return *this;
}
String& String::trim()
{
	size_t bgn = 0, end = m_sLen - 1, tlen = 0;
	for (; bgn < m_sLen; ++bgn)
	{
		if (m_pData[bgn] != ' ' &&
			m_pData[bgn] != '\t' &&
			m_pData[bgn] != '\n')
			break;
	}
	for (; end > bgn && end < m_sLen; --end)
	{
		if (m_pData[end] != ' ' &&
			m_pData[end] != '\t' &&
			m_pData[end] != '\n'){
			++end; break;
		}
	}
	tlen = end - bgn;
	if (tlen >= m_sLen) return *this;
	char* tData = new char[tlen + 1];
	std::memcpy(tData, m_pData + bgn, tlen);
	tData[tlen] = '\0';
	delete[] m_pData;
	m_pData = tData;
	m_sLen = tlen;
	return *this;
}
String& String::erase(const size_t& beg, const size_t& len)
{
	size_t tLen = m_sLen - len;
	if (tLen >= m_sLen || beg + len > m_sLen) return *this;
	char* tData = new char[tLen + 1];
	memcpy(tData, m_pData, beg);
	memcpy(tData + beg, m_pData + beg + len, tLen - beg);
	tData[tLen] = '\0';
	delete[] m_pData;
	m_pData = tData;
	m_sLen = tLen;
	return *this;
}
String& String::reverse()
{
	size_t bgn = 0, end = m_sLen - 1;
	if (end >= m_sLen) return *this;
	while (bgn < end)
	{
		m_pData[bgn] ^= m_pData[end];
		m_pData[end] ^= m_pData[bgn];
		m_pData[bgn++] ^= m_pData[end--];
	}
	return *this;
}
String& String::toUpperCase()
{
	for (size_t i = 0; i < m_sLen; ++i)
	{
		if (m_pData[i] >= 'a' && m_pData[i] <= 'z')
			m_pData[i] -= 32;
	}
	return *this;
}
String& String::toLowerCase()
{
	for (size_t i = 0; i < m_sLen; ++i)
	{
		if (m_pData[i] >= 'A' && m_pData[i] <= 'Z')
			m_pData[i] += 32;
	}
	return *this;
}

String String::operator+(const String& str)const
{
	String tStr;
	delete[] tStr.m_pData;
	tStr.m_sLen = m_sLen + str.m_sLen;
	tStr.m_pData = new char[tStr.m_sLen + 1];
	std::strcpy(tStr.m_pData, m_pData);
	std::strcat(tStr.m_pData, str.m_pData);
	return tStr;
}
String String::operator+(const char* str)const
{
	if (str == nullptr) return *this;
	String tStr;
	delete[] tStr.m_pData;
	tStr.m_sLen = m_sLen + std::strlen(str);
	tStr.m_pData = new char[tStr.m_sLen + 1];
	std::strcpy(tStr.m_pData, m_pData);
	std::strcat(tStr.m_pData, str);
	return tStr;
}
String String::operator+(const char& ch)const
{
	String tStr;
	delete[] tStr.m_pData;
	tStr.m_sLen = m_sLen + 1;
	tStr.m_pData = new char[tStr.m_sLen + 1];
	std::strcpy(tStr.m_pData, m_pData);
	tStr.m_pData[m_sLen] = ch;
	tStr.m_pData[tStr.m_sLen] = '\0';
	return tStr;
}
String& String::operator=(const String& str)
{
	delete[] m_pData;
	m_sLen = str.m_sLen;
	m_pData = new char[m_sLen + 1];
	strcpy(m_pData, str.m_pData);
	return *this;
}
String& String::operator=(const char* str)
{
	delete[] m_pData;
	if (str == nullptr)
	{
		m_sLen = 0;
		m_pData = new char[1];
		m_pData[0] = '\0';
		return *this;
	}
	m_sLen = strlen(str);
	m_pData = new char[m_sLen + 1];
	strcpy(m_pData, str);
	return *this;
}
String& String::operator=(const char& ch)
{
	delete[] m_pData;
	m_sLen = 1;
	m_pData = new char[2];
	m_pData[0] = ch;
	m_pData[1] = '\0';
	return *this;
}
String& String::operator+=(const String& str){ return append(str); }
String& String::operator+=(const char* str){ return append(str); }
String& String::operator+=(const char& ch){ return append(ch); }

bool String::operator==(const String& str)const{ return (std::strcmp(m_pData, str.m_pData) == 0); }
bool String::operator!=(const String& str)const{ return (std::strcmp(m_pData, str.m_pData) != 0); }
bool String::operator<(const String& str)const{ return (std::strcmp(m_pData, str.m_pData) < 0); }
bool String::operator>(const String& str)const{ return (std::strcmp(m_pData, str.m_pData) > 0); }
String::operator size_t()const{ return static_cast<size_t>(*((int*)m_pData) + m_sLen); }
char& String::operator[](const size_t& index){ return at(index); }

size_t String::find(const char* src, const size_t& begin)const
{
	if (begin > m_sLen) return -1;

	size_t bgn = begin;
	size_t count = std::strlen(src);
	if (m_sLen >= count)
	{
		while (m_pData[bgn] && bgn <= m_sLen - count)
		{
			size_t j = bgn, i = 0;
			while (src[i] && m_pData[j] && src[i] == m_pData[j]){ ++i; ++j; }
			if (src[i] == '\0') return bgn;
			++bgn;
		}
	}
	return -1;
}
size_t String::find(const String& src, const size_t& begin)const
{
	if (begin > m_sLen) return -1;

	size_t bgn = begin;
	size_t count = src.m_sLen;
	if (m_sLen >= count)
	{
		while (m_pData[bgn] && bgn <= m_sLen - count)
		{
			size_t j = bgn, i = 0;
			while (src.m_pData[i] && m_pData[j] && src.m_pData[i] == m_pData[j]){ ++i; ++j; }
			if (src.m_pData[i] == '\0') return bgn;
			++bgn;
		}
	}
	return -1;
}
size_t String::rfind(const char* src)const
{
	size_t bgn = m_sLen;
	size_t count = std::strlen(src);
	if (bgn >= count)
	{
		bgn = bgn - count;
		while (bgn >= 0)
		{
			size_t j = bgn, i = 0;
			while (src[i] && m_pData[j] && src[i] == m_pData[j]){ ++i; ++j; }
			if (src[i] == '\0') return bgn;
			if (bgn == 0) break;
			--bgn;
		}
	}
	return -1;
}
size_t String::rfind(const String& src)const
{
	size_t bgn = m_sLen;
	size_t count = src.m_sLen;
	if (bgn >= count)
	{
		bgn = bgn - count;
		while (bgn >= 0)
		{
			size_t j = bgn, i = 0;
			while (src.m_pData[i] && m_pData[j] && src.m_pData[i] == m_pData[j]){ ++i; ++j; }
			if (src.m_pData[i] == '\0') return bgn;
			if (bgn == 0) break;
			--bgn;
		}
	}
	return -1;
}

bool String::beginWith(const char* str) const{ return find(str) == 0; }
bool String::beginWith(const String& str) const{ return find(str) == 0; }
bool String::endWith(const char* str) const{ return m_sLen - find(str) == std::strlen(str); }
bool String::endWith(const String& str) const{ return m_sLen - find(str) == str.m_sLen; }

void String::replace(const String* in, String* out, const char* src, const char* tar, const size_t& srcLen, const size_t& tarLen)
{
	size_t index = 0;
	std::queue<size_t> signs;
	while (index <= in->m_sLen - srcLen)
	{
		index = in->find(src, index);
		if (index == -1) break;
		signs.push(index);
		++index;
	}
	size_t tLen = in->m_sLen + (tarLen - srcLen) * signs.size();
	char* tData = new char[tLen + 1];
	index = 0;
	size_t newIndex = 0;
	while (newIndex <= tLen)
	{
		if (!signs.empty() && index == signs.front())
		{
			for (size_t i = 0; i < tarLen; ++i)
				tData[newIndex + i] = tar[i];
			newIndex = newIndex + tarLen;
			index = index + srcLen;
			signs.pop();
		}
		else
			tData[newIndex++] = in->m_pData[index++];
	}
	tData[tLen] = '\0';
	delete[] out->m_pData;
	out->m_sLen = tLen;
	out->m_pData = tData;
}
String String::replace(const char* src, const char* tar)const
{
	String str;
	replace(this, &str, src, tar, std::strlen(src), std::strlen(tar));
	return str;
}
String String::replace(const String& src, const char* tar)const
{
	String str;
	replace(this, &str, src.m_pData, tar, src.m_sLen, std::strlen(tar));
	return str;
}
String String::replace(const char* src, const String& tar)const
{
	String str;
	replace(this, &str, src, tar.m_pData, std::strlen(src), tar.m_sLen);
	return str;
}
String String::replace(const String& src, const String& tar)const
{
	String str;
	replace(this, &str, src.m_pData, tar.m_pData, src.m_sLen, tar.m_sLen);
	return str;
}

String String::substr(const size_t& bgn, const size_t& end)const
{
	String str;
	delete[] str.m_pData;
	size_t tEnd = (end > m_sLen ? m_sLen : end);
	str.m_sLen = tEnd - bgn;
	if (str.m_sLen > m_sLen) return "";
	str.m_pData = new char[str.m_sLen + 1];
	std::memcpy(str.m_pData, m_pData + bgn, str.m_sLen);
	str.m_pData[str.m_sLen] = '\0';
	return str;
}
std::vector<String> String::split(const String& key)const
{
	std::vector<String> list;
	String str = *this, tstr;
	size_t pos = 0, tPos = 0;
	while ((pos = find(key, pos)) < m_sLen)
	{
		tstr = substr(tPos, pos);
		if (tstr != "")
			list.push_back(tstr);
		pos = pos + key.m_sLen;
		tPos = pos;
	}
	tstr = substr(tPos);
	if (tstr != "")
		list.push_back(tstr);
	return list;
}
const char* String::data()const{ return m_pData; }
const size_t& String::length()const{ return m_sLen; }
bool String::isEmpty()const { return (m_sLen == 0); }

String String::Join(const std::vector<String>& strs, const String& joinSign)
{
	String str;
	for (size_t i = 0; i < strs.size(); ++i)
	if (i + 1 < strs.size()) str += (strs[i] + joinSign);
	else str += strs[i];
	return str;
}

std::ostream& operator<<(std::ostream& os, const String& str)
{
	os << str.m_pData;
	return os;
}
std::istream& operator>>(std::istream& is, String& str)
{
	memset(buffer, 0, BUF_SIZE);
	is >> buffer;
	str = buffer;
	return is;
}

