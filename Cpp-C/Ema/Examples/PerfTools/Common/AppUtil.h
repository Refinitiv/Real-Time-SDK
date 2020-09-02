///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#ifndef __PERFTOOL__COMMON__APPUTIL__H__
#define __PERFTOOL__COMMON__APPUTIL__H__

// Utility class for use by applications and/or common classes.

#include "Ema.h"
#include "Mutex.h"
#include <time.h>

#include <stdio.h>
#include <stdarg.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#if defined(WIN32)
	#define SNPRINTF _snprintf
	#define VSNPRINTF _vsnprintf
#else
	#define SNPRINTF snprintf
	#define VSNPRINTF vsnprintf
#endif

namespace perftool {

namespace common {

class AppUtil
{
public:
	static void init(const rtsdk::ema::access::EmaString& appName, FILE* LOG);
	static void setSum(FILE* SUM) {_SUM = SUM;}
	static void log(const char* reason, ...);
	static void logError(const rtsdk::ema::access::EmaString text);
	static bool abort(const char* reason ...);
	static bool hadExitError() {return _exitError;}

	static const rtsdk::ema::access::EmaString& getSysTimeStr();
	static void sleep(rtsdk::ema::access::UInt64 millisecs);

	static rtsdk::ema::access::Int32 getHostAddress(rtsdk::ema::access::UInt32* address);

	static rtsdk::ema::access::UInt32 getProcessId();
	static rtsdk::ema::access::UInt32 getThreadId();

	static void formatNameValue(rtsdk::ema::access::EmaString& str, const rtsdk::ema::access::EmaString& name, rtsdk::ema::access::Int32 max, const rtsdk::ema::access::EmaString& value);
	static void formatNameValue(rtsdk::ema::access::EmaString& str, const rtsdk::ema::access::EmaString& name, rtsdk::ema::access::Int32 max, rtsdk::ema::access::Int32 value);
	static void formatNameValue(rtsdk::ema::access::EmaString& str, const rtsdk::ema::access::EmaString& name, rtsdk::ema::access::Int32 max, bool value);
	static void formatNameValue(rtsdk::ema::access::EmaString& str, const rtsdk::ema::access::EmaString& name, rtsdk::ema::access::Int32 max, rtsdk::ema::access::Int64 value);

	static void printCurrentTimeUTC(FILE *file);

private:
	static void setExitError(bool val) {_exitError = val;}

	static rtsdk::ema::access::EmaString				_appName;
	static rtsdk::ema::access::EmaString				_sysTimeStr;
	static const rtsdk::ema::access::Int32				_SIZE = 26;
	static char													_sysTimeBuf[_SIZE];
	static time_t*												_pTime;
	static tm*													_pTm;
	static rtsdk::ema::access::UInt32					_pid;
	static rtsdk::ema::access::UInt32					_tid;

	static FILE*												_OUT;
	static FILE*												_LOG;
	static FILE*												_SUM;
	static perftool::common::Mutex							_logMutex;
	static bool													_exitError;
};

} // common

} // perftool

#endif // __PERFTOOL__COMMON__APPUTIL__H__
