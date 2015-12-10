/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmLoggerClient.h"
#include "OmmConsumerImpl.h"
#include "ExceptionTranslator.h"
#include "OmmException.h"
#include "OmmConsumerActiveConfig.h"
#include "time.h"
#include "Utilities.h"
#include <new>

#define LOGFILEEXT	".log"

using namespace thomsonreuters::ema::access;

Mutex OmmLoggerClient::_printLock;
struct LoggerClientFiles OmmLoggerClient::clientFiles = { 0, 0, 0 };

OmmLoggerClient::OmmLoggerClient( ActiveConfig& activeConfig ) :
_pFile(0),
 _activeConfig( activeConfig ),
 _pOutput( 0 ),
 _logLine( 0, 2048 )
{
	if ( _activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum )
		openLogFile();
	else
		_pOutput = stdout;

	_includeDateInLoggerOutput = _activeConfig.loggerConfig.includeDateInLoggerOutput;
}

OmmLoggerClient::~OmmLoggerClient()
{
	closeLogFile();
}

OmmLoggerClient* OmmLoggerClient::create( ActiveConfig& _activeConfig )
{
	OmmLoggerClient* pClient = 0;

	try {
		pClient = new OmmLoggerClient( _activeConfig );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
	{
		const char* temp = "Failed to create OmmLoggerClient.";
		throwMeeException( temp );
	}

	return pClient;
}

void OmmLoggerClient::destroy( OmmLoggerClient*& pClient )
{
	if ( pClient )
	{
		delete pClient;
		pClient = 0;
	}
}

void OmmLoggerClient::openLogFile()
{
	_printLock.lock();

    if ( _pFile )
        _pOutput = _pFile;
	else
	{
	    EmaString fileName( _activeConfig.loggerConfig.loggerFileName );
	    fileName.append( "_" );

#ifdef WIN32
		fileName += (UInt64)GetCurrentProcessId();
#else
		fileName += (UInt64)getpid();
#endif
	    fileName += LOGFILEEXT;

        for ( int i = 0; i < clientFiles.fileCount; ++i )
        {
			if (clientFiles.openFiles[i].fileName == fileName)
			{
				_pFile = _pOutput = clientFiles.openFiles[i].ptr;
				++clientFiles.openFiles[i].clientCount;
                _printLock.unlock();
				return;
			}
		}

        _pOutput = fopen( fileName.c_str(), "w" );
	    if ( ! _pOutput )
	    {
			_printLock.unlock();

			EmaString temp( "Failed to open " );
			temp.append( fileName ).append( " file for writing log messages." );

			throwLfiException( fileName, temp );
			return;
		}

		_pFile = _pOutput;
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString text("opened ");
			text.append(fileName).append(" at ").append(timeString(true));
			log( "OmmLoggerClient", OmmLoggerClient::VerboseEnum, text);
		}

        int record(-1);
        for ( int i = 0; i < clientFiles.fileCount; ++i )
            if ( ! clientFiles.openFiles[i].clientCount )
			{
                record = i;
                break;
			}

        if ( record == -1 )
		{
			if (clientFiles.openFilesSize)
				clientFiles.openFilesSize *= 2;
			else
				clientFiles.openFilesSize = 4;
			clientFiles.openFiles = static_cast<struct LoggerFile *>( realloc(static_cast<void *>( clientFiles.openFiles ), sizeof(struct LoggerFile) * clientFiles.openFilesSize ));
			for (int i = clientFiles.fileCount; i < clientFiles.openFilesSize; ++i)
				memset(static_cast<void *>(&clientFiles.openFiles[i]), 0, sizeof clientFiles.openFiles[i]);
			record = clientFiles.fileCount;
		}
		clientFiles.openFiles[record].clientCount = 1;
		clientFiles.openFiles[record].ptr = _pOutput;
		clientFiles.openFiles[record].fileName = fileName;
		++clientFiles.fileCount;
	}

	_printLock.unlock();
}

void OmmLoggerClient::closeLogFile()
{
	_printLock.lock();

	if ( _pOutput && _pOutput != stdout )
	{
		for ( int i = 0; i < clientFiles.fileCount; ++i )
			if ( clientFiles.openFiles[i].ptr == _pOutput )
			{
				if ( ! --clientFiles.openFiles[i].clientCount )
				{
					if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
					{
						EmaString text("closed ");
						text.append(clientFiles.openFiles[i].fileName).append(" at ").append(timeString(true));
						log( "OmmLoggerClient", OmmLoggerClient::VerboseEnum, text );
					}
					fclose( clientFiles.openFiles[i].ptr );
					clientFiles.openFiles[i].ptr = 0;
                    --clientFiles.fileCount;
				}

				_pFile = _pOutput = 0;				
				break;
			}
	}

	_printLock.unlock();
}

char *
OmmLoggerClient::timeString(bool includeDate)
{
	static char timeString[32];
#ifdef WIN32
	SYSTEMTIME time;
	GetLocalTime( &time );
	if ( includeDate )
		snprintf( timeString, sizeof timeString, "%04d/%02d/%02d %02d:%02d:%02d.%03d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	else
		snprintf( timeString, sizeof timeString, "%02d:%02d:%02d.%03d", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
#else
	struct timespec tp;
	long result = clock_gettime( CLOCK_REALTIME, &tp );
	int next;
	if ( includeDate )
		next = strftime( timeString, sizeof timeString, "%Y/%m/%d %H:%M:%S", localtime(&tp.tv_sec));
	else
		next = strftime( timeString, sizeof timeString, "%H:%M:%S", localtime(&tp.tv_sec));
	 snprintf(timeString + next, sizeof timeString - next, ".%03d", tp.tv_nsec/static_cast<long>(1E6));
#endif
	return timeString;
}

void OmmLoggerClient::log( const EmaString& callbackClientName, Severity severity , const EmaString& text )
{
	log( callbackClientName, severity, text.c_str() );
}

void OmmLoggerClient::log( const EmaString& callbackClientName, Severity severity , const char* text )
{
	EMA_ASSERT( severity >= _activeConfig.loggerConfig.minLoggerSeverity,
		"OmmLoggerClient::log should not be called with severity less than minLoggerSeverity");

	_printLock.lock();

	_logLine.set( "loggerMsg\n" )
		.append( "    TimeStamp: " ).append( timeString( _activeConfig.loggerConfig.includeDateInLoggerOutput ) ).append( "\n" )
		.append( "    ClientName: " ).append( callbackClientName ).append( "\n" )
		.append( "    Severity: " ).append( loggerSeverityString( severity ) ).append( "\n" )
		.append( "    Text:    " ).append( text ).append( "\n" )
		.append( "loggerMsgEnd\n\n" );

	if ( _pOutput ) {
		fprintf( _pOutput, "%s", _logLine.c_str() );
		fflush( _pOutput );
	}

	_printLock.unlock();
}

const char* OmmLoggerClient::loggerSeverityString( Severity severity )
{
	switch ( severity )
	{
	case VerboseEnum :
		return "Verbose";
	case SuccessEnum :
		return "Success";
	case WarningEnum :
		return "Warning";
	case ErrorEnum :
		return "Error";
	case NoLogMsgEnum :
		return "NoLogMsg";
	default :
		return "Unknown Severity";
	}
}
