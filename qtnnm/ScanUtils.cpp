
#include "stdafx.h"

#include "scanutils.h"

#include <windows.h>
#include <stdio.h>
#include <time.h>

//#include <Ice/Ice.h>

QString s2q(const std::string &s)
{
    return QString(QString::fromLocal8Bit(s.c_str()));
}

std::string q2s(const QString &s)
{
    return string((const char *)s.toLocal8Bit());
}



/*
std::string StringToUtf8(std::string astr)
{
	wchar_t wbuf[1024]={0};
	std::string srt;
        //std::wstring wst;
        //std::string stt=astr;
	int count=::MultiByteToWideChar(CP_ACP,0,astr.c_str(),(int)astr.size(),wbuf,0);
	if(count>=1024)
	{
		wchar_t *pwbuf=new wchar_t[count+1];
		memset(pwbuf,0,count*sizeof(wchar_t));

		MultiByteToWideChar(CP_ACP,0,astr.c_str(),(int)astr.size(),pwbuf,count+1);
		std::wstring wst=pwbuf;
		delete [] pwbuf;
		srt=IceUtil::wstringToString(wst);
	}else
	{
		count=MultiByteToWideChar(CP_ACP,0,astr.c_str(),(int)astr.size(),wbuf,1024);
		srt=IceUtil::wstringToString(wbuf);
	}
	return srt;
}

std::string Utf8ToString(std::string ustr)
{
	std::wstring wst=IceUtil::stringToWstring(ustr);
	std::string srt;

	char buf[1024]={0};
	int count=::WideCharToMultiByte(CP_ACP,0,wst.c_str(),(int)wst.size(),buf,0,NULL,NULL);
	if(count>=1024)
	{
		char *pbuf=new char[count+1];
		memset(pbuf,0,count);
		count=WideCharToMultiByte(CP_ACP,0,wst.c_str(),(int)wst.size(),buf,count+1,NULL,NULL);
		srt=pbuf;
		delete [] pbuf;
		return srt;
	}else
	{
		count=WideCharToMultiByte(CP_ACP,0,wst.c_str(),(int)wst.size(),buf,1024,NULL,NULL);
		srt=buf;
	}
	return srt;
}
*/


string&  lTrim(string   &ss)
{   
    //string::iterator   p= find_if(ss.begin(),ss.end(),not1(ptr_fun(isspace)));
    //ss.erase(ss.begin(),p);
    return  ss;   
}   

string&  lTrim(string   &ss, const string chr)
{   
	basic_string <char>::size_type p = ss.find_first_not_of(chr);
	if(p != npos)
	{
		ss = ss.substr(p);
	}
	return  ss;
}   

string&  rTrim(string   &ss) 
{   
    //string::reverse_iterator  p=find_if(ss.rbegin(),ss.rend(),not1(ptr_fun(isspace)));
    //ss.erase(p.base(),ss.end());
    return   ss;   
}   

string&  rTrim(string   &ss, const string chr)   
{   
	basic_string <char>::size_type p = ss.find_last_not_of(chr);
	if(p != npos)
	{
		ss = ss.substr(0,p+1);
	}
	return   ss;   
}   

string&   trim(string   &st)   
{   
    lTrim(rTrim(st)); 
    return   st;   
}   

string&   trim(string   &st, const string chr)   
{   
    lTrim(rTrim(st, chr)); 
	return st;
}   


//trim指示是否保留空串，默认为保留。
vector<string> tokenize(const string& src, string tok, bool trim, string null_subst)
{
	vector<string> v;
	if( src.empty() || tok.empty() )
	{
		return v;
//		throw "tokenize: empty string\0";
	}
	S_T pre_index = 0, index = 0, len = 0;
	while( (index = src.find(tok, pre_index)) != npos )
//	while( (index = src.find_first_of(tok, pre_index)) != npos )
	{
		if( (len = index-pre_index)!=0 )
		v.push_back(src.substr(pre_index, len));
		else if(trim==false)
		v.push_back(null_subst);
		pre_index = index+ tok.length();
//		pre_index = index+1;
	}
	string endstr = src.substr(pre_index);
	if( trim==false ) v.push_back( endstr.empty()? null_subst:endstr );
	else if( !endstr.empty() ) v.push_back(endstr);
	return v;
}

//int -> string
string  int2str(int  num)
{
	if (num == 0)
		return  "0";
    string  str = "" ;
    int  num_ = num > 0 ? num : -1 * num;
    while (num_)
    {
		str = (char)(num_ % 10 + 48) + str;
		num_ /= 10 ;
    }
    if (num < 0)
		str =  "-" + str;
    return  str;
}

//string -> int
int str2int(const string  str)
{
	int  i, len = str.size(), num = 0 ;
	i  =   0 ;   
	if (str[ 0 ]  ==   '-' )
	i  =   1 ;
	while (i  <  len)
	{
		num  =  num * 10   +  ( int )(str[i] - '0' );
		i ++ ;
	} 
	if (str[ 0 ]  == '-' )
	num  *=   - 1 ;
	return  num;
} 

//带基数的str->int
int str2int(const string  str_in, int ibase)
{
	string str = str_in;
        std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))toupper);

	int  i, len = str.size(), num = 0 ;
	i = 0 ;   
	if (str[0]  == '-')
	{
		i = 1;
	}
	while (i < len)
	{
		num  =  num  *   ibase;
		if(ibase == 16)
		{
			if((int)(str[i] - 'A') >= 0)
			{
				num += (int)(str[i] - 'A') + 10;
			}
			else
			{
				num += (int)(str[i] - '0');
			}
		}
		else
		{
			num+=  (int)(str[i] -  '0');
		}
		i++;
	} 
	if (str[0] == '-')
	{
		num *= -1;
	}
	return num;
} 

//替换全部子串
string& replaceAll(string& context, const string& from, const string& to)
{
    // replace all substring 'from' with substring 'to' in string context
    string::size_type curPos = 0;
    string::size_type foundPos = 0;
    while ((foundPos = context.find(from, curPos)) != string::npos) {
        context.replace(foundPos, from.size(), to);
        curPos = foundPos + to.size();
    }
    return context;
}

//产生一个GUID
string GenerateGuid()
{
        //USES_CONVERSION;

	GUID id;
	string ret;
        /*
	OLECHAR guid_str[64];

	if (CoCreateGuid(&id) != S_OK) 
	{
		return ret;	
	}
	StringFromGUID2(id, guid_str, sizeof(guid_str));

	ret = OLE2CA(guid_str);
        */

	return ret;
}

vector<string> stringSplit(const string& src, string tok)
{
	vector<string> dest;
	basic_string<char>::size_type pre_pos = 0, pos = 0, len = 0;
	while( (pos = src.find_first_of(tok, pre_pos)) != -1 )
	{
		if( (len = pos-pre_pos)!=0 )
			dest.push_back(src.substr(pre_pos, len));
		pre_pos = pos+1;
	}
	if(src.length() > pre_pos)
	{
		dest.push_back(src.substr(pre_pos));
	}
	return dest;
}

static std::map<std::string, int> tbl;

//获取mask的非0位数
int getMaskBitLen(const string& msk)
{
	if(tbl.empty())
	{
		tbl["255"] = 8;
		tbl["254"] = 7;
		tbl["252"] = 6;
		tbl["248"] = 5;
		tbl["240"] = 4;
		tbl["224"] = 3;
		tbl["192"] = 2;
		tbl["128"] = 1;
		tbl["0"] = 0;
	}

	int len = 0;
	vector<string> dest = stringSplit(msk, string("."));
	if(dest.size() != 4)
	{
                //throw std::runtime_error("Bad Mask");
	}
	return tbl[dest[0]] + tbl[dest[1]] + tbl[dest[2]] + tbl[dest[3]];
}

//
////获取mask的非0位数
//int getMaskBitLen(const string & msk)
//{
//	if(!msk.empty())
//	{
//		vector<string> dotCs = tokenize(msk, ".", true);
//		if(dotCs.size() != 4)
//		{
//			return -1;
//		}
//		int iLen = 0;
//		for(vector<string>::iterator i = dotCs.begin();
//			i != dotCs.end();
//			i++)
//		{
//			int myi = str2int(*i);
//			switch(myi)
//			{
//               case 255: 
//                   iLen += 8;
//				   break;
//               case 254: 
//                   iLen += 7;
//				   break;
//               case 252: 
//                   iLen += 6;
//				   break;
//               case 248: 
//                   iLen += 5;
//				   break;
//               case 240: 
//                   iLen += 4;
//				   break;
//               case 224: 
//                   iLen += 3;
//				   break;
//               case 192: 
//                   iLen += 2;
//				   break;
//               case 128: 
//                   iLen += 1;
//				   break;
//               case 0: 
//                   iLen += 0;
//				   break;
//			   default:
//				   return -1;
//			}
//		}
//		return iLen;
//	}
//	return -1;
//}

string getSubnetByIPMask(const string& ip, const string& mask)
{
	unsigned long masknum = ntohl(inet_addr(mask.c_str()));
	unsigned long subnet = ntohl(inet_addr(ip.c_str())) & masknum;
	int iLen = getMaskBitLen(mask);
	struct in_addr ipsubnet;
	ipsubnet.S_un.S_addr = htonl(subnet);
	return string(inet_ntoa(ipsubnet)) + "/" + int2str(iLen);
}

string getFacOid(const string& sysOid)
{
	size_t pos = 0;
	int count = -1;
	while(++count < 7 && (pos= sysOid.find(".", ++pos, 1)) != -1 );
	return sysOid.substr(0, pos);
}


//获取子网,以逗号分割的string
string getSubnetStr(const vector<string> & iplist, const vector<string>& msklist, const std::map<string, string>& subnets)
{
	string str_res = "";
	if(!iplist.empty() && !msklist.empty())
	{//
		vector<string>::const_iterator i,j;
		for(i = iplist.begin(), j = msklist.begin(); 
			i != iplist.end() && j != msklist.end();
			i++, j++)
		{
			if((*j).empty())
			{//掩码为"",使用子网匹配
				unsigned long ipnum = ntohl(inet_addr((*i).c_str()));
				unsigned long subnet = 0;
				for(std::map<string, string>::const_iterator k = subnets.begin();
					k != subnets.end();
					k++)
				{
					int sftlen = 32 - str2int(k->second);
					subnet = ntohl(inet_addr(k->first.c_str()));
                                        if((ipnum >> sftlen) == (subnet >> sftlen))
					{
						str_res += k->first + "/" + k->second + ",";
						break;
					}
				}
			}
			else
			{
				str_res += getSubnetByIPMask(*i,*j) + ",";
			}
		}
		if(!str_res.empty())
		{
			str_res = str_res.substr(0, str_res.length()-1);
		}
	}
	return str_res;
}

//
std::pair<string,string> getScaleByIPMask(const std::pair<string,string>& ipmask)
{
	unsigned long ipnum   = ntohl(inet_addr(ipmask.first.c_str()));
	unsigned long masknum = ntohl(inet_addr(ipmask.second.c_str()));
	unsigned long allipnum= ntohl(inet_addr("255.255.255.255"));
	unsigned long subnet = ipnum & masknum;
    unsigned long ip_min = subnet + 1;
    unsigned long ip_max = ((allipnum - masknum) | subnet) - 1;
	struct in_addr ipmin, ipmax;
	if(ipmask.second == "255.255.255.255")
	{
		return(make_pair(ipmask.first, ipmask.first));
	}
	else
	{
		ipmin.S_un.S_addr = htonl(ip_min);
		ipmax.S_un.S_addr = htonl(ip_max);
		string ipStrMin = inet_ntoa(ipmin);
		string ipStrMax = inet_ntoa(ipmax);
		return(make_pair(ipStrMin, ipStrMax));
	}
}


//Converting a WChar string to a Ansi string
std::string WChar2Ansi(const wchar_t *pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);

	if (nLen<= 0) return std::string("");

	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen -1] = 0;

	std::string strTemp(pszDst);
	delete [] pszDst;

	return strTemp;
}

string ws2s(wstring& inputws)
{ 
	return WChar2Ansi(inputws.c_str()); 
}

//Converting a Ansi string to WChar string
std::wstring Ansi2WChar(const char *pszSrc, int nLen)
{
	int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, 0, 0);
	if(nSize <= 0) return NULL;

	WCHAR *pwszDst = new WCHAR[nSize+1];
	if( NULL == pwszDst) return NULL;

	MultiByteToWideChar(CP_ACP, 0,(LPCSTR)pszSrc, nLen, pwszDst, nSize);
	pwszDst[nSize] = 0;

	if( pwszDst[0] == 0xFEFF) // skip Oxfeff
		for(int i = 0; i < nSize; i ++)
			pwszDst[i] = pwszDst[i+1];

	wstring wcharString(pwszDst);
	delete pwszDst;

	return wcharString;
}

std::wstring s2ws(const string& s)
{
	return Ansi2WChar(s.c_str(),s.size());
}

