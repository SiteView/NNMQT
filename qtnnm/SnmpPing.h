#ifndef SNMPPING_H
#define SNMPPING_H
#pragma once

#include <string>
#include <iostream>

#include <sys/types.h>
#include <time.h>
//#include <Winsock2.h>
#include <ws2tcpip.h>
//#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "snmp_pp.lib")
#pragma comment(lib, "libdes.lib")

using namespace std;

extern char* snmp_errors[];


class SnmpPing
{
public:
	SnmpPing();
	~SnmpPing();

	bool Ping(const int verNo, const string& ip, const string& community, const int retrys, const int timeout);

private:
	int Init(const int verNo, const string& ip, const string& community);
	int parse_asn_length(u_char* buf, int buf_size, int* i);
	int skip_asn_length(u_char* buf, int buf_size, int* i);
	int parse_asn_integer(u_char* buf, int buf_size, int* i);
	int print_asn_string(u_char* buf, int buf_size, int* i);
	int parse_snmp_header(u_char* buf, int buf_size, int* i);
	int parse_snmp_version(u_char* buf, int buf_size, int* i);
	int parse_snmp_community(u_char* buf, int buf_size, int* i);
	int parse_snmp_pdu(u_char* buf, int buf_size, int* i);
	int parse_snmp_requestid(u_char* buf, int buf_size, int* i);
	int parse_snmp_errorcode(u_char* buf, int buf_size, int* i);
	int parse_snmp_errorindex(u_char* buf, int buf_size, int* i);
	int parse_snmp_objheader(u_char* buf, int buf_size, int* i);
	int parse_snmp_objheader6(u_char* buf, int buf_size, int* i);
	int parse_snmp_value(u_char* buf, int buf_size, int* i);
	void parse_snmp_response(u_char* buf, int buf_size);
	bool ReceiveSnmp(int sock, long wait,const string& srcip, struct sockaddr_in* remote_addr);

private:
	char sendbuf[1500];

};
#endif
