/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_Utilities_h
#define __refinitiv_ema_access_Utilities_h

#include <stdlib.h>
#ifdef WIN32
#include <Windows.h>
#endif

#if defined (_WIN32) || defined(WIN32)
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

#include "DataType.h"
#include "rtr/rsslState.h"
#include "rtr/rsslErrorInfo.h"

namespace refinitiv {
	namespace ema {
		namespace access {
			class EmaString;
			class EmaBuffer;
		}
	}
}

extern const refinitiv::ema::access::DataType::DataTypeEnum msgDataType[];

bool getCurrentDir( refinitiv::ema::access::EmaString& );

void stateToString( RsslState* , refinitiv::ema::access::EmaString& );

void hexToChar( char* out, char in );

void hexToString( refinitiv::ema::access::EmaString& output, const refinitiv::ema::access::EmaBuffer& in );

const char* ptrToStringAsHex( void* );

const char* timeString();

int emaGetUserName(refinitiv::ema::access::EmaString& string);

int emaGetPosition(refinitiv::ema::access::EmaString& string);

refinitiv::ema::access::EmaString& addIndent( refinitiv::ema::access::EmaString& temp, refinitiv::ema::access::UInt64 indent, bool addLine = false );

void clearRsslErrorInfo( RsslErrorInfo* pRsslErrorInfo );

#ifdef WIN32
static __forceinline
#else
static inline
#endif

void emafail( const char* cond, const char* msg, const char* file, int line )
{
	static char fmt[] = "Fatal Error: %s, %s (file %s, line %d)\n";
	fprintf(stderr, fmt, cond, msg, file, line);
	abort();
}

#define EMAFAIL(cond, msg) emafail(cond, msg, __FILE__, __LINE__)

#if defined(NDEBUG)
	#define EMA_ASSERT(COND, MSG)
#else
	#if defined(__STDC__) || defined(_MSC_VER)
		#define EMA_ASSERT(COND, MSG) (void)((COND) || (EMAFAIL(#COND, #MSG), 0))
	#else
		#define EMA_ASSERT(COND, MSG) (void)((COND) || (EMAFAIL("COND", "MSG"), 0))
	#endif
#endif

#ifdef WIN32

int	emaExceptionFilter( LPEXCEPTION_POINTERS pExcetionPointers, const char *sourceFile, 
						unsigned int line, char *reportBuffer, unsigned int reportBufferLen );

bool emaStackTraceInitialize() ;

void emaGenerateStackTrace( void *pContext, char *reportBuffer, int reportBufferLen );

const char *getExceptionCodeString( DWORD exceptionCode );

#endif 

int emaProblemReport( void* pContext, const char* sourceFile, unsigned int line, 
					 const char* message, char* reportBuffer, unsigned int reportBufferLen );


#endif // __refinitiv_ema_access_Utilities_h
