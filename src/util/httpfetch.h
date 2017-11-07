/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013 - 2015  LiteSpeed Technologies, Inc.                 *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program. If not, see http://www.gnu.org/licenses/.      *
*****************************************************************************/
#ifndef HTTPFETCH_H
#define HTTPFETCH_H

#include <lsdef.h>
#include <log4cxx/logger.h>
#include <sslpp/sslconnection.h>
#include <util/autobuf.h>
#include <util/hashstringmap.h>
#include <stddef.h>
#include <time.h>

class GSockAddr;
class HttpFetch;
class HttpFetchDriver;
using namespace LOG4CXX_NS;

typedef int (*HFProcessFn)(void *, HttpFetch *);

class AdnsReq;
class VMemBuf;
class HttpFetch
{
    int         m_iFdHttp;
    VMemBuf    *m_pBuf;
    int         m_iStatusCode;
    char       *m_pReqBuf;
    int         m_iReqBufLen;
    int         m_iReqSent;
    char       *m_pExtraReqHdrs;
    int         m_iReqHeaderLen;
    int         m_iHostLen;
    short       m_iReqState;
    char        m_iNonBlocking;
    char        m_iEnableDriver;
    const char *m_pReqBody;
    int64_t     m_iReqBodyLen;
    int         m_iConnTimeout;

    int64_t     m_iRespBodyLen;
    char       *m_pRespContentType;
    int64_t     m_iRespBodyRead;
    char       *m_psProxyServerAddr;
    GSockAddr  *m_pServerAddr;
    AdnsReq    *m_pAdnsReq;

    HFProcessFn m_pfProcessor;
    void       *m_pProcessorArg;

    short       m_iSsl;
    short       m_iVerifyCert;
    SslConnection  m_ssl;
    StrStrHashMap  m_respHeaders;

    char        m_achHost[256];
    AutoBuf     m_resHeaderBuf;

    HttpFetchDriver *m_pHttpFetchDriver;
    time_t      m_tmStart;
    int         m_iTimeoutSec;
    int         m_iReqInited;
    Logger     *m_pLogger;
    int         m_iLoggerId;
    int         m_iEnableDebug;


    int endReq(int res);
    int getLine(char *&p, char *pEnd,
                char *&pLineBegin, char *&pLineEnd);
    int allocateBuf(const char *pSaveFile);
    int pollEvent(int evt, int timeoutSecs);
    int buildReq(const char *pMethod, const char *pURL,
                 const char *pContentType = NULL);
    int startProcessReq(const GSockAddr &sockAddr);
    int startProcess();
    static int asyncDnsLookupCb(void *arg, const long lParam, void *pParam);
    int startDnsLookup(const char *addrServer);

    int connectSSL();

    int recvSSL();
    void setSSLAgain();

    int sendReq();
    int recvResp();
    void startDriver();
    void stopDriver();
    int initReq(const char *pURL, const char *pBody, int bodyLen,
                const char *pSaveFile, const char *pContentType);
    int getLoggerId()     {   return m_iLoggerId;    }

    int verifyDomain();
    void addRespHeader(char *pLineBegin, char *pLineEnd);

public:
    enum
    {
        ERROR_DNS_FAILURE  = -20,
        ERROR_CONN_FAILURE,
        ERROR_CONN_TIMEOUT,
        ERROR_SOCKET_ERROR,
        ERROR_SSL_HANDSHAKE,
        ERROR_SSL_CERT_VERIFY,
        ERROR_SSL_UNMATCH_DOMAIN,
        ERROR_HTTP_SEND_REQ_HEADER_FAILURE,
        ERROR_HTTP_SEND_REQ_BODY_FAILURE,
        ERROR_HTTP_PROTOCOL_ERROR,
        ERROR_HTTP_RECV_RESP_FAILURE,
        ERROR_CANCELED,
    };
    HttpFetch();
    ~HttpFetch();
    void setResProcessor(HFProcessFn cb, void *pArg)
    {   m_pfProcessor = cb; m_pProcessorArg = pArg;  }
    int getHttpFd() const                   {   return m_iFdHttp;    }


    int startReq(const char *pURL, int nonblock, int enableDriver = 1,
                 const char *pBody = NULL,
                 int bodyLen = 0, const char *pSaveFile = NULL,
                 const char *pContentType = NULL, const char *addrServer = NULL);
    int startReq(const char *pURL, int nonblock, int enableDriver,
                 const char *pBody, int bodyLen, const char *pSaveFile,
                 const char *pContentType, const GSockAddr &sockAddr);
    short getPollEvent() const;
    int processEvents(short revent);
    int process();
    int cancel();
    int getStatusCode() const               {   return m_iStatusCode;    }
    const char *getRespContentType() const  {   return m_pRespContentType;  }
    VMemBuf *getResult() const              {   return m_pBuf;          }
    void releaseResult();
    void reset();
    void setProxyServerAddr(const char *pAddr);
    const char *getProxyServerAddr() const
    {   return m_psProxyServerAddr;  }

    void closeConnection();
    void setTimeout(int timeoutSec)         {   m_iTimeoutSec = timeoutSec; }
    int getTimeout()                        {   return m_iTimeoutSec;   }
    void writeLog(const char *s)
    {   LS_INFO(m_pLogger, "HttpFetch[%d]: %s", getLoggerId(), s);    }
    void enableDebug(int d)                 {   m_iEnableDebug = d;     }
    time_t getTimeStart() const             {   return m_tmStart;       }

    void setUseSsl(int s)                   {   m_iSsl = s;             }
    int isUseSsl() const                    {   return m_iSsl;          }

    void setVerifyCert(int s)               {   m_iVerifyCert = s;      }

    int setExtraHeaders(const char *pHdrs, int len);
    const char *getRespHeader(const char *pName) const;
    X509 *getSslCert() const;


    LS_NO_COPY_ASSIGN(HttpFetch);
};


#endif
