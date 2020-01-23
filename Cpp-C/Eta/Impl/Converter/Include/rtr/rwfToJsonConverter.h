/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


#ifndef __rtr_rwfToJsonConverter
#define __rtr_rwfToJsonConverter
#include "rtr/rwfToJsonBase.h"

class rwfToJsonConverter : public rwfToJsonBase
{
public:
	// Constructor
	rwfToJsonConverter(int bufSize, u_16 convFlags = 0);

	// Destructor
	~rwfToJsonConverter();

	const RsslBuffer* getJsonMsg(RsslInt32 streamId, bool solicited = true, bool close = false);

	inline void setEncodeRealAsPrimitive(RsslBool encodeRealAsPrimitive)
	{
		if (encodeRealAsPrimitive) _convFlags |= EncodeRealAsPrimitive;
		else _convFlags &= EncodeRealAsPrimitive;
	}

 protected:
	// Message Handlers
	int processMsg(RsslDecodeIterator*, RsslMsg &, bool first=true);
	//	typedef  int (rwfToJsonConverter::*msgHandlerFuncPtr)(RsslDecodeIterator*, RsslMsg &);
	//	static const msgHandlerFuncPtr _msgHandlers[];
	int processRequestMsg(RsslDecodeIterator*, RsslMsg &);
	int processRefreshMsg(RsslDecodeIterator*, RsslMsg &);
	int processStatusMsg(RsslDecodeIterator*, RsslMsg &);
	int processUpdateMsg(RsslDecodeIterator*, RsslMsg &);
	int processCloseMsg(RsslDecodeIterator*, RsslMsg &);
	int processAckMsg(RsslDecodeIterator*, RsslMsg &);
	int processGenericMsg(RsslDecodeIterator*, RsslMsg &);
	int processPostMsg(RsslDecodeIterator*, RsslMsg &);

	int processMsgKey(const RsslMsgKey*, RsslDecodeIterator*);
	int processQOS(const RsslQos*);
	int processState(const RsslState*);
	int processPostUserInfo(RsslPostUserInfo*);

	int processFieldListSetdb(RsslLocalFieldSetDefDb&);
	int processElementListSetdb(RsslLocalElementSetDefDb&);

	static DEV_THREAD_LOCAL char* _elementSetDefDbMem;
	static DEV_THREAD_LOCAL char* _fieldSetDefDbMem;

	// Container Handlers
	int processContainer(RsslUInt8 containerType, RsslDecodeIterator *iterPtr , const RsslBuffer* encDataBuf, bool writeTag, void* setDefPtr = 0);

	int processFieldList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processElementList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processAnsiPage(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processFilterList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processVector(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processMap(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processSeries(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processJson(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	//	int processMsg(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 

	// Primitive Handlers
	int processUnknown(RsslDecodeIterator *);
	int processInteger(RsslDecodeIterator *);
	int processUnsignedInteger(RsslDecodeIterator *);
	int processFloat(RsslDecodeIterator *);
	int processDouble(RsslDecodeIterator *);
	int processReal(RsslDecodeIterator *);
	int processDate(RsslDecodeIterator *);
	int processTime(RsslDecodeIterator *);
	int processDateTime(RsslDecodeIterator *);
	int processQOS(RsslDecodeIterator *);
	int processState(RsslDecodeIterator *);
	int processEnumeration(RsslDecodeIterator *);
	int processArray(RsslDecodeIterator *);
	int processBuffer(RsslDecodeIterator *);
	int processAsciiString(RsslDecodeIterator *);
	int processUTF8String(RsslDecodeIterator *);
	int processRMTESString(RsslDecodeIterator *);

	int estimateJsonLength(RsslUInt32 rwfLength);
};
#ifdef DO_NOT_COMPILE
inline void rwfToJsonConverter::writeVar(char var, bool comma)
{
	if (comma)
		*_pstr++ = _COMMA_CHAR;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = var;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = _COLON_CHAR;
}

inline void rwfToJsonConverter::writeOb()
{
	*_pstr++ = _OB_CHAR;
}

inline void rwfToJsonConverter::writeOe()
{
	*_pstr++ = _OE_CHAR;
}
inline void rwfToJsonConverter::writeAb()
{
	*_pstr++ = _AB_CHAR;
}

inline void rwfToJsonConverter::writeAe()
{
	*_pstr++ = _AE_CHAR;
}
inline void rwfToJsonConverter::writeComma()
{
	*_pstr++ = ',';
}
inline void rwfToJsonConverter::writeNull()
{
	*_pstr++ = 'n';
	*_pstr++ = 'u';
	*_pstr++ = 'l';
	*_pstr++ = 'l';
}
// MJD - ToDo Need to look for Double Quotes and escape them
//			  Need to find out JSON standard 
//			  URLs use somthing like this
//            Replace " with %22
//            Replace % with %%
inline void rwfToJsonConverter::writeString(const char* value)
{
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	while (*_pstr = *value)
		_pstr++, value++;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}


inline void rwfToJsonConverter::writeString(const char* value, int len)
{
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	(len <= 42) ? doSimpleMemCopy(_pstr,value,len) :
			memcpy(_pstr, value, len);
	_pstr += len;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}

inline void rwfToJsonConverter::writeFieldId(RsslUInt32 fidNum, bool comma)
{
	if (comma)
		*_pstr++ = _COMMA_CHAR;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	int32ToString(fidNum);
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = _COLON_CHAR;
}
inline void rwfToJsonConverter::writeCharDQ(char val)
{
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = val;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = _COLON_CHAR;
}
inline void rwfToJsonConverter::writeValueDQ(const char* value)
{
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	while (*_pstr = *value)
		_pstr++, value++;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}
inline void rwfToJsonConverter::writeValueDQ(const char* value, int len)
{
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	(len <= 42) ? doSimpleMemCopy(_pstr,value,len) :
			memcpy(_pstr, value, len);
	_pstr += len;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	
}
#endif
#endif
