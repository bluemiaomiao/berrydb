/*******************************************************************************
                  Copyright (C) 2021 BerryDB Software Inc.
This application is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License, Version 3, as
published by the Free Software Foundation.

This application is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR PARTICULAR PURPOSE, See the GNU Affero General Public License for more
details.

You should have received a copy of the GNU Affero General Public License along
with this application. If not, see <http://www.gnu.org/license/>
*******************************************************************************/

#ifndef _OS_SERVICE_SOCKET_HPP__
#define _OS_SERVICE_SOCKET_HPP__

#include "core.hpp"

#ifdef _WINDOWS
    #define SOCKET_GET_LAST_ERROR          WSAGetLastError()

    #define OSS_EAGAIN	                   WSAEINPROGRESS
    #define OSS_EINTR	                   WSAEINTR
#else
    #define SOCKET_GET_LAST_ERROR          errno
    #define OSS_EAGAIN	                   EAGAIN
    #define OSS_EINTR	                   EINTR
    #define closesocket	                   close
#endif

#define OS_SERVICE_SOCKET_DFT_TIMEOUT      10000
#define OS_SERVICE_MAX_HOSTNAME            NI_MAXHOST
#define OS_SERVICE_MAX_SERVICE_NAME        NI_MAXSERV

#ifndef _WINDOWS
    typedef int SOCKET;
#endif

class _OSServiceSocket {
private:
	int _fd;                                      // 套接字句柄
	socklen_t _addressLen;                        // 套接字地址长度
	socklen_t _peerAddressLen;                    // 对端套接字地址长度
	struct sockaddr_in _sockAddress;              // 套接字地址
	struct sockaddr_in _peerAddress;              // 对端套接字地址
	bool _is_init;                                // 初始化标志
	int _timeout;                                 // 超时时间
protected:
	unsigned int _getPort(sockaddr_in *addr);
	int _getAddress (sockaddr_in *addr, char *pAddr, unsigned int len);
public:
    int setAnSYN() ;
	int setSocketLi(int lOnOff, int linger);
	void setAddress(const char *pHostname, unsigned int port);

	_OSServiceSocket();
	_OSServiceSocket(int *sock, int timeout=0);
	_OSServiceSocket(unsigned int port, int timeout=0);
	_OSServiceSocket(const char *pHostname, unsigned int port, int timeout=0);
	~_OSServiceSocket() { close(); }

	int initSocket();
	int bindListen();
	int connect();
	void close();
	int accept(int *sock, struct sockaddr *addr, socklen_t *addrLen, int timeout=OS_SERVICE_SOCKET_DFT_TIMEOUT);
	int send(const char *pMsg, int len, int timeout=OS_SERVICE_SOCKET_DFT_TIMEOUT, int flags=0);
	int recv(char *pMsg, int len, int timeout=OS_SERVICE_SOCKET_DFT_TIMEOUT, int flags=0);
	int recvNF(char *pMsg, int len, int timeout=OS_SERVICE_SOCKET_DFT_TIMEOUT);

	// Nagle算法是TCP/IP协议栈中将多个小数据包汇聚成一个大数据包打包发送的协议, 数据库系统通常关闭该网络功能
	int disableNagle();
	bool isConnected() const;
	int setTimeout(int secs);
	static int getPort(const char *pServiceName, unsigned short &port);
	unsigned int getPeerPort();
	unsigned int getLocalPort();
	int getPeerAddress(char *pAddress, unsigned int len);
	int getLocalAddress(char *pAddress, unsigned int len);
	static int getHostname(char *pName, int nameLen);
};

typedef class _OSServiceSocket OSServiceSocket;

#endif

