/*
 * socket_common.h
 *
 *  Created on: 2013-5-21
 *      Author: ltmit
 */

#ifndef SOCKET_COMMON_H_
#define SOCKET_COMMON_H_

//common include files
#ifdef _WIN32

//#define _WIN32_WINNT 0x0502
#include <ws2tcpip.h>
#define MSG_NOSIGNAL	0	//dummy

#else  //linux
typedef int SOCKET;
#define INVALID_SOCKET (SOCKET)(-1)
#define SOCKET_ERROR	(-1)
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

#ifndef WSABASEERR
#define WSABASEERR 10000
#define WSAEINTR 10004L
#define WSAEBADF 10009L
#define WSAEACCES 10013L
#define WSAEFAULT 10014L
#define WSAEINVAL 10022L
#define WSAEMFILE 10024L
#define WSAEWOULDBLOCK 10035L
#define WSAEINPROGRESS 10036L
#define WSAEALREADY 10037L
#define WSAENOTSOCK 10038L
#define WSAEDESTADDRREQ 10039L
#define WSAEMSGSIZE 10040L
#define WSAEPROTOTYPE 10041L
#define WSAENOPROTOOPT 10042L
#define WSAEPROTONOSUPPORT 10043L
#define WSAESOCKTNOSUPPORT 10044L
#define WSAEOPNOTSUPP 10045L
#define WSAEPFNOSUPPORT 10046L
#define WSAEAFNOSUPPORT 10047L
#define WSAEADDRINUSE 10048L
#define WSAEADDRNOTAVAIL 10049L
#define WSAENETDOWN 10050L
#define WSAENETUNREACH 10051L
#define WSAENETRESET 10052L
#define WSAECONNABORTED 10053L
#define WSAECONNRESET 10054L
#define WSAENOBUFS 10055L
#define WSAEISCONN 10056L
#define WSAENOTCONN 10057L
#define WSAESHUTDOWN 10058L
#define WSAETOOMANYREFS 10059L
#define WSAETIMEDOUT 10060L
#define WSAECONNREFUSED 10061L
#define WSAELOOP 10062L
#define WSAENAMETOOLONG 10063L
#define WSAEHOSTDOWN 10064L
#define WSAEHOSTUNREACH 10065L
#define WSAENOTEMPTY 10066L
#define WSAEPROCLIM 10067L
#define WSAEUSERS 10068L
#define WSAEDQUOT 10069L
#define WSAESTALE 10070L
#define WSAEREMOTE 10071L
#define WSAEMAX	10072L
#endif	//WSABASEERR
#endif
#include <stdio.h>
#include <string.h>
#ifndef _GLIBCXX_STRING
#include <string>
#endif

#ifndef __in
#define __in
#define __out
#define __inout
#endif

#ifdef __APPLE__
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0//SO_NOSIGPIPE
#endif
#endif

int sk_sys_init();

int sk_sys_deinit();

int sockerr_get();
void sockerr_set(int e);

class sockaddr_ex:public sockaddr{
	char sa_data_ex[/*sizeof(sockaddr_in6)*/112-sizeof(sockaddr)];
public:
	sockaddr_ex() { memset(this,0,sizeof(sockaddr_ex)); }

	//only support IPv4/v6
	static inline int sa_len(const sockaddr* paddr) {
		switch(paddr->sa_family)
		{
		case AF_INET:
			return sizeof(sockaddr_in);
		case AF_INET6:
			return sizeof(sockaddr_in6);
		case AF_UNIX:
			return 110;//sizeof(sockaddr_un);
		default:
			return 0;
		}
	}

	inline int addr_len() const {
		return sa_len(this);
	}

	inline sockaddr_in* inaddr_ptr() {
		return sa_family==AF_INET?(sockaddr_in*)this:NULL;
	}

	inline sockaddr_in6* in6addr_ptr() {
		return sa_family==AF_INET6?(sockaddr_in6*)this:NULL;
	}

	inline bool operator ==(const sockaddr_ex& sa) const {
		int cmp_len=addr_len();
		if(cmp_len==0) cmp_len=sizeof(sa);
		return memcmp(this,&sa,cmp_len)==0;
	}

	inline bool operator <(const sockaddr_ex& sa) const {
		int cmp_len=addr_len();
		if(cmp_len==0) cmp_len=sizeof(sa);
		return memcmp(this,&sa,cmp_len)<0;
	}

	inline sockaddr_ex& operator=(const sockaddr_ex& sa) {
		memcpy(this,&sa,sizeof(sa));
		return *this;
	}

	inline int bindto(SOCKET sock) {
		return ::bind(sock,this,addr_len());
	}
};

SOCKET sk_create(int af,int type,int prot);

int sk_close(SOCKET sock);

int sk_set_nonblock(__inout SOCKET sock,bool bNblock);

int sk_set_tcpnodelay(SOCKET sock,bool bNodelay);

int sk_unix_addr(sockaddr_ex& saex,const char* uri);

int sk_addr_from_uri(sockaddr_ex& saex, const char* uri);

//parse url(ip), and get the 1st net address an fill to *paddr. return real length
int sk_tcp_addr(__out sockaddr* paddr,__inout int* plen,__in const char* url,__in int port);

int sk_tcp_addr(__out sockaddr_ex& saex,__in const char* url,int port);

int sk_solve_url(__out sockaddr_ex* paddrs,__inout int* addr_count,__in const char* url,int port);

int sk_solve_addr(__in const sockaddr* paddr,char* url_buf,int urllen,char* srv_buf,int srvlen,int flag=0);

int sk_solve_addr(const sockaddr* paddr,char* addrstrbuf,int buflen);

int sk_get_port(const sockaddr* addr,int* pport);

int sk_get_ip(const sockaddr* addr,char* ipbuf,int buflen);
/**
 * socket connect with timeout(if tm_ms==0,then wait for INFINITE time.)
 * retval:	 0->succeed	  WSAETIMEOUT->time out   -1: invalid sock  -2: select() error
 *
 * */
int sk_connect_ex(SOCKET sock,const sockaddr* paddr,int salen,unsigned int tm_ms=0);

int sk_connect_ex(SOCKET sock,const sockaddr_ex& addr,unsigned int tm_ms);

SOCKET sk_accept1(SOCKET serv_sock);

SOCKET sk_accept2(SOCKET serv_sock, sockaddr_ex& addr);

int send_all(SOCKET sock,const void* buf,int len,int flag=MSG_NOSIGNAL);

int recv_all(SOCKET sock,void* buf,int len,int flag=MSG_NOSIGNAL);

template<class T>
static inline int send_s_obj(SOCKET sock,const T& obj,int flag=MSG_NOSIGNAL) {
	return send_all(sock,&obj,sizeof(obj),flag);
}

template<class T>
static inline int recv_s_obj(SOCKET sock, T& obj,int flag=MSG_NOSIGNAL) {
	return recv_all(sock,&obj,sizeof(obj),flag);
}

///////////////////////////////////////////////
template<class T>
static inline void cxx_recv(SOCKET sock,T& obj) {
	if(recv_s_obj(sock,obj)<=0)
		throw (int)sockerr_get();
}

static inline void cxx_recv(SOCKET sock,void* buf,int sz) {
	if(recv_all(sock,buf,sz)<=0)
		throw (int)sockerr_get();
}

template<class T>
static inline void cxx_send(SOCKET sock,const T& obj) {
	if(send_s_obj(sock,obj)<=0)
		throw (int)sockerr_get();
}

static inline void cxx_send(SOCKET sock,const void* buf,int sz) {
	if(send_all(sock,buf,sz)<=0)
		throw (int)sockerr_get();
}

static inline void cxx_setsopt(SOCKET sock,int level,int name,const void* buf,int size) {
	if(setsockopt(sock,level,name,(const char*)buf,size)!=0)
		throw (int)sockerr_get();
}

///////////////extension v2////////////////////////////


static inline std::string cxx_recv_string(SOCKET sock) {
	int len=0;
	cxx_recv(sock,len);
	std::string str(len,'\0');
	cxx_recv(sock,(char*)str.data(),len);
	return str;
}

static inline void cxx_send_string(SOCKET sock,const std::string& str) {
	int len=(int)str.size();
	cxx_send(sock,len);
	cxx_send(sock,str.data(),len);
}



//-------------------------------------------
class sockwrapper{
	sockwrapper(const sockwrapper&) ;
protected:
	SOCKET _sock;
public:
	sockwrapper():_sock(INVALID_SOCKET) {}
	explicit sockwrapper(SOCKET sock):_sock(sock) {}
	~sockwrapper() {
		if(_sock!=INVALID_SOCKET) {
			close();
		}
	}
	SOCKET attach(SOCKET newsock) {
		SOCKET olds=_sock;
		_sock=newsock;
		return olds;
	}
	SOCKET detach() {
		return attach(INVALID_SOCKET);
	}
	operator SOCKET() {
		return _sock;
	}
	int close() {
		return sk_close(_sock);
	}
	bool is_valid() const {
		return _sock!=INVALID_SOCKET;
	}
	int bind(const sockaddr* paddr,socklen_t adlen) {
		return ::bind(_sock,paddr,adlen);
	}
	int bind(const sockaddr_ex& addr) {
		return bind(&addr,addr.addr_len());
	}
	int listen(int backlogs) {
		return ::listen(_sock,backlogs);
	}
	int connect_ex(const sockaddr* paddr,socklen_t adlen,unsigned int tmo=3000) {
		return sk_connect_ex(_sock,paddr,adlen,tmo);
	}
	int connect_ex(const sockaddr_ex& addr,unsigned int tmo=3000) {
		return sk_connect_ex(_sock,addr,tmo);
	}
	SOCKET accept0() {
		return sk_accept1(_sock);
	}
	int recv(void* buf,int len,int flags=MSG_NOSIGNAL) {
		return ::recv(_sock,(char*)buf,len,flags);
	}
	int send(const void* buf,int buflen,int flags=MSG_NOSIGNAL) {
		return ::send(_sock,(const char*)buf,buflen,flags);
	}

};


#endif /* SOCKET_COMMON_H_ */
