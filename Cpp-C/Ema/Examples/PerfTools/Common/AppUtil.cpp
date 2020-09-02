///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "AppUtil.h"
#include "CtrlBreakHandler.h"
#include "Mutex.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/syscall.h>
#endif

#if defined(WIN32)
#include <Winsock2.h>
#include <WS2tcpip.h>
#endif

using namespace perftool::common;
using namespace rtsdk::ema::access;

static time_t s_time;

EmaString      AppUtil::_appName = EmaString("UNINITIALIZED_APP_NAME");
EmaString      AppUtil::_sysTimeStr;
char           AppUtil::_sysTimeBuf[_SIZE];
struct tm*     AppUtil::_pTm = new tm();
time_t*        AppUtil::_pTime = &s_time;
UInt32         AppUtil::_pid = 0;
UInt32         AppUtil::_tid = 0;
FILE*          AppUtil::_OUT = stdout;
FILE*          AppUtil::_LOG = 0;
FILE*          AppUtil::_SUM = 0;
Mutex       AppUtil::_logMutex;
bool           AppUtil::_exitError = false;

void AppUtil::init(const EmaString& appName, FILE* LOG)
{
	AppUtil::_appName = appName;
	AppUtil::_LOG = LOG;

#if defined(WIN32)
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		
		EmaString text("AppUtil::int() failed to call WSAStartup with error:");
		text.append(err);
		text.append("\n");
		AppUtil::logError(text);
	}
#endif
}

bool AppUtil::abort(const char* reason, ...)
{
	_logMutex.lock();
	setExitError(true);
	va_list _vargs;
	printf("\nError! ");
	va_start(_vargs, reason);
	vprintf(reason, _vargs);
	va_end(_vargs);
	printf("\n\n");
	fflush(stdout);
	if(_LOG)
	{
		fprintf(_LOG, "\nError! ");
		va_start(_vargs, reason);
		vfprintf(_LOG, reason, _vargs);
		va_end(_vargs);
		fprintf(_LOG, "\n\n");
		fflush(_LOG);
	}
	if(_SUM)
	{
		fprintf(_SUM, "\nError! ");
		va_start(_vargs, reason);
		vfprintf(_SUM, reason, _vargs);
		va_end(_vargs);
		fprintf(_SUM, "\n\n");
		fflush(_SUM);
	}
	_logMutex.unlock();
	CtrlBreakHandler::forceExit();
	return false;
}

void AppUtil::log(const char* reason, ...)
{
	_logMutex.lock();
	va_list _vargs;
	va_start(_vargs, reason);
	vprintf(reason, _vargs);
	va_end(_vargs);
	fflush(stdout);
	if(_LOG)
	{
		va_start(_vargs, reason);
		vfprintf(_LOG, reason, _vargs);
		va_end(_vargs);
		fflush(_LOG);
	}
	_logMutex.unlock();
}
void AppUtil::logError(const rtsdk::ema::access::EmaString text)
{
	_logMutex.lock();
	fprintf(stdout, "%s\n", text.c_str());
	_logMutex.unlock();
}
const EmaString& AppUtil::getSysTimeStr()
{
	time(_pTime);
#if defined (WIN32) || defined (_WIN32)
	gmtime_s(_pTm, _pTime);
#else
	gmtime_r(_pTime, _pTm);
#endif
	_sysTimeStr.clear();
	SNPRINTF(_sysTimeBuf, _SIZE, "%4d-%02d-%02d %02d:%02d:%02d", _pTm->tm_year+1900, _pTm->tm_mon+1, _pTm->tm_mday, _pTm->tm_hour, _pTm->tm_min, _pTm->tm_sec);
	_sysTimeStr = _sysTimeBuf;
	return _sysTimeStr;
}

UInt32 AppUtil::getProcessId()
{
	if(!_pid)
{
#if defined(WIN32)
	_pid = static_cast<unsigned long>(GetCurrentProcessId());
#else
	_pid = static_cast<unsigned long>(getpid());
#endif
	}
	return _pid;
}

UInt32 AppUtil::getThreadId()
{
#if defined (WIN32)
	_tid = GetCurrentThreadId();
#elif defined(Linux)
	_tid = syscall( SYS_gettid );
#endif
	return _tid;
}

void AppUtil::sleep(UInt64 millisecs)
{
#if defined (_WIN32)
    ::Sleep( (DWORD)(millisecs) );
#else
    struct timespec sleeptime;
    sleeptime.tv_sec = millisecs / 1000;
    sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
    nanosleep(&sleeptime,0);
#endif
}

Int32 AppUtil::getHostAddress(UInt32* address)
{
	char				hostName[256];
#if defined(Linux)
	struct hostent*		hep;
	struct hostent		hostEntry;
	int					h_errnop;
	char*				hostBuff = (char*)malloc(sizeof(hostName));
	int					buflen = sizeof(hostName);
#endif

	Int32 retValue = gethostname(hostName, (int)sizeof(hostName));

	if(gethostname(hostName, (int)sizeof(hostName)))
	{
		return -1;
	}

#if defined(Linux) 

	while(gethostbyname_r(hostName, &hostEntry, hostBuff, buflen, &hep, &h_errnop))
	{
		if (h_errnop == NETDB_INTERNAL && errno == ERANGE)
		{
			buflen *= 2;
			hostBuff = (char*)realloc(hostBuff, buflen);
		}
		else
			break;
	}
	if (hep)
	{
		*address = *((UInt32*)(hep->h_addr));
		*address = ntohl(*address);

		free(hostBuff);
		hostBuff = NULL;
		return(0);
	}

	free(hostBuff);
	hostBuff = NULL;
#else
	struct addrinfo hints,*res;
	struct sockaddr_in *addr;
	memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_DGRAM;
	DWORD dwRetval = getaddrinfo(hostName, NULL, &hints, &res );

	if( dwRetval != 0 )
		return (-1);	
 
	if( res != NULL )
	{
		addr = (struct sockaddr_in *)res->ai_addr; 
		*address = ntohl(addr->sin_addr.s_addr);
		freeaddrinfo(res);
		return(0);
	}
#endif
	return(-1);
}

void AppUtil::formatNameValue(EmaString& str, const EmaString& name, Int32 max, const EmaString& value)
{
	char tmp[256];
	str += "\n   ";
	str += name;
	SNPRINTF(tmp, sizeof(tmp), "%*c: ", max - name.length(), ' ');
	str += tmp;
	str += value;
}

void AppUtil::formatNameValue(EmaString& str, const EmaString& name, Int32 max, Int32 value)
{
	char tmp[256];
	str += "\n   ";
	str += name;
	SNPRINTF(tmp, sizeof(tmp), "%*c: ", max - name.length(), ' ');
	str += tmp;
	str.append( value );
}

void AppUtil::formatNameValue(EmaString& str, const EmaString& name, Int32 max, Int64 value)
{
	char tmp[256];
	str += "\n   ";
	str += name;
	SNPRINTF(tmp, sizeof(tmp), "%*c: ", max - name.length(), ' ');
	str += tmp;
	str.append( value );
}

void AppUtil::formatNameValue(EmaString& str, const EmaString& name, Int32 max, bool value)
{
	char tmp[256];
	str += "\n   ";
	str += name;
	SNPRINTF(tmp, sizeof(tmp), "%*c: ", max - name.length(), ' ');
	str += tmp;
	value ? str += "true" : str += "false";
}

void AppUtil::printCurrentTimeUTC(FILE *file)
{
	time_t statsTime;
	struct tm *pStatsTimeTm;

	time(&statsTime);
	pStatsTimeTm = gmtime(&statsTime);

	fprintf(file, "%d-%02d-%02d %02d:%02d:%02d",
			pStatsTimeTm->tm_year + 1900, pStatsTimeTm->tm_mon + 1, pStatsTimeTm->tm_mday,
			pStatsTimeTm->tm_hour, pStatsTimeTm->tm_min, pStatsTimeTm->tm_sec);
}
