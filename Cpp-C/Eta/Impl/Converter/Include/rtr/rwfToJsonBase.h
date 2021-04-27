/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtr_rwfToJsonBase
#define __rtr_rwfToJsonBase

#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/custmem.h"
#include "rtr/platform.h"
#include "rtr/rjcbuffer.h"
#include "rtr/rsslJsonConverter.h"
#include "jsmn.h"

#define MAX_MSG_PREQUEL 24
#define JSON_FIXED_PREQUEL 10
#define AJAX_FIXED_PREQUEL 12
#define AJAX_SUFFIX 2

#define _COMMA_CHAR ','
#define _COLON_CHAR ':'
#define _DOUBLE_QUOTE_CHAR '"'
#define _DOUBLE_QUOTE "\""
#define _BACK_SLASH_CHAR_NEW '\\'
#define _BACK_SLASH "\\"
#define _OB_CHAR '{'
#define _OE_CHAR '}'
#define _AB_CHAR '['
#define _AE_CHAR ']'

#ifndef u_16
	#define	u_16	RsslUInt16
#endif

#ifndef u_32
	#define	u_32	RsslUInt32
#endif

// Be careful changing this value. There are copy operations below that assume
// the results character string is 6 bytes or less.
#define RTR_RTMC_MAX_INT_TO_STR_TABLE 65535

#define RTR_NEW_DO_LONG_TO_STRING(quo,rem,value,dptr) \
{ \
	while (value != 0) \
	{ \
		__rtr_u32div10(value,quo,rem); \
		*(--dptr) = (char) (rem + '0'); \
		value = quo;\
	}; \
}

#define RTR_NEW_DO_LONGLONG_TO_STRING(value,dptr) \
{ \
	while (value != 0) \
	{ \
		RsslUInt64 quo = value / 10; \
		*(--dptr) = (char) ((value - (quo * 10)) + '0'); \
		value = quo; \
	}; \
}

#define RTR_NEW2_DO_LONGLONG_TO_STRING(value,dptr) \
{ \
	while (value != 0) \
	{ \
		*(--dptr) = (char) ((value % 10) + '0'); \
		value = value / 10; \
	}; \
}

#if !defined(DEV_QUAD_32_ALIGN) || defined(i86pc)

#define __rtr_copy8orless( to, from, length ) \
{ \
	register unsigned int ___len = length; \
	*(unsigned int*)(to) = *(unsigned int*)(from); \
	*(unsigned int*)(to+4) = *(unsigned int*)((from) + 4); \
	to += ___len; \
}

#define __rtr_copy6orless( to, from, length ) \
{ \
	register unsigned int ___len = length; \
	*(unsigned int*)(to) = *(unsigned int*)(from); \
	*(unsigned short*)(to+4) = *(unsigned short*)((from) + 4); \
	to += ___len; \
}

#define __rtr_copy6orless_uselen( to, from, length ) \
{ \
	*(unsigned int*)(to) = *(unsigned int*)(from); \
	*(unsigned short*)(to+4) = *(unsigned short*)((from) + 4); \
	to += length; \
}

#define __rtr_copy4orless( to, from, length ) \
{ \
	*(unsigned int*)(to) = *(unsigned int*)(from); \
	to += length; \
}

#define __rtr_copy2orless( to, from, length ) \
{ \
	*(unsigned short*)(to) = *(unsigned short*)(from); \
	to += length; \
}

#else

#define __rtr_copy8orless( to, from, length ) \
{ \
	writeValue(from,length); \
}

#define __rtr_copy6orless( to, from, length ) \
{ \
	writeValue(from,length); \
}

#define __rtr_copy6orless_uselen( to, from, length ) \
{ \
	writeValue(from,length); \
}

#define __rtr_copy4orless( to, from, length ) \
{ \
	writeValue(from,length); \
}

#define __rtr_copy2orless( to, from, length ) \
{ \
	writeValue(from,length); \
}

#endif


class RJCSmallString;
typedef RJCSmallString* (*getServiceNameByIdCb)(void*, RsslUInt16);

class rwfToJsonBase
{
public:

	enum ConversionFlags {
		NoFlags				= 0x00,
		EncodeRealAsPrimitive		= 0x01,
		EnumExpansionFlag		= 0x02
	};


	// Constructor
	rwfToJsonBase(int bufSize, int maxPrequel, RsslUInt16 convFlags = 0, int numTokens = DEFAULT_NUM_TOKENS, int incSize = DEFAULT_NUM_TOKENS);

	// Destructor
	virtual ~rwfToJsonBase();

	static void initializeIntToStringTable();
	static void uninitializeIntToStringTable();

	static bool _intToStringTableInit;

	int convertMsg(RsslMsg &);

	virtual const RsslBuffer* getJsonMsg(RsslInt32 streamId, bool solicited = true, bool close = false) = 0;
	const RsslBuffer* getAjaxMsg(RsslUInt32 streamId, bool solicited = true);
	void setSolicitedFlag(bool solicited);
	void setStreamingState(int streamingState);

	bool buildJsonTerminateStream(RJCBuffer &oMsg, int errorCode, RJCString &text);

	void setDictionaryList(RsslDataDictionary **pDictionaryList, RsslUInt32 dictionaryCount)
	{
		_dictionaryList = pDictionaryList;
		_dictionaryCount = dictionaryCount;
	}

	inline bool error()
	{
		return _error > 0;
	};
	virtual void reset();
	void setRsslServiceIdToNameCallback(void* closure, RsslJsonServiceIdToNameCallback *callback)
	{ _closure = closure; _rsslServiceIdToNameCallback = callback; }
	bool enumExpansion() { return _convFlags & EnumExpansionFlag ? true : false; }

 protected:

	RsslDataDictionary **_dictionaryList;
	RsslUInt32	_dictionaryCount;

	RsslJsonServiceIdToNameCallback *_rsslServiceIdToNameCallback;
	void *_closure;

	int _msgType;
	char *_solicitedFlagPtr;
	char *_streamingStatePtr;
	int _msgClass;
	int _dataFormat;
	int _error;
	int _maxLen;
	int _bufLen;
	bool _firstMsg;
	bool _reset;
	char *_buf;
	bool _hasErrorDebugInfo;
	RsslInt32 _errorId;
	char *_errorText;
	char *_errorFile;
	RsslInt32 _errorLine;
	RsslInt32 _errorOffset;
	char *_errorOriginalMessage;
	RsslUInt16 _convFlags;
	RsslDecodeIterator _iter;
	RsslBuffer _buffer;
	int	_maxPrequel;

	// Buffers used for RMTES to UTF8 conversion
	RsslUInt16* _ucs2Buf;
	int _ucs2BufSz;
	char* _utf8Buf;
	int _utf8BufSz;

	// Exclusively for OMM JSON container type
	jsmntok_t *_tokens;
	int _numTokens;
	int _incSize;

	char *_pstr;
	u_32 _size;

	int resize(int newSize);
	int resizeAndRealloc(int newSize);

	// Message Handlers
	virtual int processMsg(RsslDecodeIterator*, RsslMsg &, bool first=true) = 0;
	typedef  int (rwfToJsonBase::*msgHandlerFuncPtr)(RsslDecodeIterator*, RsslMsg &);
	static const msgHandlerFuncPtr _msgHandlers[];
	virtual int processRequestMsg(RsslDecodeIterator*, RsslMsg &) = 0;
	virtual int processRefreshMsg(RsslDecodeIterator*, RsslMsg &) = 0;
	virtual int processStatusMsg(RsslDecodeIterator*, RsslMsg &) = 0;
	virtual int processUpdateMsg(RsslDecodeIterator*, RsslMsg &) = 0;
	virtual int processCloseMsg(RsslDecodeIterator*, RsslMsg &) = 0;
	virtual int processAckMsg(RsslDecodeIterator*, RsslMsg &) = 0;
	virtual int processGenericMsg(RsslDecodeIterator*, RsslMsg &) = 0;
	virtual int processPostMsg(RsslDecodeIterator*, RsslMsg &) = 0;

	virtual int processQOS(const RsslQos*) = 0;
	virtual int processState(const RsslState*) = 0;
	virtual int processPostUserInfo(RsslPostUserInfo*) = 0;

	//	int processFieldListSetdb(RsslLocalFieldSetDefDb&);
	//	int processElementListSetdb(RsslLocalElementSetDefDb&);

	//	static DEV_THREAD_LOCAL char* _elementSetDefDbMem;
	//	static DEV_THREAD_LOCAL char* _fieldSetDefDbMem;

	// Container Handlers
	typedef  int (rwfToJsonBase::*containerHandlerFuncPtr)( RsslDecodeIterator *, const RsslBuffer *, void *, bool writeTag);
	static const containerHandlerFuncPtr _containerHandlers[];
	virtual int processOpaque(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag);
	virtual int processXml(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag);
	virtual int processFieldList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag) = 0;
	virtual int processElementList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag) = 0;
	int processAnsiPage(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag);
	virtual int processFilterList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag) = 0;
	virtual int processVector(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag) = 0;
	virtual int processMap(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag) = 0;
	virtual int processSeries(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag) = 0;
	virtual int processMsg(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag);
	virtual int processJson(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag) = 0;

	// Primitive Handlers
	int processPrimitive(int primitiveType, RsslBuffer * bufPtr);
	int processPrimitive(RsslDecodeIterator *, int primitiveType);
	typedef  int (rwfToJsonBase::*primitiveHandlerPtr)(RsslDecodeIterator *);
	static const primitiveHandlerPtr _primitiveHandlers[];
	int processUnknown(RsslDecodeIterator *);
	int processInteger(RsslDecodeIterator *);
	int processUnsignedInteger(RsslDecodeIterator *);
	int processFloat(RsslDecodeIterator *);
	int processDouble(RsslDecodeIterator *);
	virtual int processReal(RsslDecodeIterator *) = 0;
	virtual int processDate(RsslDecodeIterator *) = 0;
	virtual int processTime(RsslDecodeIterator *) = 0;
	virtual int processDateTime(RsslDecodeIterator *) = 0;
	int processQOS(RsslDecodeIterator *);
	int processState(RsslDecodeIterator *);
	int processEnumeration(RsslDecodeIterator *);
	virtual int processArray(RsslDecodeIterator *) = 0;
	int processBuffer(RsslDecodeIterator *);
	int processAsciiString(RsslDecodeIterator *);
	int processUTF8String(RsslDecodeIterator *);
	int processRMTESString(RsslDecodeIterator *);

	static char *_intToStringTable[];
	static RsslUInt8 _intToStringTableLengths[];

	static const char _ajaxMsgType[];
	static const int _setToBaseType[];
	static const int _exponentTable[];

	void largeUInt32ToString(RsslUInt32 value, int hint = 0, 
			// int32ToStringOffBuffer may call this function, so allow the buffer check to be disabled 
			// (since _pstr is not pointing to the internal message buffer in that case)
			bool verifyBufferSize = true);

	void largeUInt64ToString(RsslUInt64 value, int hint = 0);
	void uInt32ToString(RsslUInt32 value);
	void int32ToString(RsslInt32 value);
	void int32ToStringOffBuffer(RsslInt32 value); // Used when manipualting _pstr off of the Json Message Buffer
	void uInt64ToString(RsslUInt64 value);
	void int64ToString(RsslInt64 value);
	
	inline void writeVar(char var, bool comma);
	inline void writeOb();
	inline void writeOe();
	inline void writeAb();
	inline void writeAe();
	inline void writeComma();
	inline void writeNull();
	inline void writeEmptyObject();
	inline void writeCharDQ(char val);
	inline void writeValueDQ(const char* value);
	inline void writeValueDQ(const char* value, int len);
	inline void writeFieldId(RsslUInt32 fidNum, bool comma);
	inline void writeValue(const char* value);
	inline void writeValue(const char* value, int len);
	inline void writeString(const char* value); //Write this value in ""
	inline void writeString(const char* value, int len); //Write this value in ""
	inline void writeSafeString(const char* value); //Write this value in ""
	inline void writeSafeString(const char* value, int len); //Write this value in ""
	void rmtesToUtf8(const RsslBuffer& buf);
	void encodeBase64(const unsigned char *in, int length);
	void real32ToStr(RsslUInt32 value, int hint);
	void real64ToStr(RsslUInt64 value, int hint);
	virtual int estimateJsonLength(RsslUInt32 rwfLength) = 0;
	
	inline int verifyJsonMessageSize(int additionalLen = 0);
	inline void writeJsonString(const char* value, int len);
	inline void writeJsonString(char value);

	virtual void floatToStr(RsslFloat value);
	virtual void doubleToStr(RsslDouble value);
};

inline void rwfToJsonBase::writeVar(char var, bool comma)
{
	if (verifyJsonMessageSize(5) == 0) return;
	
	if (comma)
		*_pstr++ = _COMMA_CHAR;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = var;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = _COLON_CHAR;
}

inline void rwfToJsonBase::writeOb()
{
	writeJsonString(_OB_CHAR);
}

inline void rwfToJsonBase::writeOe()
{
	writeJsonString(_OE_CHAR);
}
inline void rwfToJsonBase::writeAb()
{
	writeJsonString(_AB_CHAR);
}

inline void rwfToJsonBase::writeAe()
{
	writeJsonString(_AE_CHAR);
}
inline void rwfToJsonBase::writeComma()
{
	writeJsonString(',');
}
inline void rwfToJsonBase::writeNull()
{
	writeJsonString("null", 4);
}

inline void rwfToJsonBase::writeEmptyObject()
{
	writeJsonString("{}", 2);
}

inline void rwfToJsonBase::writeString(const char* value)
{
	size_t len = strlen(value);
	if (value)
	{
		if (verifyJsonMessageSize((int)len + 2) == 0) return;
	}
	else if (verifyJsonMessageSize(2) == 0) return;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	// If null value address, ignore it
	if (!value)
	{
		*_pstr++ = _DOUBLE_QUOTE_CHAR;
		return;
	}
	(len <= 42) ? doSimpleMemCopy(_pstr, value, len) :
		memcpy(_pstr, value, len);
	_pstr += len;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}

inline void rwfToJsonBase::writeString(const char* value, int len)
{
	if (verifyJsonMessageSize(len + 2) == 0) return;	

	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	// If null value address, ignore it
	if (!value)
	{
		*_pstr++ = _DOUBLE_QUOTE_CHAR;
		return;
	}
	(len <= 42) ? doSimpleMemCopy(_pstr,value,len) :
			memcpy(_pstr, value, len);
	_pstr += len;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}

// This function will escape control/special characters if needed.
inline void rwfToJsonBase::writeSafeString(const char* value)
{
	int len = (int)strlen(value);
	writeSafeString(value, len);
}

// This function will escape control/special characters if needed.
inline void rwfToJsonBase::writeSafeString(const char* value, int len)
{
	if (verifyJsonMessageSize(len * 6 + 2) == 0) return; // Assumes worst case scenario
	*_pstr++ = _DOUBLE_QUOTE_CHAR;

	const char* end = value+len;
	while (value != end)
	{
		if (*value > 0x1F && *value != 0x7F)
		{
			switch (*value)
			{
				case '\\':
				//Check to see if we already have back slash or double quotes preceeded by a back slash
					*_pstr++ = '\\';
					if ((value + 1) == end)
					{
						*_pstr++ = '\\';
					}
					else if (*(value+1) == '"')
					{
						*_pstr++ = '"';
						value++;
					}
					else if (*(value+1) == '\\')
					{
						*_pstr++ = '\\';
						value++;
					}
					else
						*_pstr++ = '\\';
					break;
				case '"':
					*_pstr++ = '\\';
					*_pstr++ = '"';
					break;
				default:
					*_pstr++ = *value;
					break;
			}
		}
		else if (*value & 0x80) //Assuming this is UTF-8 encoding
		{
			*_pstr++ = *value; 
		}
		else
		{
			switch (*value)
			{
				case '\b':
					*_pstr++ = '\\';
					*_pstr++ = 'b';
					break;
				case '\f':
					*_pstr++ = '\\';
					*_pstr++ = 'f';
					break;
				case '\n':
					*_pstr++ = '\\';
					*_pstr++ = 'n';
					break;
				case '\r':
					*_pstr++ = '\\';
					*_pstr++ = 'r';
					break;
				case '\t':
					*_pstr++ = '\\';
					*_pstr++ = 't';
					break;
				default:
					_pstr += snprintf(_pstr, 7, "\\u%04x", (unsigned char)*value);
					break;
			}
		}

		value++;
	}

	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}

inline void rwfToJsonBase::writeFieldId(RsslUInt32 fidNum, bool comma)
{
	// Max possible digits in 32 bit int plus commas and quotes
	if (verifyJsonMessageSize(14) == 0) return;
	
	if (comma)
		*_pstr++ = _COMMA_CHAR;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	int32ToString(fidNum);
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = _COLON_CHAR;

}

inline void rwfToJsonBase::writeValue(const char* value)
{
	size_t len = strlen(value);
	if (verifyJsonMessageSize((int)len) == 0) return;

	(len <= 42) ? doSimpleMemCopy(_pstr, value, (unsigned int)len) : memcpy(_pstr, value, (unsigned int)len);
	_pstr += len;
}

inline void rwfToJsonBase::writeValue(const char* value, int len)
{
	if (verifyJsonMessageSize(len) == 0) return;
	(len <= 42) ? doSimpleMemCopy(_pstr, value, (unsigned int)len) : memcpy(_pstr, value, (unsigned int)len);
	_pstr += len;
}

inline void rwfToJsonBase::writeCharDQ(char val)
{
	if (verifyJsonMessageSize(4) == 0) return;
	
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = val;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = _COLON_CHAR;
}

inline void rwfToJsonBase::writeValueDQ(const char* value)
{
	size_t len = strlen(value);
	if (verifyJsonMessageSize((int)len + 2) == 0) return;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	(len <= 42) ? doSimpleMemCopy(_pstr, value, len) :
		memcpy(_pstr, value, len);
	_pstr += len;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}
inline void rwfToJsonBase::writeValueDQ(const char* value, int len)
{
	if (verifyJsonMessageSize(len + 2) == 0) return;
	
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	(len <= 42) ? doSimpleMemCopy(_pstr,value,len) :
			memcpy(_pstr, value, len);
	_pstr += len;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}

inline int rwfToJsonBase::verifyJsonMessageSize(int additionalLen)
{
	if (_pstr - _buf + additionalLen >= _maxLen)	// ">=" to prevent overrun buffer when writing unknown lengths
	{
		// We've gone over buffer max, realloc
		//If resize fails, return failure.
		if( resizeAndRealloc(_maxLen * 2) == 0 )
		{
			_error = 1;
			return 0;
		}
	}
	return 1;
}

inline void rwfToJsonBase::writeJsonString(const char* value, int len)
{	
	if (verifyJsonMessageSize(len) == 0) return;
	
	(len <= 42) ? doSimpleMemCopy(_pstr,value,len) :
			memcpy(_pstr, value, len);
	_pstr += len;
}

inline void rwfToJsonBase::writeJsonString(char value)
{	
	if (verifyJsonMessageSize(1) == 0) return;
	
	*_pstr++ = value;
}

inline int rwfToJsonBase::estimateJsonLength(RsslUInt32 rwfLength)
{
	return ((rwfLength * 6) + 300);
}

#endif
