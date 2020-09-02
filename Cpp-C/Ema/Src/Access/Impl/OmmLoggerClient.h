/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmLoggerClient_h
#define __thomsonreuters_ema_access_OmmLoggerClient_h

#include "EmaString.h"
#include "Mutex.h"
#include <stdio.h>

namespace rtsdk {

namespace ema {

namespace access {

static EmaString CR( "\n\t" );

struct LoggerFile {
	int			clientCount;
	FILE*		ptr;
	EmaString	*fileName;
};

struct LoggerClientFiles {
	int					fileCount;
	int					openFilesSize;
	struct LoggerFile*	openFiles;
};

class OmmLoggerClient
{
public :

	enum Severity {
		VerboseEnum = 0,	// output all log messages including information
		SuccessEnum,		// indicates successful completion of a task / step
		WarningEnum,		// indicates completion of a task / step; may require attention and correction
		ErrorEnum,			// indicates failure while performing task / step; requires correction to proceed
		NoLogMsgEnum		// do not print any log messages
	};

	enum LoggerType {
		FileEnum = 0,
		StdoutEnum
	};

	static OmmLoggerClient* create( LoggerType loggerType, bool includeDate, Severity severity, const EmaString& fileName );

	static void destroy( OmmLoggerClient*& );

	void log( const EmaString& instanceName, Severity , const EmaString& text );

	void log( const EmaString& instanceName, Severity , const char* text );

	static const char* loggerSeverityString( Severity );

	static char* timeString( bool includeDate = false );

private :

	OmmLoggerClient( LoggerType loggerType, bool includeDate, Severity severity, const EmaString& fileName );

	virtual ~OmmLoggerClient();

	void openLogFile( const EmaString& );

	void closeLogFile();

	static Mutex			_printLock;

	FILE*					_pFile;

	FILE*					_pOutput;

	EmaString				_logLine;

	bool					_includeDateInLoggerOutput;

	Severity				_severity;

	OmmLoggerClient();
	OmmLoggerClient( const OmmLoggerClient& );
	OmmLoggerClient& operator=( const OmmLoggerClient& );

	static LoggerClientFiles clientFiles;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmLoggerClient_h
