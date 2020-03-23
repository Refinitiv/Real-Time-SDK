/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2020 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

/*
 * Macros to help debug transport based applications running from within a console command line.  
 * To enable via CMake:
 *		1) add entries to a pre-cache file (e.g. precache.txt and run cmake -C precache.txt)
 *		      set(RCDEV_C_EXTRA_FLAGS "${RCDEV_C_EXTRA_FLAGS} -D_DEBUG_CONNECTION" CACHE STRING "") and/or
 *		      set(RCDEV_CXX_EXTRA_FLAGS "${RCDEV_CXX_EXTRA_FLAGS} -D_DEBUG_CONNECTION" CACHE STRING "") 
 *		2) Run on cmake command line
 *		      cmake -D_DEBUG_CONNECTION=1 ....
 *		3) Add as extra args within a CMakeLists.txt build for the library
 *
 *	OPTIONS:
 *		When the follow are defined, there will be stderr statements 
 *		printed, related too,
 *		
 *	#define _AUTOTEST_OUTPUT	- Enable special output for validation via terminal based applications
 *	#define _DEBUG_CONN			- ipc socket, ripc connection negotiation
 *	#define _DEBUG_READ			- ipc socket transport read related calls
 *	#define _DEBUG_WRITE		- ipc socket transport write related calls
 *	#define _DEBUG_COMPRESSION	- compression and decompression of data
 *	#define _DEBUG_REF			- misc statements for tracking the creation/deletion of SocketChannels, etc ....
 *	#define _DEBUG_BUFFER		- misc statements for tracking the creation/deletion different buffers types
 *	#define _DEBUG_PARSE_HTTP	- the parsing of HTTP data payloads, their fields and token values
 *	#define _DEBUG_WS_CONN		- ipc websocket connectiuon, and ripc over websockets connection negotiation
 *	#define _DEBUG_WS_READ		- ipc websocket transport read related calls
 *	#define _DEBUG_WS_WRITE		- ipc websocket transport write related calls
 *	#define _DEBUG_WS_FRAMES	- reading/writing websocket frame headers.  When defined, this will
 *	                              additional macros to help display all information related to each
 *								  websocket frame.  The additional macros defined:
 *								  _DEBUG_WS_MASK			
 *								  _DEBUG_WS_BUFFER_DUMP
 *								  _DEBUG_WS_FRAME_STRUCT
 *								  _DEBUG_WS_FRAME_STRUCT_FIELD
 *								  To turn off any individul websocket frame trace 
 *								  define one or all of the following:
 *								  _NO_DEBUG_WS_MASK			
 *								  _NO_DEBUG_WS_BUFFER_DUMP
 *								  _NO_DEBUG_WS_FRAME_STRUCT
 *								  _NO_DEBUG_WS_FRAME_STRUCT_FIELD
 **/

//#ifndef __RTR_DEBUG_PRINT_H
//#define __RTR_DEBUG_PRINT_H

#if defined (_DEBUG_PRINT)  || \
			(_DEBUG_INIT)  || \
			(_DEBUG_SHUTDOWN)  || \
			(_AUTOTEST_OUTPUT)  || \
			(_DEBUG_CONN)  || \
			(_DEBUG_READ)  || \
			(_DEBUG_WRITE)  || \
			(_DEBUG_COMPRESSION)  || \
			(_DEBUG_REF)  || \
			(_DEBUG_BUFFER)  || \
			(_DEBUG_PARSE_HTTP)  || \
			(_DEBUG_WS_CONN)  || \
			(_DEBUG_WS_READ)  || \
			(_DEBUG_WS_WRITE)  || \
			(_DEBUG_WS_FRAMES)

#include <stdio.h>

#ifdef WIN32
	#include <io.h>
	#ifndef isatty
		#define isatty _isatty
	#endif

	#ifndef __func__ 
	#define __func__ __FUNCTION__
	#endif
#endif

#define	_B_WHITE	0
#define	_B_RED		1
#define	_B_GREEN	2
#define	_B_YELLOW	3
#define	_B_BLUE		4
#define	_B_MAGENTA	5
#define	_B_CYAN		6
#define	_WHITE		7
#define	_RED		8
#define	_GREEN		9
#define	_YELLOW		10
#define	_BLUE		12
#define	_MAGENTA	13
#define	_CYAN		14	

#ifndef _DEBUG_TRACE_COLOR_ENUM
#define _DEBUG_TRACE_COLOR_ENUM

static const char * const _lbColor[] = {
"[30;1m", /* Bright White */
"[31;1m", /* Bright Red */
"[32;1m", /* Bright Green */
"[33;1m", /* Bright Yellow */
"[34;1m", /* Bright Blue */
"[35;1m", /* Bright Magenta */
"[36;1m", /* Bright Cyan */
"[30m", /* White */
"[31m", /* Red */
"[32m", /* Green */
"[33m", /* Yellow */
"[34m", /* Blue */
"[35m", /* Magenta */
"[36m", /* Cyan */
};
#endif  /* _DEBUG_TRACE_COLOR_ENUM */

#ifndef _DEBUG_NO_COLOR
	#define _TRACE_FPRINTF(fmt, lbl, ci, ...) \
		{ \
			static int tty = 0; \
			if (!tty) \
				tty = isatty(2) + 1; \
			if (tty >= 2 && ci >= 0)\
				fprintf(stderr, "\n%c%s%15s - %s():%d:%c[0m " fmt, 27, _lbColor[ci], lbl, __func__, __LINE__, 27, ##__VA_ARGS__);\
			else\
				fprintf(stderr, "\n%15s - %s():%d: " fmt, lbl, __func__, __LINE__, ##__VA_ARGS__);\
		} 
#else
	#define _TRACE_FPRINTF(fmt, lbl, ci, ...) \
		{ \
			fprintf(stderr, "\n%15s - %s():%d: " fmt, lbl, __func__, __LINE__, ##__VA_ARGS__);\
		} 
#endif

#else
	#define _TRACE_FPRINTF(fmt, lbl, ci, ...) 

#endif /* if defined _DEBUG_CONN || _DEBUG_READ || ...  */

#if defined (_DEBUG_MUTEX) || (MUTEX_DEBUG)
	
	#ifdef WIN32
		#define _RSSL_THREAD_ID()     GetCurrentThreadId()
	#else
		#define _RSSL_THREAD_ID()     pthread_self()
	#endif
	// _type - a string label for the type of Mutex call(i.e. "_MUTEX_UNLOCK"
	// _container - pointer to containg structure where mutex is a member
	// _mutex - is the pointer to the mutex
	#define _DEBUG_MUTEX_TRACE(_type, _container, _mutex) \
		{\
			fprintf(stderr, "-- thrd:%u %s():%d - %s\n\t\t%s:%p\n\t\t%s:%p\n", \
								_RSSL_THREAD_ID(), \
								__func__, __LINE__, _type, \
								#_container, _container, \
								#_mutex, _mutex);\
		}
#else
	#define _DEBUG_MUTEX_TRACE_DISABLED
	#define _DEBUG_MUTEX_TRACE(_type, _container, _mutex)
#endif


#ifdef _DEBUG_WS_FRAMES

	#ifndef _NO_DEBUG_WS_MASK
	#define _DEBUG_TRACE_WS_MASK(__pos)\
		{   rtrUInt32 __m = 0;\
			rwfGet32(__m, __pos);\
			fprintf(stderr, "\t                  Mask Key Value : 0x%02x(%u)\n", __m, __m);\
		}
	#else
	#define _DEBUG_TRACE_WS_MASK(__pos)
	#endif

	#ifndef _NO_DEBUG_WS_BUFFER_DUMP
	#define _DEBUG_TRACE_WS_BUFFER_DUMP(__buf, __len)\
		{	int i=0; fprintf(stderr, "%15s (%u) : '", "Buffer Hex Dump", __len);\
			for (i=0; i < __len; i++){\
				fprintf(stderr, "0x%02x%s", (unsigned char)__buf[i], (i+1<__len?" ":"")); }\
			fprintf(stderr, "' \n");\
		}

	#else
	#define _DEBUG_TRACE_WS_BUFFER_DUMP(__buf, __len)
	#endif


	#define _DEBUG_TRACE_WS_FRAME(__buf) \
	{  int __hdrlen = _WS_CONTROL_HEADER_LEN + ((rwfGetBit(__buf[1], 7)==1)?_WS_MASK_KEY_FIELD_LEN:0); \
		 __hdrlen += ((__buf[1] & 0x7F) == 126?_WS_2BYTE_EXT_PAYLOAD:\
					  ((__buf[1] & 0x7F) == 127?_WS_8BYTE_EXT_PAYLOAD:0)); \
			fprintf(stderr, "\n");\
			fprintf(stderr, "%15s - %s():%d:\n", "WS Frame", __func__, __LINE__);\
			fprintf(stderr, "                   F 1 2 3 OpC M PayLen\n");\
			fprintf(stderr, "                  +-+-+-+-+---+-+------+\n");\
			fprintf(stderr, "                  |%d|%d|%d|%d|0x%1x|%d| 0x%02x |\n",\
								rwfGetBit(__buf[0], _WS_BIT_POS_FIN), \
								rwfGetBit(__buf[0], _WS_BIT_POS_RSV1), \
								rwfGetBit(__buf[0], _WS_BIT_POS_RSV2), \
								rwfGetBit(__buf[0], _WS_BIT_POS_RSV3), \
								(__buf[0] & 0x0F), \
								rwfGetBit(__buf[1], _WS_BIT_POS_MASKKEY), \
								(__buf[1] & 0x7F));\
			fprintf(stderr, "                  +-+-+-+-+---+-+------+\n");\
			fprintf(stderr, "\t                      Header Len : %d \n", __hdrlen); \
			if ((__buf[1] & 0x7F) == 126) {\
				rtrUInt16 __pl126;\
				rwfGet16(__pl126, (__buf+_WS_CONTROL_HEADER_LEN));\
				fprintf(stderr, "\t         16 Bit Extended Payload : 0x%x(%u)\n", __pl126, __pl126);\
				if (rwfGetBit(__buf[1], _WS_BIT_POS_MASKKEY)) {\
					_DEBUG_TRACE_WS_MASK((__buf+_WS_CONTROL_HEADER_LEN+_WS_126PAYLOAD_FIELD_LEN)) \
				}\
			} else if ((__buf[1] & 0x7F) == 127) {\
				rtrUInt64 __pl127;\
				rwfGet64(__pl127, (__buf+_WS_CONTROL_HEADER_LEN));\
				fprintf(stderr, "\t         64 Bit Extended Payload : 0x"RTR_LLX"("RTR_LLD")\n", __pl127, __pl127);\
				if (rwfGetBit(__buf[1], _WS_BIT_POS_MASKKEY)) {\
					_DEBUG_TRACE_WS_MASK((__buf+_WS_CONTROL_HEADER_LEN+_WS_127PAYLOAD_FIELD_LEN)) \
				}\
			} else {\
				fprintf(stderr, "\t                  Payload Length : 0x%x(%u)\n", (__buf[1] & 0x7F), (__buf[1] & 0x7F));\
				if (rwfGetBit(__buf[1], _WS_BIT_POS_MASKKEY)) {\
					_DEBUG_TRACE_WS_MASK((__buf+_WS_CONTROL_HEADER_LEN)) \
				}\
			}\
		}
	#ifndef _NO_DEBUG_WS_FRAME_STRUCT_FIELD
	#define _DEBUG_TRACE_WS_FRAME_STRUCT_FIELD(fmt, __mbr, ...)\
	{ \
		fprintf(stderr, "              %s : " fmt, #__mbr, __mbr, ##__VA_ARGS__);\
	}

	#else
	#define _DEBUG_TRACE_WS_FRAME_STRUCT_FIELD(fmt, __mbr, ...)
	#endif

	#ifndef _NO_DEBUG_WS_FRAME_STRUCT
	#define _DEBUG_TRACE_WS_FRAME_STRUCT(__fs)\
	{ \
		fprintf(stderr, "\n");\
		fprintf(stderr, "%15s - %s():%d:\n", "WS Frame Struct", __func__, __LINE__);\
		if(__fs->hdrLen) { _DEBUG_TRACE_WS_BUFFER_DUMP(__fs->buffer, __fs->hdrLen)}\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->cursor)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->hdrLen)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->extHdrLen)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->partial)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->finSet)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("0x%1x\n", __fs->opcode)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->dataType)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->control)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->fragment)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->compressed)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->maskSet)\
		if(__fs->maskSet) { _DEBUG_TRACE_WS_MASK(__fs->mask) }\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD("%u\n", __fs->maskVal)\
		_DEBUG_TRACE_WS_FRAME_STRUCT_FIELD(RTR_LLD, __fs->payloadLen)\
		fprintf(stderr, "\n\n");\
	}
	#else
	#define _DEBUG_TRACE_WS_FRAME_STRUCT(__fs)
	#endif

#else
	#define _DEBUG_TRACE_WS_MASK(__pos)

	#define _DEBUG_TRACE_WS_BUFFER_DUMP(__buf, __len)

	#define _DEBUG_TRACE_WS_FRAME(__buf)

	#define _DEBUG_TRACE_WS_FRAME_STRUCT_FIELD(fmt, __mbr, ...)

	#define _DEBUG_TRACE_WS_FRAME_STRUCT(__fs)
#endif

#ifdef _DEBUG_PRINT
	#define _DEBUG_TRACE_PRINT(fmt, ...)\
			_TRACE_FPRINTF(fmt, "     Debug", _BLUE, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_PRINT(fmt, ...)
#endif

#ifdef _DEBUG_INIT
	#define _DEBUG_TRACE_INIT(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Init Trace", _BLUE, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_INIT(fmt, ...)
#endif

#ifdef _DEBUG_SHUTDOWN
	#define _DEBUG_TRACE_SHUTDOWN(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Shutdown Trace", _RED, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_SHUTDOWN(fmt, ...)
#endif

#ifdef _AUTOTEST_OUTPUT
			// Passing -1 turns off color formatting
	#define _AUTOTEST_DISPLAY(fmt, ...)\
			_TRACE_FPRINTF(fmt, "AT ", -1, ##__VA_ARGS__)
#else
	#define _AUTOTEST_DISPLAY(fmt, ...)
#endif

#ifdef _DEBUG_CONN
	#define _DEBUG_TRACE_CONN(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Conn Trace", _B_RED, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_CONN(fmt, ...)
#endif

#ifdef _DEBUG_READ
	#define _DEBUG_TRACE_READ(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Read Trace", _B_GREEN, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_READ(fmt, ...)
#endif

#ifdef _DEBUG_WRITE
	#define _DEBUG_TRACE_WRITE(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Write Trace", _B_YELLOW, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_WRITE(fmt, ...)
#endif

#ifdef _DEBUG_COMPRESSION
	#define _DEBUG_TRACE_COMPRESSION(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Compres Trace", _BLUE, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_COMPRESSION(fmt, ...)
#endif

#ifdef _DEBUG_WS_READ
	#define _DEBUG_TRACE_WS_READ(fmt, ...)\
			_TRACE_FPRINTF(fmt, "WS Read Trace", _B_MAGENTA, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_WS_READ(fmt, ...)
#endif

#ifdef _DEBUG_WS_WRITE
	#define _DEBUG_TRACE_WS_WRITE(fmt, ...)\
			_TRACE_FPRINTF(fmt, "WS Write Trace", _B_BLUE, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_WS_WRITE(fmt, ...)
#endif

#ifdef _DEBUG_REF
	#define _DEBUG_TRACE_REF(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Ref Trace", _YELLOW, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_REF(fmt, ...)
#endif

#ifdef _DEBUG_BUFFER
	#define _DEBUG_TRACE_BUFFER(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Buffer Trace", _B_GREEN, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_BUFFER(fmt, ...)
#endif

#ifdef _DEBUG_WS_CONN
	#define _DEBUG_TRACE_WS_CONN(fmt, ...)\
			_TRACE_FPRINTF(fmt, "WS Conn Trace", _B_CYAN, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_WS_CONN(fmt, ...)
#endif

#ifdef _DEBUG_PARSE_HTTP
	#define _DEBUG_TRACE_PARSE_HTTP(fmt, ...)\
			_TRACE_FPRINTF(fmt, "Http Trace", _GREEN, ##__VA_ARGS__)
#else
	#define _DEBUG_TRACE_PARSE_HTTP(fmt, ...)
#endif


//#endif /* ifndef __RTR_DEBUG_PRINT_H */
