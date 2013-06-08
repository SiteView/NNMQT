
#include "stdafx.h"
#include "SnmpPing.h"

char* snmp_errors[] = {
	"NO ERROR",				/* 0 */
	"TOO BIG",				/* 1 */
	"NO SUCH NAME",			/* 2 */
	"BAD VALUE",			/* 3 */
	"READ ONLY",			/* 4 */
	"GENERIC ERROR",		/* 5 */
	"NO ACCESS",			/* 6 */
	"WRONG TYPE",			/* 7 */
	"WRONG LENGTH",			/* 8 */
	"WRONG ENCODING",		/* 9 */
	"WRONG VALUE",			/* 10 */
	"NO CREATION",			/* 11 */
	"INCONSISTENT VALUE",	/* 12 */
	"RESOURCE UNAVAILABLE",	/* 13 */
	"COMMIT FAILED",		/* 14 */
	"UNDO FAILED",			/* 15 */
	"AUTHORIZATION ERROR",	/* 16 */
	"NOT WRITABLE",			/* 17 */
	"INCONSISTENT NAME",	/* 18 */
};

int SnmpPing::parse_asn_length(u_char* buf, int buf_size, int* i)
{
	int len;
	if (*i >= buf_size)
	{
		return -1;
	}
	
	if (buf[*i] < 0x81) 
	{
		len = buf[*i];
		*i += 1;
	}
	else if (buf[*i] == 0x81)
	{
		*i += 1;
		if ((*i)+1 > buf_size) 
		{
			return -1;
		}
		len = buf[*i];
		*i += 1;
	}
	else if (buf[*i] == 0x82)
	{
		*i += 1;
		if ((*i)+2 > buf_size)
		{
			return -1;
		}
		len = (buf[*i] << 8) + buf[(*i)+1];
		*i += 2;
	}
	else if (buf[*i] == 0x83) 
	{
		*i += 1;
		if ((*i)+3 > buf_size)
		{
			return -1;
		}
		len = (buf[*i] << 16) + (buf[(*i)+1] << 8) + buf[(*i)+2];
		*i += 3;
	}
	else if (buf[*i] == 0x84)
	{
		*i += 1;
		if ((*i)+4 > buf_size)
		{
			return -1;
		}
		len = (buf[*i] << 24) + (buf[(*i)+1] << 16) + (buf[(*i)+2] << 8) + buf[(*i)+3];
		*i += 4;
	}
	else 
	{
		return -1;
	}

	if ((*i)+len > buf_size) 
	{
		return -1;
	}

	return len;
}

int SnmpPing::skip_asn_length(u_char* buf, int buf_size, int* i)
{
	int ret;
	
	if ((ret = parse_asn_length(buf, buf_size, i)) > 0)
		*i += ret;

	return ret;
}

int SnmpPing::parse_asn_integer(u_char* buf, int buf_size, int* i)
{
	int ret;

	if (*i >= buf_size)
	{
		return -1;
	}

	if (buf[*i] == 0x81)
	{
		*i += 1;
		if (*i >= buf_size)
		{
			return -1;
		}
	}

	if (buf[*i] == 0x01)
	{
		if ((*i)+2 > buf_size) 
		{
			return -1;
		}
		ret = (int)buf[(*i)+1];
		*i += 2;
	} 
	else if (buf[*i] == 0x02)
	{
		if ((*i)+3 > buf_size) 
		{
			return -1;
		}
		ret = ((int)buf[(*i)+1] << 8) +	
			   (int)buf[(*i)+2];
		*i += 3;
	}
	else if (buf[*i] == 0x04) 
	{
		if ((*i)+5 > buf_size) 
		{
			return -1;
		}
		ret = 	((int)buf[(*i)+1] << 24) +
				((int)buf[(*i)+2] << 16) +
				((int)buf[(*i)+3] << 8) +
				(int)buf[(*i)+4];
		*i += 5;
	}
	else 
	{
		//cout << "Unable to decode SNMP packet: unrecognized integer length" << endl;
		return -1;
	}

	return ret;
}

int SnmpPing::print_asn_string(u_char* buf, int buf_size, int* i)
{
	int ret;
	int string_end;

	if ((ret = parse_asn_length(buf, buf_size, i)) == -1)
		return -1;
	else
		string_end = *i + ret;

	for (;*i < string_end; *i += 1) 
	{
		if (buf[*i] < 0x20 || buf[*i] > 0x80)
			cout << " ";
		else
			cout <<(char)buf[*i];
	}

	return 0;
}

int SnmpPing::parse_snmp_header(u_char* buf, int buf_size, int* i)
{
	if (*i >= buf_size) 
	{
		return -1;
	}

	if (buf[(*i)++] != 0x30) 
	{
		return -1;
	}

	if (parse_asn_length(buf, buf_size, i) < 0)
		return -1;

	return 0;
}

int SnmpPing::parse_snmp_version(u_char* buf, int buf_size, int* i)
{
	int ret;

	if (*i >= buf_size) 
	{
		return -1;
	}

	if (buf[(*i)++] != 0x02)
	{
		return -1;
	}

	if ((ret = parse_asn_integer(buf, buf_size, i)) == -1)
		return -1;
	else if (ret != 0 && ret != 1) 
	{
		return -1;
	}
	return 0;
}

int SnmpPing::parse_snmp_community(u_char* buf, int buf_size, int* i)
{
	if (*i >= buf_size)
	{
		return -1;
	}
	if (buf[(*i)++] != 0x04) 
	{
		return -1;
	}
	if (print_asn_string(buf, buf_size, i) == -1)
		return -1;

	return 0;
}

int SnmpPing::parse_snmp_pdu(u_char* buf, int buf_size, int* i)
{
	if (*i >= buf_size) 
	{
		return -1;
	}

	if (buf[(*i)++] != 0xa2) {
		return -1;
	}
	
	if (parse_asn_length(buf, buf_size, i) < 0)
		return -1;

	return 0;
}

int SnmpPing::parse_snmp_requestid(u_char* buf, int buf_size, int* i)
{
	if (*i >= buf_size) 
	{
		return -1;
	}

	if (buf[(*i)++] != 0x02) 
	{
		return -1;
	}
	if (parse_asn_integer(buf, buf_size, i) < 0)
		return -1;

	return 0;
}

int SnmpPing::parse_snmp_errorcode(u_char* buf, int buf_size, int* i)
{
	int ret;

	if (*i >= buf_size) 
	{
		return -1;
	}

	if (buf[(*i)++] != 0x02)
	{
		return -1;
	}
	if ((ret = parse_asn_integer(buf, buf_size, i)) < 0)
		return -1;
	if (ret != 0) 
	{
		if (ret < 0 || ret > 18) 
		{
			return -1;
		}			
		return -1;
	}

	return 0;
}

int SnmpPing::parse_snmp_errorindex(u_char* buf, int buf_size, int* i)
{
	if (*i >= buf_size) 
	{
		return -1;
	}

	if (buf[(*i)++] != 0x02) {
		return -1;
	}
	if (parse_asn_integer(buf, buf_size, i) < 0)
		return -1;

	return 0;
}

int SnmpPing::parse_snmp_objheader(u_char* buf, int buf_size, int* i)
{
	if (*i >= buf_size) {
		return -1;
	}

	if (buf[(*i)++] != 0x30) {
		return -1;
	}
	if (parse_asn_length(buf, buf_size, i) < 0)
		return -1;

	return 0;
}

int SnmpPing::parse_snmp_objheader6(u_char* buf, int buf_size, int* i)
{
	if (*i >= buf_size) {
		return -1;
	}

	if (buf[(*i)++] != 0x06) {
		return -1;
	}
	if (skip_asn_length(buf, buf_size, i) < 0)
		return -1;

	return 0;
}

int SnmpPing::parse_snmp_value(u_char* buf, int buf_size, int* i)
{
	if (*i >= buf_size) {
		return -1;
	}

	if (buf[(*i)++] != 0x04) {
		return -1;
	}
	if (print_asn_string(buf, buf_size, i) < 0)
		return -1;

	return 0;
}


void SnmpPing::parse_snmp_response(u_char* buf, int buf_size)
{
	int i;

	i = 0;

	if (parse_snmp_header(buf, buf_size, &i) == -1) return;
	if (parse_snmp_version(buf, buf_size, &i) == -1) return;
	if (parse_snmp_community(buf, buf_size, &i) == -1) return;
	if (parse_snmp_pdu(buf, buf_size, &i) == -1) return;
	if (parse_snmp_requestid(buf, buf_size, &i) == -1) return;
	if (parse_snmp_errorcode(buf, buf_size, &i) == -1) return;
	if (parse_snmp_errorindex(buf, buf_size, &i) == -1) return;
		
	if (i+3 <= buf_size && buf[i] == 0x00 && buf[i+1] == 0x30 && buf[i+2] == 0x20)	// Bug in an HP JetDirect
		i += 3;
	
	if (parse_snmp_objheader(buf, buf_size, &i) == -1) return;
	if (parse_snmp_objheader(buf, buf_size, &i) == -1) return;		// yes, this should be called twice
	if (parse_snmp_objheader6(buf, buf_size, &i) == -1) return;
	if (parse_snmp_value(buf, buf_size, &i) == -1) return;

}

bool SnmpPing::ReceiveSnmp(int sock, long wait, const string& srcip, struct sockaddr_in* remote_addr)
{
	int remote_addr_len;
	char buf[1500];
	int ret;
	fd_set fds;

	struct timeval tv_wait;
	tv_wait.tv_sec = wait / 1000;
	tv_wait.tv_usec = wait % 1000 * 1000;

	DWORD 	dwStartTime = GetTickCount();

	DWORD 	dwEndTime = dwStartTime + (DWORD)wait;
	do {
		/* Put the socket into the fd set */
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
	
		if ((ret = select(sock+1, &fds, NULL, NULL, &tv_wait)) == -1) 
		{
			return false;
		}
		else if (ret > 0) 
		{
			memset(&buf, 0x0, sizeof(buf));
			remote_addr_len = sizeof(*remote_addr);
	
			ret = recvfrom(sock, (char*)&buf, sizeof(buf), 0, (struct sockaddr*)remote_addr, &remote_addr_len);
			if(ret >= 0 && srcip == string(inet_ntoa(remote_addr->sin_addr)))
			{
				return true;
			}
		}
		dwEndTime = GetTickCount();
	} while ( dwEndTime - dwStartTime < (DWORD)wait*1000);

	return false;
}

SnmpPing::SnmpPing()
{
}

SnmpPing::~SnmpPing(void)
{
}

int SnmpPing::Init(const int verNo, const string& ip, const string& community)
{
	int i;
	static int id;
	char object[] = "\x30\x0e\x30\x0c\x06\x08\x2b\x06\x01\x02\x01\x01\x01\x0\x05\x00";
	if (21 + community.size() + strlen(object) > 1500) 
	{
//		printf("SNMP packet length exceeded.\nCommunity: %s\nObject: %s\n", community, object);
		return -2;
	}

	if (--id > 0x7ffe) id = 0;

	memset(sendbuf, 0, sizeof(sendbuf));

	sendbuf[0] = 0x30;
	sendbuf[1] = 19 + (char)community.size() + sizeof(object)-1;
	
	if(verNo == 2)
	{
		// Version: 2
		sendbuf[2] = 0x02;
		sendbuf[3] = 0x01;
		sendbuf[4] = 0x01;
	}
	else
	{
		// Version: 1
		sendbuf[2] = 0x02;
		sendbuf[3] = 0x01;
		sendbuf[4] = 0x00;
	}
	// Community
	sendbuf[5] = 0x04;
	sendbuf[6] = (char)community.size();

	strcpy((sendbuf + 7), community.c_str());
	i = 7 + (int)community.size();
	
	// PDU type: GET
	sendbuf[i++] = 0xa0;
	sendbuf[i++] = 12 + sizeof(object)-1;
	
	// Request ID
	sendbuf[i++] = 0x02; 
	sendbuf[i++] = 0x04;
	sendbuf[i++] = (char)((id >> 24) & 0xff);
	sendbuf[i++] = (char)((id >> 16) & 0xff);
	sendbuf[i++] = (char)((id >> 8) & 0xff);
	sendbuf[i++] = (char)((id >> 0) & 0xff);
	
	// Error status: no error
	sendbuf[i++] = 0x02;
	sendbuf[i++] = 0x01;
	sendbuf[i++] = 0x00;
	
	// Error index
	sendbuf[i++] = 0x02;
	sendbuf[i++] = 0x01;
	sendbuf[i++] = 0x00;

	// Object ID
	memcpy((char*)&sendbuf[i], &object, sizeof(object)-1);
	i = i + sizeof(object)-1;

	return i;
}

bool SnmpPing::Ping(const int verNo, const string& ip, const string& community, const int retrys, const int timeout)
{
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	int sock;
	int ret;
	int sendbuf_size;

	// Æô¶¯ Winsocket
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
    {
        return false;
    }

	/* socket creation */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) 
	{
		WSACleanup();
		return false;
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(0);
	
	ret = bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (ret < 0) 
	{
		WSACleanup();
		return false;
	}
	
	/* remote address */
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(161);

	sendbuf_size = Init(verNo, ip, community);//build_snmp_req((char*)&sendbuf, sizeof(sendbuf), community[0]);
	if(sendbuf_size < 0)
	{
		WSACleanup();
		return false;
	}
	remote_addr.sin_addr.s_addr = htonl(ntohl(inet_addr(ip.c_str())));
	bool bOK = false;
	for(int i = 0; i < retrys; ++i)
	{
		//printf("Start to ping %s with snmpv%d\n", inet_ntoa(*(struct in_addr*)&remote_addr.sin_addr.s_addr), verNo);

		ret = sendto(sock, (char*)&sendbuf, sendbuf_size, 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
		if (ret < 0) 
		{
			//printf("Error in sendto: %s\n", strerror(errno));
			continue;
		}
		if(ReceiveSnmp(sock, timeout, ip, &remote_addr))
		{
			bOK = true;
			break;
		}
	}
	WSACleanup();
	return bOK;
}
