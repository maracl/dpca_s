/*
 * socket_common.cpp
 *
 *  Created on: 2013-5-21
 *      Author: ltmit
 */
//#include "stdafx.h"
#include "socket_common.h"
#include <stdlib.h>
#include <assert.h>

#ifndef min
#define min(x,y) ((x) < (y) ? x : y)
#endif

#ifdef __linux
#include <sys/ioctl.h>
#include <poll.h>

#define TRANS_CASE(name)  case name: return WSA##name; break
int sockerr_get() {
	int rawe=errno;
	switch(rawe)
	{
	TRANS_CASE(EWOULDBLOCK);
	TRANS_CASE(EINPROGRESS);
	TRANS_CASE(EALREADY);
	TRANS_CASE(ENOTSOCK);
	TRANS_CASE(EDESTADDRREQ);
	TRANS_CASE(EMSGSIZE);
	TRANS_CASE(EPROTOTYPE);
	TRANS_CASE(ENOPROTOOPT);
	TRANS_CASE(EPROTONOSUPPORT);
	TRANS_CASE(ESOCKTNOSUPPORT);
	TRANS_CASE(EOPNOTSUPP);
	TRANS_CASE(EPFNOSUPPORT);
	TRANS_CASE(EAFNOSUPPORT);
	TRANS_CASE(EADDRINUSE);
	TRANS_CASE(EADDRNOTAVAIL);
	TRANS_CASE(ENETDOWN);
	TRANS_CASE(ENETUNREACH);
	TRANS_CASE(ENETRESET);
	TRANS_CASE(ECONNABORTED);
	TRANS_CASE(ECONNRESET);
	TRANS_CASE(ENOBUFS);
	TRANS_CASE(EISCONN);
	TRANS_CASE(ENOTCONN);
	TRANS_CASE(ESHUTDOWN);
	TRANS_CASE(ETOOMANYREFS);
	TRANS_CASE(ETIMEDOUT);
	TRANS_CASE(ECONNREFUSED);
	TRANS_CASE(ELOOP);
	TRANS_CASE(ENAMETOOLONG);
	TRANS_CASE(EHOSTDOWN);
	TRANS_CASE(EHOSTUNREACH);
	TRANS_CASE(ENOTEMPTY);
	//TRANS_CASE(EPROCLIM);
	TRANS_CASE(EUSERS);
	TRANS_CASE(EDQUOT);
	TRANS_CASE(ESTALE);
	TRANS_CASE(EREMOTE);
	default:
		return rawe;
	}
}
void sockerr_set(int e) {  errno=e; }

int sk_sys_init() {
	return 0;
}

int sk_sys_deinit() {
	return 0;
}
#endif
#ifdef _WIN32
int sockerr_get() {return WSAGetLastError();}
void sockerr_set(int e) {  WSASetLastError(e); }
#define  snprintf _snprintf
#pragma warning(disable:4996)

int sk_sys_init() {
	WSAData wd;
	return WSAStartup(MAKEWORD(2,2),&wd);
}

int sk_sys_deinit() {
	return WSACleanup();
}
#endif




#ifdef _MSC_VER
#pragma comment(lib,"ws2_32")
#endif
///-------------------------------------------------------------
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

int sk_unix_addr(sockaddr_ex& saex, const char* uri)
{
	saex.sa_family = AF_UNIX;
	saex.sa_data[108] = 0;
	strncpy(saex.sa_data, uri, 108);
	return 0;
}

int sk_addr_from_uri(sockaddr_ex& bindaddr, const char* uri)
{
	using namespace std;
	string server_uri(uri);
	size_t p1 = server_uri.find("://");
	if (p1 == string::npos)
		return -1;
	server_uri[p1] = 0;
	if (strcmp(server_uri.data(), "ip") == 0) {
		size_t p2 = server_uri.find(':', p1 + 3);
		server_uri[p2] = 0;
		int port = 0;	sscanf(&server_uri[p2 + 1], "%d", &port);
		char* ip = &server_uri[p1 + 3];
		if (sk_tcp_addr(bindaddr, ip, port)) {
			return -2;
		}
	}
	else if (strcmp(server_uri.data(), "un") == 0) {
		sk_unix_addr(bindaddr, &server_uri[p1 + 3]);
	}
	return 0;
}

int sk_tcp_addr(sockaddr* paddr,int* plen,const char* url,int port)
{
	struct addrinfo hints={0},*pInfos=0;
	hints.ai_family= AF_UNSPEC;
	hints.ai_socktype= SOCK_STREAM;
	hints.ai_protocol= IPPROTO_TCP;
	char portbuf[22];	snprintf(portbuf,sizeof(portbuf),"%d",port);
	int rc= getaddrinfo(url,portbuf,&hints,&pInfos);
	if(rc!=0) return -1;
	if(pInfos) {
		memcpy(paddr,pInfos->ai_addr,*plen=(int)pInfos->ai_addrlen);
		freeaddrinfo(pInfos);
		return 0;
	}
	return 1;
}

int sk_tcp_addr(sockaddr_ex& saex,const char* url,int port)
{
	int len=sizeof(saex);
	return sk_tcp_addr(&saex,&len,url,port);
}

int sk_solve_url(sockaddr_ex* paddrs,int* count, const char* url,int port)
{
	struct addrinfo hints={0},*pInfos=0;
	hints.ai_family= AF_UNSPEC;
	hints.ai_socktype= SOCK_STREAM;
	hints.ai_protocol= IPPROTO_TCP;
	char portbuf[22];	snprintf(portbuf,sizeof(portbuf),"%d",port);
	int rc= getaddrinfo(url,portbuf,&hints,&pInfos);
	if(rc!=0) return -1;
	if(pInfos) {
		int i=0;	struct addrinfo* pi=pInfos;
		for(;i<*count && pi!=NULL;++i,pi=pi->ai_next) {
			memcpy(&paddrs[i],pi->ai_addr,pi->ai_addrlen);
		}
		*count=i;
		i= (pi==NULL?0:2);
		freeaddrinfo(pInfos);
		return  i;
	}
	*count=0;
	return 1;
}

int sk_solve_addr(const sockaddr* paddr,char* addrstrbuf,int buflen)
{
	switch(paddr->sa_family)
	{
	case AF_INET:
		{
			sockaddr_in& addrin=*(sockaddr_in*)paddr;
			unsigned char* bts=(unsigned char*)&addrin.sin_addr;
			if(snprintf(addrstrbuf,buflen,"%d.%d.%d.%d:%d",bts[0],bts[1],bts[2],bts[3],ntohs(addrin.sin_port))<0)
				return 1;
		}
		break;
	case AF_INET6:
		{
			sockaddr_in6& addrin6=*(sockaddr_in6*)paddr;
			unsigned short* sts=(unsigned short*)&addrin6.sin6_addr;
			if(snprintf(addrstrbuf,buflen,"[%x:%x:%x:%x:%x:%x:%x:%x]:%d",ntohs(sts[0]),ntohs(sts[1]),ntohs(sts[2])
				,ntohs(sts[3]),ntohs(sts[4]),ntohs(sts[5]),ntohs(sts[6]),ntohs(sts[7]),ntohs(addrin6.sin6_port) )<0)
				return 1;
		}
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

int sk_get_ip(const sockaddr* paddr,char* ipbuf,int buflen)
{
	switch(paddr->sa_family)
	{
	case AF_INET:
		{
			sockaddr_in& addrin=*(sockaddr_in*)paddr;
			unsigned char* bts=(unsigned char*)&addrin.sin_addr;
			if(snprintf(ipbuf,buflen,"%d.%d.%d.%d",bts[0],bts[1],bts[2],bts[3])<0)
				return 1;
		}
		break;
	case AF_INET6:
		{
			sockaddr_in6& addrin6=*(sockaddr_in6*)paddr;
			unsigned short* sts=(unsigned short*)&addrin6.sin6_addr;
			if(snprintf(ipbuf,buflen,"%x:%x:%x:%x:%x:%x:%x:%x",ntohs(sts[0]),ntohs(sts[1]),ntohs(sts[2])
				,ntohs(sts[3]),ntohs(sts[4]),ntohs(sts[5]),ntohs(sts[6]),ntohs(sts[7]) )<0)
				return 1;
		}
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

int sk_get_port(const sockaddr* paddr,int* pport)
{
	if(paddr==0||pport==0) return -1;
	switch(paddr->sa_family)
	{
	case AF_INET:
		{
			sockaddr_in& addrin=*(sockaddr_in*)paddr;
			*pport=ntohs(addrin.sin_port);
		}
		break;
	case AF_INET6:
		{
			sockaddr_in6& addrin6=*(sockaddr_in6*)paddr;
			*pport=ntohs(addrin6.sin6_port);
		}
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

int sk_solve_addr(const sockaddr* paddr,char* url_buf,int urllen,char* srv_buf,int srvlen,int flag)
{
	return getnameinfo(paddr,sockaddr_ex::sa_len(paddr),url_buf,urllen,srv_buf,srvlen,flag);
}

SOCKET sk_create(int af,int type,int prot)
{
	return socket(af,type,prot);
}

int sk_close(SOCKET sock)
{
#if defined(__linux) || defined(__APPLE__)
	return close(sock);
#elif defined(_WIN32)
	return closesocket(sock);
#else
	assert(!("not supported"));
	return -1;
#endif
}

int sk_set_nonblock(SOCKET sock,bool bNblock)
{
#if defined(__linux) || defined(__APPLE__)
    int flag = fcntl(sock,F_GETFL,0);
    return fcntl(sock,F_SETFL,bNblock?(flag|O_NONBLOCK):(flag& ~O_NONBLOCK));
 //   return ioctl(sock,FIONBIO,&on);
#else	//win
    unsigned long on= bNblock?1:0;
	return ioctlsocket(sock,FIONBIO,&on);
#endif
}

int sk_set_tcpnodelay(SOCKET sock,bool bNodelay)
{
	int on= bNodelay?1:0;
	return setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(char*)&on,sizeof(on));
}
//------------------
int sk_connect_ex(SOCKET sock,const sockaddr* paddr,int salen,unsigned int tm_ms)
{
	if(sk_set_nonblock(sock,true)<0)
		return -1;

#ifdef __linux
	struct pollfd fds = {};
	fds.fd=sock;
	fds.events=POLLOUT | POLLERR | POLLHUP | POLLNVAL;
	sockerr_set(0);
	int ret = connect(sock, paddr, salen);
	int wr = sockerr_get();
	if (wr != WSAEINPROGRESS && wr != WSAEWOULDBLOCK && wr != 0) {
		wr = -1;	goto _ret;
	}
	ret = poll(&fds, 1, tm_ms);
#else //windows?
	struct timeval tmo={(long)tm_ms/1000,(int)(tm_ms%1000)*1000};
	struct timeval* ptm = tm_ms == 0 ? NULL : &tmo;
	fd_set w;	FD_ZERO(&w);
	sockerr_set(0);
	int ret=connect(sock,paddr,salen);
	int wr=sockerr_get();
	if(wr!=WSAEINPROGRESS && wr!=WSAEWOULDBLOCK && wr!=0) {
		wr=-1;	goto _ret;
	}
	FD_SET(sock,&w);
	ret=select((int)(sock+1),NULL,&w,NULL,ptm);
#endif
	if (ret > 0) {	//poll/select OK!
		wr = 0;
	}
	else if (ret == 0) {//failed.timed-out
		sockerr_set(wr = WSAETIMEDOUT);
	}
	else {//error!
		wr = -2;
	}
_ret:
	sk_set_nonblock(sock,false);
	return wr;
}

int sk_connect_ex(SOCKET sock,const sockaddr_ex& addr,unsigned int tm_ms)
{
	return sk_connect_ex(sock,&addr,addr.addr_len(),tm_ms);
}

SOCKET sk_accept1(SOCKET serv_sock)
{
	sockaddr_ex addr;
	socklen_t len=sizeof(addr);
	return accept(serv_sock,&addr,&len);
}

SOCKET sk_accept2(SOCKET serv_sock, sockaddr_ex& addr)
{
	socklen_t len = sizeof(addr);
	return accept(serv_sock, &addr, &len);
}

int send_all(SOCKET sock,const void* buf,int len,int flag)
{
	for(int offset=0;offset<len;) {
		int r=send(sock,static_cast<const char*>(buf)+offset,min(len-offset,1024*1024),flag);
		if(r<=0) return r;
		offset+=r;
	}
	return len;
}

int recv_all(SOCKET sock,void* buf,int len,int flag)
{
	for(int offset=0;offset<len;) {
		int r=recv(sock,((char*)buf)+offset,min(len-offset,1024*1024),flag);
		if(r<=0) return r;
		offset+=r;
	}
	return len;
}
