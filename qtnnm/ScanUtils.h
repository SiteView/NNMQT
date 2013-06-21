
#pragma once

#ifndef SCANUTILS_H
#define SCANUTILS_H

#include <string>
#include <iostream> 
#include <fstream> 
#include <strstream> 
#include <vector>
#include <list>
#include <map>
#include <algorithm>   
#include <functional> 
#include <QString>

using namespace std;

typedef basic_string<char>::size_type S_T;
static const S_T npos = -1;

//std::string StringToUtf8(std::string astr);

//std::string Utf8ToString(std::string ustr);
QString s2q(const std::string &s);
std::string q2s(const QString &s);


string&  lTrim(string   &ss);

string&  lTrim(string   &ss, const string chr);

string&  rTrim(string   &ss);

string&  rTrim(string   &ss, const string chr);

string&   trim(string   &st);

string&   trim(string   &st, const string chr);

//trim指示是否保留空串，默认为保留。
vector<string> tokenize(const string& src, string tok, bool trim=false, string null_subst="");

//int -> string
string  int2str(int  num);

//string -> int
int str2int(const string  str);
//带基数的str->int
int str2int(const string str_in, int ibase);

//替换全部子串
string& replaceAll(string& context, const string& from, const string& to);


//产生一个GUID
string GenerateGuid();

string getSubnetByIPMask(const string& ip, const string& mask);

//获取子网,以逗号分割的string
string getSubnetStr(const vector<string> & iplist, const vector<string>& msklist, const std::map<string, string>& subnets);

string getFacOid(const string& str);

vector<string> stringSplit(const string& src, string tok);

//获取mask的非0位数
int getMaskBitLen(const string & msk);

//Converting a WChar string to a Ansi string
//std::string WChar2Ansi(LPCWSTR pwszSrc);
std::string WChar2Ansi(const wchar_t* pwszSrc);
string ws2s(wstring& inputws);

//Converting a Ansi string to WChar string
//std::wstring Ansi2WChar(LPCSTR pszSrc, int nLen);
std::wstring Ansi2WChar(const char *pszSrc, int nLen);
std::wstring s2ws(const string& s);

std::pair<string,string> getScaleByIPMask(const std::pair<string,string>& ipmask);


#endif

