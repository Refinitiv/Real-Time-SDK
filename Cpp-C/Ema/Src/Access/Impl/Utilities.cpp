/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <time.h>

#ifdef WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#include "EmaBuffer.h"
#include "EmaString.h"
#include "Mutex.h"
#include "Utilities.h"

#define MAX_LENGTH 2056

#ifdef WIN32
#include <direct.h>
#else
#define getCWD getcwd
#endif

#ifdef WIN32
#include "Dbghelp.h"
	#pragma comment(lib, "Dbghelp.lib")		
	#pragma comment(lib, "DELAYIMP.LIB")
#define	EMA_ARRAY_LEN( X )	(sizeof( X )/sizeof(*( X )))
#define EMA_STR_BUFF_SIZE 512

static bool	_DbgHelpIntialized = false;
#endif


using namespace thomsonreuters::ema::access;

#ifdef WIN32
bool getCurrentDir( thomsonreuters::ema::access::EmaString& dir )
{
	DWORD length = MAX_LENGTH;
	char* temp = new char[length];
	DWORD dwRet = GetCurrentDirectory( length, temp );

	if ( dwRet > length )
	{
		length = dwRet + 1;

		delete [] temp;
		temp = new char[length];

		dwRet = GetCurrentDirectory( length, temp );
	}

	if ( dwRet == 0 )
	{
		// todo ... GetLastError()
		return false;
	}
	else
	{
		dir.set( temp );

		delete [] temp;
		return true;
	}
}
#else
bool getCurrentDir( thomsonreuters::ema::access::EmaString& dir )
{
	size_t length = MAX_LENGTH;
	char* temp = new char[length];
	char* retTemp = getcwd( temp, length );

	while ( !retTemp )
	{
		if ( errno != ERANGE ) break;

		length += length;
		delete [] temp;
		temp = new char[length];

		retTemp = getcwd( temp, length );
	}

	if ( retTemp )
	{
		dir.set( retTemp );

		delete [] temp;
		return true;
	}
	else
	{
		delete [] temp;
		return false;
	}
}
#endif

void stateToString( RsslState* pState, thomsonreuters::ema::access::EmaString& stateString )
{
	if ( !pState )
	{
		stateString.clear().set( "'(not set)'" );
		return;
	}

	stateString.clear().append( rsslStreamStateInfo( pState->streamState ) )
		.append( " / " )
		.append( rsslDataStateInfo( pState->dataState ) )
		.append( " / " )
		.append( rsslStateCodeInfo( pState->code ) )
		.append( " / '" );

	EmaString text( pState->text.data, pState->text.length );

	stateString.append( text ).append( "'" );
}

void hexToChar( char* out, char in )
{
	char temp = in;

	temp >>= 4;
	temp &= 0xF;

	if ( temp <= 0x9 )
		*out = 0x30 + temp;
	else
		*out = 0x37 + temp;

	temp = in;
	temp &= 0xF;

	if ( temp <= 0x9 )
		*(out + 1) = 0x30 + temp;
	else
		*(out +1) = 0x37 + temp;

	*(out + 2) = 0x00;
}

void hexToString( EmaString& output, const EmaBuffer& in )
{
	for ( UInt32 idx = 0; idx < in.length(); ++idx )
	{
		char charOmmArray[10];

		hexToChar( charOmmArray, *(in.c_buf() + idx) );

		output.append( charOmmArray );
		
		if ( in.length() > 1 && idx < in.length() - 1 ) output.append( " " );
	}
}

const char* ptrToStringAsHex( void* ptr )
{
    static char tmp[ 32 ];
    tmp[ 0 ] = 0;
    snprintf( tmp, 32, "0x%p", ptr );
    return tmp;
}

EmaString& addIndent( EmaString& temp, UInt64 indent, bool addLine )
{
	if ( addLine )
		temp.append( "\n" );

	while ( indent-- > 0 )
		temp.append( "    " );

	return temp;
}


#ifdef WIN32

const char* getExceptionCodeString( DWORD exceptionCode )
{
	switch ( exceptionCode )
	{
	case EXCEPTION_ACCESS_VIOLATION:
		return "Access Violation";

	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return "Datatype Misalignment";
   
	case EXCEPTION_BREAKPOINT:
		return "Breakpoint";
              
	case EXCEPTION_SINGLE_STEP:
		return "Single Step";
        
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return "Array Bounds Exceeded";
  
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return "Floating Point Denormal Operand";

	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return "Floating Point Division By Zero";
 
	case EXCEPTION_FLT_INEXACT_RESULT:
		return "Floating Point Inexact Result";
    
	case EXCEPTION_FLT_INVALID_OPERATION:
		return "Floating Point Invalid Operation";

	case EXCEPTION_FLT_OVERFLOW:
		return "Floating Point Overflow";

	case EXCEPTION_FLT_STACK_CHECK:
		return "Stack Corrupted as a result of a Floating Point operation";

	case EXCEPTION_FLT_UNDERFLOW:
		return "Floating Point Underflow";

	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		return "Integer Division by Zero";

	case EXCEPTION_INT_OVERFLOW:
		return "Integer Overflow";

	case EXCEPTION_PRIV_INSTRUCTION:
		return "Attempt to execute a priviliged instruction";

	case EXCEPTION_IN_PAGE_ERROR:
		return "Cannot load memory page";

	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return "Attempt to execute an invalid instruction";

	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		return "Execution cannot be continued ";

	case EXCEPTION_STACK_OVERFLOW:
		return "Stack overflow";

	case EXCEPTION_GUARD_PAGE:
		return "Guard page";

	case EXCEPTION_INVALID_HANDLE:
		return "Invalid handle";

	case 0xe06d7363:
		return "C++ exception";

	default:
		return "";
	}
}

int emaExceptionFilter( LPEXCEPTION_POINTERS pExcetionPointers,
						const char* sourceFile, 
						unsigned int line, 
						char* reportBuffer,
						unsigned int reportBufferLen )
{
	EXCEPTION_RECORD* pEXCEPTION_RECORD = pExcetionPointers->ExceptionRecord;

	switch( pEXCEPTION_RECORD->ExceptionCode )
	{
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_UNDERFLOW:
		return EXCEPTION_CONTINUE_EXECUTION;
	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_SINGLE_STEP:
		if ( IsDebuggerPresent() )
			return EXCEPTION_CONTINUE_SEARCH;
	}

	char message[EMA_STR_BUFF_SIZE];
	int  msgLen = snprintf( message, sizeof(message),"Exception \"%s\" (0x%lX) occured at 0x%p", 
						   getExceptionCodeString(pEXCEPTION_RECORD->ExceptionCode),
						   pEXCEPTION_RECORD->ExceptionCode,
						   pEXCEPTION_RECORD->ExceptionAddress );


	if ( pEXCEPTION_RECORD->ExceptionCode == EXCEPTION_ACCESS_VIOLATION  && 
		pEXCEPTION_RECORD->NumberParameters )
		snprintf( message + msgLen, sizeof(message) - msgLen,": memory at 0x%p cannot be %s",
					(void*)pEXCEPTION_RECORD->ExceptionInformation[1],
					pEXCEPTION_RECORD->ExceptionInformation[0] ? "read" : "written" );


	return emaProblemReport( pExcetionPointers->ContextRecord,
							sourceFile,
							line, 
							message,
							reportBuffer,
							reportBufferLen );
}

static Mutex _DbgHelpMutex;

bool StackTraceInitialize() 
{
	if ( !_DbgHelpIntialized )
	{
		_DbgHelpMutex.lock();
		if ( !_DbgHelpIntialized ) 
		{
			char moduleSearchPath[EMA_STR_BUFF_SIZE];
			*moduleSearchPath = 0;
			GetSystemDirectoryA( moduleSearchPath, EMA_STR_BUFF_SIZE-1 );
			int len = static_cast< int >( strlen( moduleSearchPath ) );
			if( len )
				moduleSearchPath[len++] = ';';

			if( GetModuleFileNameA( NULL, moduleSearchPath + len, EMA_STR_BUFF_SIZE-1 - len ) )
			{
				char *tStr = strrchr( moduleSearchPath, '\\' );
				if( tStr ) *tStr = 0;
			}

			__try
			{
				SymSetOptions( SymGetOptions() | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS );

				_DbgHelpIntialized = SymInitialize( GetCurrentProcess(),  *moduleSearchPath ? moduleSearchPath : NULL, TRUE ) != FALSE;
			}
			__except( EXCEPTION_EXECUTE_HANDLER )
			{
			}
		}

		_DbgHelpMutex.unlock();
	}

	return _DbgHelpIntialized;
}

void  GenerateStackTrace( void* pContext, char* reportBuffer, int reportBufferLen )
{
	if ( !StackTraceInitialize() )
		return;

	char tmpBuffer[EMA_STR_BUFF_SIZE];
	HANDLE hCurrThread = GetCurrentThread();
	HANDLE hCurrProcess = GetCurrentProcess();

	CONTEXT contextRecord;

	bool retVal = true;

	if ( !pContext )
	{
		ZeroMemory( &contextRecord, sizeof( contextRecord ) );
		contextRecord.ContextFlags = CONTEXT_FULL;
	
		::RtlCaptureContext(&contextRecord);
	}
	else
		CopyMemory( &contextRecord, pContext, sizeof( contextRecord ) );

	if ( retVal )
	{
		STACKFRAME64 stackFrame;
		ZeroMemory( &stackFrame, sizeof( stackFrame ) );
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrStack.Mode   = AddrModeFlat;
		stackFrame.AddrFrame.Mode   = AddrModeFlat;

		unsigned long imageType = IMAGE_FILE_MACHINE_I386; 

#ifdef _M_IX86
		imageType = IMAGE_FILE_MACHINE_I386;
		stackFrame.AddrPC.Offset = contextRecord.Eip;
		stackFrame.AddrFrame.Offset = contextRecord.Ebp;
		stackFrame.AddrStack.Offset = contextRecord.Esp;
#elif _M_X64  
		imageType = IMAGE_FILE_MACHINE_AMD64;
		stackFrame.AddrPC.Offset = contextRecord.Rip;
		stackFrame.AddrFrame.Offset = contextRecord.Rsp;
		stackFrame.AddrStack.Offset = contextRecord.Rsp;
#else //_M_IA64
		imageType = IMAGE_FILE_MACHINE_IA64;
		stackFrame.AddrPC.Offset = contextRecord.StIIP;
		stackFrame.AddrFrame.Offset = contextRecord.IntSp;
		stackFrame.AddrBStore.Offset = contextRecord.RsBSP;
		stackFrame.AddrStack.Offset = contextRecord.IntSp;
		stackFrame.AddrBStore.Mode = AddrModeFlat;
#endif

		strcat( reportBuffer, "\n\nStack Trace:" );
		DWORD64 addresses[16];

		int addrCount = 0;
		int i;

		for ( i = 0; addrCount < EMA_ARRAY_LEN(addresses); i++ )
		{
			if ( FALSE == ::StackWalk64( imageType, 
								  hCurrProcess, 
								  hCurrThread, 
								  &stackFrame, 
								  &contextRecord,
								  NULL, 
								  ::SymFunctionTableAccess64, 
								  ::SymGetModuleBase64 , 
								  NULL ) )
			{
					break;
			}
			
			 if (stackFrame.AddrPC.Offset == 0)
			  {
				printf("Stack walk complete!\n");
				break;
			  }

			addresses[addrCount++] = stackFrame.AddrPC.Offset;
		}

		for ( i = 0; i < addrCount; i++ )
		{
			 if (  addresses[i] == 0 )
				 continue;

			IMAGEHLP_MODULE64 moduleInfo;
			ZeroMemory( &moduleInfo, sizeof(moduleInfo) );
			moduleInfo.SizeOfStruct = sizeof( IMAGEHLP_MODULE );

			char 	symbolInfoBuff[sizeof( IMAGEHLP_SYMBOL ) + MAX_PATH];
			ZeroMemory( symbolInfoBuff, sizeof( symbolInfoBuff ) );
			PIMAGEHLP_SYMBOL pSymbolInfo = reinterpret_cast<PIMAGEHLP_SYMBOL>(symbolInfoBuff);
			pSymbolInfo->SizeOfStruct = sizeof( *pSymbolInfo );
			pSymbolInfo->Address =  addresses[i] ;
			pSymbolInfo->MaxNameLength = MAX_PATH ;
#ifdef  _WIN64
			DWORD64 dwSymDispl = 0;
#else
			DWORD dwSymDispl = 0;
#endif

			IMAGEHLP_LINE		lineInfo;
			ZeroMemory( &lineInfo, sizeof( lineInfo ) );
			lineInfo.SizeOfStruct = sizeof( lineInfo );
			DWORD dwLineDispl = 0;


			BOOL bModInfoRetVal = SymGetModuleInfo64( hCurrProcess, addresses[i] , &moduleInfo );
			BOOL bSymInfoRetVal = SymGetSymFromAddr( hCurrProcess, addresses[i], &dwSymDispl, pSymbolInfo );
			BOOL bLineInfoRetVal = SymGetLineFromAddr( hCurrProcess, addresses[i], &dwLineDispl, &lineInfo );
			
			int tmpBufferPos = snprintf( tmpBuffer, sizeof(tmpBuffer),"\n0x%p ", (void*)addresses[i] );
			
			tmpBufferPos += snprintf( tmpBuffer + tmpBufferPos,sizeof(tmpBuffer)-tmpBufferPos, " %s", 
					bModInfoRetVal != FALSE ?
					moduleInfo.ImageName :
					"<unknown module>" );
		
			if ( bSymInfoRetVal != FALSE )
			{
				tmpBufferPos += snprintf( tmpBuffer + tmpBufferPos,sizeof(tmpBuffer)-tmpBufferPos, ": %s()", pSymbolInfo->Name );
				if( dwSymDispl )
					tmpBufferPos += snprintf( tmpBuffer + tmpBufferPos, sizeof(tmpBuffer)-tmpBufferPos," + %lld bytes", dwSymDispl );
			}

			if ( bLineInfoRetVal != FALSE &&  lineInfo.LineNumber &&  lineInfo.FileName )
				tmpBufferPos += snprintf( tmpBuffer + tmpBufferPos, sizeof(tmpBuffer)-tmpBufferPos, "\n           %s, line %ld", lineInfo.FileName, lineInfo.LineNumber );

			int bufLen = static_cast< int > ( strlen( reportBuffer ) );

			if ( bufLen + tmpBufferPos + EMA_STR_BUFF_SIZE >= reportBufferLen )
				break;

			strcpy( reportBuffer + bufLen, tmpBuffer );
		}

		if ( addrCount == 0  )
			strcat( reportBuffer, " <unavailable>" );
	}
}

#endif

int emaProblemReport( void* pContext, const char* sourceFile, unsigned int line, const char *message, char *reportBuffer, unsigned int reportBufferLen )
{

	char lineBuff[20];

	if ( line )
		snprintf( lineBuff, sizeof( lineBuff ), "%d", line );
	else
		*lineBuff = 0;

#ifdef WIN32
	char appNameStr[MAX_PATH] = "EMA";
	wchar_t wAppName[MAX_PATH] = L"EMA";
	wchar_t* pWPosition = NULL;
	wchar_t* pWPosition2 = NULL;
	if ( GetModuleFileNameW( NULL, wAppName, MAX_PATH ) > 0 )
	{
		// Find app name after the last path delimiter
		pWPosition = wcsrchr( wAppName, L'\\' );
		if ( pWPosition )
			pWPosition++;
		else
			pWPosition = wAppName;

		pWPosition2 = wcsrchr( wAppName, L'.' );
		if ( pWPosition2 )
			*pWPosition2 = L'\0';
	}

	WideCharToMultiByte(CP_UTF8, 0, pWPosition, -1, appNameStr, MAX_PATH, NULL, NULL);
	
#else
	char* appNameStr  = (char *) "EMA Application";
#endif 

	snprintf( reportBuffer, reportBufferLen,  "%s" 
						    "\n\nApplication: %s"
							"\nProcess Id: 0x%X"
#ifdef WIN32
							"\nThread Id: 0x%X"
#endif
							"%s%s"
							"%s%s"
							"\n%s",
							"EMA Exception Handler",
							appNameStr,
#ifdef WIN32
							(unsigned long)GetCurrentProcessId(),
#else
							(unsigned long)getpid(),
#endif
#ifdef WIN32
							GetCurrentThreadId(),
#endif
							sourceFile ? "\n\nFile: " : "",
							sourceFile ? sourceFile  : "",
							line	   ? "\nLine: " : "",
							lineBuff,
							message	);

#ifdef WIN32
		GenerateStackTrace( pContext, reportBuffer, reportBufferLen );
#endif 

	return 1;
}

const char* timeString()
{
	static char timeString[ 64 ];
    struct tm * tm;
#ifdef WIN32
	struct _timeb tv;
	_ftime64_s( &tv );
    tm = _localtime64( &(tv.time) );
#else
	struct timeval tv;
	gettimeofday( &tv, 0 );
    tm = localtime( &tv.tv_sec );
#endif
	strftime( timeString, sizeof timeString, "%H:%M:%S", tm );
#ifdef WIN32
	snprintf( timeString + strlen( timeString ), sizeof ( timeString )- strlen( timeString ), ".%03d ", tv.millitm );
#else
	snprintf( timeString + strlen(timeString), sizeof ( timeString )- strlen( timeString ), ".%06d ", tv.tv_usec);
#endif

	return timeString;
}

void clearRsslErrorInfo( RsslErrorInfo* pRsslErrorInfo )
{
	if ( !pRsslErrorInfo ) return;

	pRsslErrorInfo->errorLocation[0] = 0x00;
	pRsslErrorInfo->rsslErrorInfoCode = RSSL_EIC_SUCCESS;
	pRsslErrorInfo->rsslError.channel = 0;
	pRsslErrorInfo->rsslError.rsslErrorId = 0;
	pRsslErrorInfo->rsslError.sysError = 0;
	pRsslErrorInfo->rsslError.text[0] = 0x00;
}

#define UnknownDT (DataType::DataTypeEnum)(-1)

const DataType::DataTypeEnum dataType[] = {
	UnknownDT,
	UnknownDT,
	UnknownDT,
	DataType::IntEnum,
	DataType::UIntEnum,
	DataType::FloatEnum,
	DataType::DoubleEnum,
	UnknownDT,
	DataType::RealEnum,
	DataType::DateEnum,
	DataType::TimeEnum,
	DataType::DateTimeEnum,
	DataType::QosEnum,
	DataType::StateEnum,
	DataType::EnumEnum,
	DataType::ArrayEnum,
	DataType::BufferEnum,
	DataType::AsciiEnum,
	DataType::Utf8Enum,
	DataType::RmtesEnum,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	DataType::NoDataEnum,
	UnknownDT,
	DataType::OpaqueEnum,
	DataType::XmlEnum,
	DataType::FieldListEnum,
	DataType::ElementListEnum,
	DataType::AnsiPageEnum,
	DataType::FilterListEnum,
	DataType::VectorEnum,
	DataType::MapEnum,
	DataType::SeriesEnum,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, 
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
	UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT };

const DataType::DataTypeEnum msgDataType[] = {
	UnknownDT,
	DataType::ReqMsgEnum,
	DataType::RefreshMsgEnum,
	DataType::StatusMsgEnum,
	DataType::UpdateMsgEnum,
	UnknownDT,
	DataType::AckMsgEnum,
	DataType::GenericMsgEnum,
	DataType::PostMsgEnum,
	UnknownDT,
	UnknownDT,
	UnknownDT,
	UnknownDT,
	UnknownDT,
	UnknownDT };
