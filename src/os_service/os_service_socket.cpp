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

#include "os_service_socket.hpp"
#include "problem_detect.hpp"

_OSServiceSocket::_OSServiceSocket(unsigned int port, int timeout)
{
    this->_is_init = false ;
    this->_fd = 0 ;
    this->_timeout = timeout ;
    memset (&this->_sockAddress, 0, sizeof(sockaddr_in)) ;
    memset (&this->_peerAddress, 0, sizeof(sockaddr_in)) ;
    this->_peerAddressLen = sizeof(this->_peerAddress) ;
    this->_sockAddress.sin_family = AF_INET ;
    this->_sockAddress.sin_addr.s_addr = htonl (INADDR_ANY) ;
    this->_sockAddress.sin_port = htons (port) ;
    this->_addressLen = sizeof(this->_sockAddress) ;
}

_OSServiceSocket::_OSServiceSocket ()
{
    this->_is_init = false ;
    this->_fd = 0 ;
    this->_timeout = 0 ;

    memset(&this->_sockAddress, 0, sizeof(sockaddr_in));
    memset(&this->_peerAddress, 0, sizeof(sockaddr_in));
    this->_peerAddressLen = sizeof(this->_peerAddress);
    this->_addressLen = sizeof(this->_sockAddress);
}

_OSServiceSocket::_OSServiceSocket (const char *pHostname, unsigned int port, int timeout)
{
    struct hostent *hp ;
    this->_is_init = false ;
    this->_timeout = timeout ;
    this->_fd = 0 ;

    memset(&this->_sockAddress, 0, sizeof(sockaddr_in));
    memset(&this->_peerAddress, 0, sizeof(sockaddr_in));

    this->_peerAddressLen = sizeof(this->_peerAddress) ;
    this->_sockAddress.sin_family = AF_INET ;

    if((hp = gethostbyname(pHostname))) {
        this->_sockAddress.sin_addr.s_addr = *((int*)hp->h_addr_list[0]);
    } else {
        this->_sockAddress.sin_addr.s_addr = inet_addr(pHostname);
    }

    this->_sockAddress.sin_port = htons(port);
    this->_addressLen = sizeof(this->_sockAddress);
}

_OSServiceSocket::_OSServiceSocket(SOCKET *sock, int timeout)
{
    int rc = DB_OK ;
    this->_fd = *sock ;
    this->_is_init = true ;
    this->_timeout = timeout ;
    this->_addressLen = sizeof(this->_sockAddress);

    memset(&this->_peerAddress, 0, sizeof(sockaddr_in));

    this->_peerAddressLen = sizeof(this->_peerAddress);

    rc = getsockname(this->_fd, (sockaddr*)&this->_sockAddress, &this->_addressLen);
    if(rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "Failed to get sock name, error = %d", SOCKET_GET_LAST_ERROR);
        this->_is_init = false ;
    } else {
        rc = getpeername(this->_fd, (sockaddr*)&this->_peerAddress, &this->_peerAddressLen);
        PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to get peer name, error = %d", SOCKET_GET_LAST_ERROR);
    }

    done:
        return;
    error:
        goto done ;
}

int _OSServiceSocket::initSocket ()
{
    int rc = DB_OK ;
    if(this->_is_init) {
        goto done ;
    }

    memset(&this->_peerAddress, 0, sizeof(sockaddr_in));

    this->_peerAddressLen = sizeof(this->_peerAddress) ;
    this->_fd =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(this->_fd == -1) {
        PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to initialize socket, error = %d",
                                SOCKET_GET_LAST_ERROR);
    }

    this->_is_init =true ;

    setTimeout(this->_timeout);

    done:
        return rc;
    error:
        goto done;
}

int _OSServiceSocket::setSocketLi(int lOnOff, int linger)
{
    int rc = DB_OK;
    struct linger _linger;
    _linger.l_onoff = lOnOff;
    _linger.l_linger = linger;

    rc = setsockopt(this->_fd, SOL_SOCKET, SO_LINGER, (const char*)&_linger,
                    sizeof(_linger));
    return rc ;
}

void _OSServiceSocket::setAddress(const char *pHostname, unsigned int port) {
    struct hostent *hp ;

    memset(&this->_sockAddress, 0, sizeof(sockaddr_in)) ;
    memset(&this->_peerAddress, 0, sizeof(sockaddr_in));

    this->_peerAddressLen = sizeof(this->_peerAddress) ;
    this->_sockAddress.sin_family = AF_INET ;

    if(( hp = gethostbyname(pHostname))) {
        this->_sockAddress.sin_addr.s_addr = *((int*)hp->h_addr_list[0]);
    } else {
        this->_sockAddress.sin_addr.s_addr = inet_addr(pHostname);
    }

    this->_sockAddress.sin_port = htons(port);
    this->_addressLen = sizeof(this->_sockAddress);
}

int _OSServiceSocket::bindListen () {
    int rc = DB_OK;
    int temp = 1;

    rc = setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(int));
    if(rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_WARNING, "Failed to setsockopt SO_REUSEADDR, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    rc = setSocketLi(1, 30);
    if(rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_WARNING, "Failed to setsockopt SO_LINGER, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    rc = ::bind(this->_fd, (struct sockaddr*)&this->_sockAddress, this->_addressLen);
    if(rc) {
        PROBLEM_DETECT_RC_CHECK( DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to bind socket, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    rc = listen(this->_fd, SOMAXCONN);
    if(rc) {
        PROBLEM_DETECT_RC_CHECK( DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to listen socket, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    done :
        return rc ;
    error :
        close () ;
        goto done ;
}

int _OSServiceSocket::send(const char *pMsg, int len, int timeout, int flags) {
    int rc = DB_OK ;
    SOCKET maxFD;

    #ifdef _WINDOWS
        maxFD = 0;
    #else
        maxFD = this->_fd ;
    #endif

    struct timeval maxSelectTime ;
    fd_set fds ;

    maxSelectTime.tv_sec = timeout / 1000000 ;
    maxSelectTime.tv_usec = timeout % 1000000 ;

    if (len == 0) {
        return DB_OK ;
    }

    while(true) {
        FD_ZERO(&fds);
        FD_SET(this->_fd, &fds);

        rc = select((int)(maxFD + 1), NULL, &fds, NULL, timeout>=0?&maxSelectTime:NULL);
        if (rc == 0) {
            rc = DB_TIMEOUT ;
            goto done ;
        }

        if (rc < 0) {
            rc = SOCKET_GET_LAST_ERROR ;

            if (OSS_EINTR == rc) {
                continue ;
            }
            PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to select from socket, rc = %d", rc);
        }
        if(FD_ISSET(this->_fd, &fds)) {
            break ;
        }
    }
    while(len > 0) {
        rc = ::send(this->_fd, pMsg, len, MSG_NOSIGNAL | flags);
        if(rc == -1) {
            PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to send, rc = %d", SOCKET_GET_LAST_ERROR);
        }

        len -= rc ;
        pMsg += rc ;
    }

    rc = DB_OK ;

    done:
        return rc ;
    error:
        goto done ;
}

bool _OSServiceSocket::isConnected () const
{
    int rc = DB_OK ;
    rc = ::send(this->_fd, "", 0, MSG_NOSIGNAL);

    return rc >= 0;
}

#define MAX_RECV_RETRIES 5
int _OSServiceSocket::recv(char *pMsg, int len, int timeout, int flags) {
    int rc = DB_OK ;
    int retries = 0 ;
    int maxFD = (int)this->_fd ;
    struct timeval maxSelectTime ;
    fd_set fds ;

    if(len == 0) {
        return DB_OK;
    }

    maxSelectTime.tv_sec = timeout / 1000000;
    maxSelectTime.tv_usec = timeout % 1000000;

    while(true) {
        FD_ZERO(&fds);
        FD_SET(this->_fd, &fds);
        rc = select(maxFD+1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL);

        if(rc == 0) {
            rc = DB_TIMEOUT ;
            goto done ;
        }

        if(rc < 0) {
            rc = SOCKET_GET_LAST_ERROR;

            if(rc == OSS_EINTR) {
                continue ;
            }

            PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to select from socket, rc = %d", rc);
        }
        if(FD_ISSET(this->_fd, &fds)) {
            break ;
        }
    }

    while(len > 0)
    {
        rc = ::recv(this->_fd, pMsg, len, MSG_NOSIGNAL | flags);

        if(rc > 0) {
            if(flags & MSG_PEEK){
                goto done ;
            }

            len -= rc ;
            pMsg += rc ;
        } else if(rc == 0) {
            PROBLEM_DETECT_RC_CHECK(DB_NETWORK_CLOSE, PROBLEM_DETECT_WARNING, "Peer unexpected shutdown");
        } else {
            rc = SOCKET_GET_LAST_ERROR;

            if(( rc == OSS_EAGAIN || rc == EWOULDBLOCK) && this->_timeout >  0) {
                PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Recv() timeout: rc = %d", rc);
            }

            if((rc == OSS_EINTR) && (retries < MAX_RECV_RETRIES)) {
                retries ++ ;
                continue ;
            }
            PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Recv() Failed: rc = %d", rc);
        }
    }

    rc = DB_OK ;

    done:
        return rc ;
    error:
        goto done ;
}

int _OSServiceSocket::recvNF(char *pMsg, int len, int timeout)
{
    int rc = DB_OK;
    int retries = 0;
    int maxFD = (int)this->_fd;

    struct timeval maxSelectTime ;
    fd_set fds ;

    if(len == 0) {
        return DB_OK;
    }

    maxSelectTime.tv_sec = timeout / 1000000;
    maxSelectTime.tv_usec = timeout % 1000000;

    while(true) {
        FD_ZERO(&fds);
        FD_SET(this->_fd, &fds);
        rc = select(maxFD + 1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL);

        if(rc == 0) {
            rc = DB_TIMEOUT ;
            goto done ;
        }

        if(0 > rc) {
            rc = SOCKET_GET_LAST_ERROR ;

            if(OSS_EINTR == rc) {
                continue ;
            }
            PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to select from socket, rc = %d", rc);
        }

        if(FD_ISSET(this->_fd, &fds)) {
            break ;
        }
    }

    rc = ::recv(this->_fd, pMsg, len, MSG_NOSIGNAL);

    if(rc > 0) {
        len = rc ;
    } else if(rc == 0) {
        PROBLEM_DETECT_RC_CHECK(DB_NETWORK_CLOSE, PROBLEM_DETECT_WARNING, "Peer unexpected shutdown");
    } else {
        rc = SOCKET_GET_LAST_ERROR ;

        if((EAGAIN == rc || EWOULDBLOCK == rc) && this->_timeout > 0) {
            PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Recv() timeout: rc = %d", rc);
        }

        if(( OSS_EINTR == rc) && (retries < MAX_RECV_RETRIES)) {
            retries ++ ;
        }

        PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Recv() Failed: rc = %d", rc);
    }

    rc = DB_OK;

    done:
        return rc;
    error:
        goto done;
}

int _OSServiceSocket::connect ()
{
    int rc = DB_OK;

    rc = ::connect(this->_fd, (struct sockaddr *)&this->_sockAddress, this->_addressLen);
    if(rc) {
        PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to connect, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    rc = getsockname(this->_fd, (sockaddr*)&this->_sockAddress, &this->_addressLen);
    if(rc) {
        PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to get local address, rc = %d", rc);
    }

    rc = getpeername(this->_fd, (sockaddr*)&this->_peerAddress, &this->_peerAddressLen);
    if(rc) {
        PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to get peer address, rc = %d", rc);
    }

    done:
        return rc;
    error:
        goto done;
}

void _OSServiceSocket::close () {
    if(this->_is_init) {
        int i = 0 ;
        i = ::closesocket(this->_fd);

        if(i < 0) {
            i = -1 ;
        }

        this->_is_init = false ;
    }
}

int _OSServiceSocket::accept(SOCKET *sock, struct sockaddr *addr, socklen_t *addrlen, int timeout) {
    int rc = DB_OK ;
    int maxFD = (int)this->_fd ;
    struct timeval maxSelectTime ;

    fd_set fds ;
    maxSelectTime.tv_sec = timeout / 1000000;
    maxSelectTime.tv_usec = timeout % 1000000;

    while(true) {
        FD_ZERO(&fds);
        FD_SET(this->_fd, &fds);

        rc = select(maxFD + 1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL);

        if(0 == rc) {
            *sock = 0 ;
            rc = DB_TIMEOUT ;
            goto done ;
        }

        if(0 > rc) {
            rc = SOCKET_GET_LAST_ERROR;
            if(OSS_EINTR == rc) {
                continue ;
            }

            PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to select from socket, rc = %d",
                                    SOCKET_GET_LAST_ERROR);
        }

        if(FD_ISSET(this->_fd, &fds)) {
            break ;
        }
    }

    rc = DB_OK;

    *sock = ::accept(this->_fd, addr, addrlen);
    if(*sock == -1) {
        PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to accept socket, rc = %d",
                                SOCKET_GET_LAST_ERROR);
    }

    done:
        return rc;
    error:
        close();
        goto done;
}

int _OSServiceSocket::disableNagle () {
    int rc = DB_OK;
    int temp = 1;

    rc = setsockopt(this->_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &temp, sizeof(int)) ;
    if(rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_WARNING, "Failed to setsockopt, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    rc = setsockopt(this->_fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &temp, sizeof(int)) ;
    if(rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_WARNING, "Failed to setsockopt, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    return rc ;
}

unsigned int _OSServiceSocket::_getPort(sockaddr_in *addr) {
    return ntohs(addr->sin_port);
}

int _OSServiceSocket::_getAddress(sockaddr_in *addr, char *pAddress, unsigned int length) {
    int rc = DB_OK;
    length = length < NI_MAXHOST ? length : NI_MAXHOST;

    rc = getnameinfo((struct sockaddr *)addr, sizeof(sockaddr), pAddress, length, NULL,
                    0, NI_NUMERICHOST);
    if(rc) {
        PROBLEM_DETECT_RC_CHECK(DB_NETWORK, PROBLEM_DETECT_ERROR, "Failed to getnameinfo, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    done:
        return rc;
    error:
        goto done;
}

int _OSServiceSocket::setAnSYN () {
#ifdef _WINDOWS
    u_long nonblock = 1;
	return ioctlsocket(_fd, FIONBIO, &nonblock);
#else
    return fcntl(this->_fd, F_SETFL, O_NONBLOCK | fcntl(this->_fd, F_GETFL, 0));
#endif
}

unsigned int _OSServiceSocket::getLocalPort () {
    return _getPort(&this->_sockAddress);
}

unsigned int _OSServiceSocket::getPeerPort () {
    return _getPort(&this->_peerAddress);
}

int _OSServiceSocket::getLocalAddress(char * pAddress, unsigned int length) {
    return _getAddress(&this->_sockAddress, pAddress, length);
}

int _OSServiceSocket::getPeerAddress(char * pAddress, unsigned int length) {
    return _getAddress(&this->_peerAddress, pAddress, length);
}

int _OSServiceSocket::setTimeout(int seconds) {
    int rc = DB_OK ;
    struct timeval tv ;
    tv.tv_sec = seconds ;
    tv.tv_usec = 0 ;

    rc = setsockopt(this->_fd, SOL_SOCKET, SO_RCVTIMEO,(char*)&tv, sizeof(tv));
    if(rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_WARNING, "Failed to setsockopt, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    rc = setsockopt(this->_fd, SOL_SOCKET, SO_SNDTIMEO,(char*)&tv, sizeof(tv));
    if(rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_WARNING, "Failed to setsockopt, rc = %d", SOCKET_GET_LAST_ERROR);
    }

    return rc ;
}

int _OSServiceSocket::getHostname(char *pName, int nameLen) {
    return gethostname(pName, nameLen);
}

int _OSServiceSocket::getPort(const char *pServiceName, unsigned short &port) {
    int rc = DB_OK ;
    struct servent *servinfo ;

    servinfo = getservbyname(pServiceName, "tcp");

    if(servinfo) {
        port = (unsigned short)ntohs(servinfo->s_port) ;
    } else {
        port = atoi(pServiceName);
    }

    return rc ;
}
