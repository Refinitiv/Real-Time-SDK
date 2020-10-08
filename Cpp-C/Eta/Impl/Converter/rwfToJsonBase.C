/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */


#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "rtr/rsslDataPackage.h"
#include "rtr/rtrdiv.h"
#include "rtr/rtrdiv10.h"

#include "rtr/rwfToJsonBase.h"
#include "rtr/rsslRmtes.h"

//Use 1 to 3 byte variable UTF encoding
#define MaxUTF8Bytes 3

char *rwfToJsonBase::_intToStringTable[RTR_RTMC_MAX_INT_TO_STR_TABLE + 1];
RsslUInt8 rwfToJsonBase::_intToStringTableLengths[RTR_RTMC_MAX_INT_TO_STR_TABLE + 1];
bool rwfToJsonBase::_intToStringTableInit = false;

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////////
rwfToJsonBase::rwfToJsonBase(int bufSize, int maxPrequel, RsslUInt16 convFlags, int numTokens, int incSize) :
	_closure(0),
	_rsslServiceIdToNameCallback(0),
	_msgClass(0),
	_dataFormat(0),
	_error(0),
	_maxLen(0),
	_bufLen(0),
	_reset(true),
	_convFlags(convFlags),
	_buf(0),
	_solicitedFlagPtr(0),
	_streamingStatePtr(0),
	_ucs2Buf(0),
	_ucs2BufSz(0),
	_utf8Buf(0),
	_utf8BufSz(0),
	_maxPrequel(maxPrequel),
	_numTokens(numTokens),
	_incSize(incSize),
	_pstr(0),
	_size(0),
	_dictionaryList(0),
	_dictionaryCount(0)
{

	/* Add in a buffer for a worst case 8 byte memcpy by int */
	if ((_buf = (char*)malloc(bufSize + 8)) == 0)
		_error = 1;

	// Start with 1K for each string conversion buffer
	_ucs2Buf = new RsslUInt16[1024];
	_ucs2BufSz = 1024;

	_utf8Buf = new char[1024];
	_utf8BufSz = 1024;

	_pstr = _buf + _maxPrequel;
	_maxLen = bufSize;

	if ((_tokens = (jsmntok_t*)malloc(numTokens * sizeof(jsmntok_t))) == NULL)
		_error = 1;

	memset(_tokens, 0, _numTokens * sizeof(jsmntok_t));
}
//////////////////////////////////////////////////////////////////////
//
// Destructor
//
//////////////////////////////////////////////////////////////////////
rwfToJsonBase::~rwfToJsonBase()
{
	if(_tokens)
		free(_tokens);

	if (_buf)
	{
		free(_buf);
		_buf = 0;
	}
	if (_ucs2Buf)
	{
		delete [] _ucs2Buf;
		_ucs2BufSz = 0;
	}
	if (_utf8Buf)
	{
		delete [] _utf8Buf;
		_utf8BufSz = 0;
	}
}
void rwfToJsonBase::reset()
{
	_pstr = _buf + _maxPrequel;
	_msgClass = _dataFormat = 0;
	_solicitedFlagPtr = 0;
	_streamingStatePtr = 0;
	_firstMsg = true;
	_error = 0;
	_reset = true;
}

//////////////////////////////////////////////////////////////////////
//
// Our buffer is not big enough resize it.
//
//////////////////////////////////////////////////////////////////////
int rwfToJsonBase::resize(int newSize)
{
	if(_buf)
		free(_buf);

	_buf = 0;
	_maxLen = 0;
	/* Add in a buffer for a worst case 8 byte memcpy by int */
	if ((_buf = (char*)malloc(newSize+8)) == 0)
	{
		_error = 1;
		return 0;
	}
	_maxLen = newSize;
	return 1;
}
//////////////////////////////////////////////////////////////////////
//
// Our buffer is not big enough resize it - also copy previous buffer
//
//////////////////////////////////////////////////////////////////////
int rwfToJsonBase::resizeAndRealloc(int newSize)
{
	char* tempBuf = 0;
	int ptrCount = (int)(_pstr - _buf);
	/* Add in a buffer for a worst case 8 byte memcpy by int */
	if ((tempBuf = (char*)malloc(newSize+8)) == 0)
	{
		_error = 1;
		return 0;
	}
	memcpy(tempBuf, _buf, ptrCount);
	// Move _pstr
	_pstr = tempBuf + ptrCount;
	free(_buf);
	_buf = tempBuf;
	_maxLen = newSize;
	return 1;
}
//////////////////////////////////////////////////////////////////////
//
// Handler Jump Tables
//
//////////////////////////////////////////////////////////////////////
const rwfToJsonBase::msgHandlerFuncPtr rwfToJsonBase::_msgHandlers[] =
	{
		0, // Not Valid
		&rwfToJsonBase::processRequestMsg,
		&rwfToJsonBase::processRefreshMsg,
		&rwfToJsonBase::processStatusMsg,
		&rwfToJsonBase::processUpdateMsg,
		&rwfToJsonBase::processCloseMsg,
		&rwfToJsonBase::processAckMsg,
		&rwfToJsonBase::processGenericMsg,
		&rwfToJsonBase::processPostMsg
	};
const rwfToJsonBase::containerHandlerFuncPtr rwfToJsonBase::_containerHandlers[] =
	{
		0, // Reserved
		0, // Reserved
		&rwfToJsonBase::processOpaque,
		&rwfToJsonBase::processXml,
		&rwfToJsonBase::processFieldList,
		&rwfToJsonBase::processElementList,
		&rwfToJsonBase::processAnsiPage,
		&rwfToJsonBase::processFilterList,
		&rwfToJsonBase::processVector,
		&rwfToJsonBase::processMap,
		&rwfToJsonBase::processSeries,
		0, // Reserved 
		0, // Reserved
		&rwfToJsonBase::processMsg,
		&rwfToJsonBase::processJson
	};

const rwfToJsonBase::primitiveHandlerPtr rwfToJsonBase::_primitiveHandlers[] =
	{
		&rwfToJsonBase::processUnknown,  // 0
		0, // Reserved
		0, // Reserved
		&rwfToJsonBase::processInteger,
		&rwfToJsonBase::processUnsignedInteger,
		&rwfToJsonBase::processFloat,
		&rwfToJsonBase::processDouble,
		0, // Reserved
		&rwfToJsonBase::processReal,
		&rwfToJsonBase::processDate,
		&rwfToJsonBase::processTime,
		&rwfToJsonBase::processDateTime,
		&rwfToJsonBase::processQOS,
		&rwfToJsonBase::processState,
		&rwfToJsonBase::processEnumeration,
		&rwfToJsonBase::processArray,
		&rwfToJsonBase::processBuffer,
		&rwfToJsonBase::processAsciiString,
		&rwfToJsonBase::processUTF8String,
		&rwfToJsonBase::processRMTESString,  // 19
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 20 - 63 Reserved
		&rwfToJsonBase::processInteger,	// 64
		&rwfToJsonBase::processUnsignedInteger,
		&rwfToJsonBase::processInteger,
		&rwfToJsonBase::processUnsignedInteger,
		&rwfToJsonBase::processInteger,
		&rwfToJsonBase::processUnsignedInteger,
		&rwfToJsonBase::processInteger,
		&rwfToJsonBase::processUnsignedInteger,
		&rwfToJsonBase::processFloat,
		&rwfToJsonBase::processDouble,
		&rwfToJsonBase::processReal,
		&rwfToJsonBase::processReal,
		&rwfToJsonBase::processDate,
		&rwfToJsonBase::processTime,
		&rwfToJsonBase::processTime,
		&rwfToJsonBase::processDateTime,
		&rwfToJsonBase::processDateTime	// 80
	};

//const char rwfToJsonBase::_ajaxMsgType[] = {0,'r','i','s','u','c','a','g','p'};
const char rwfToJsonBase::_ajaxMsgType[] = {0,'d','d','d','d','d','d','d','d'};

const int rwfToJsonBase::_setToBaseType[] = {3,4,3,4,3,4,3,4,5,6,8,8,9,10,10,11,11};
const int rwfToJsonBase::_exponentTable[] = {-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7};



void rwfToJsonBase::uninitializeIntToStringTable()
{
	if (_intToStringTableInit)
	{
		for (unsigned long i = 0; i <= RTR_RTMC_MAX_INT_TO_STR_TABLE; i++)
		{
			delete [] _intToStringTable[i];
			_intToStringTable[i] = 0;
			_intToStringTableLengths[i] = 0;
		}

		_intToStringTableInit = false;
	}
}

void rwfToJsonBase::initializeIntToStringTable()
{
	unsigned long quo, rem;
	char temp[12];
	unsigned int len ;
	unsigned long value;

		// Must initialize zero case before loop (macro does not handle zero).
	char *mem = new char[7];
	memset(mem, 0 , 7);

	mem[0] = '0';

	_intToStringTable[0] = mem;
	_intToStringTableLengths[0] = 1;

	for (unsigned long i = 1; i <= RTR_RTMC_MAX_INT_TO_STR_TABLE; i++)
	{
		char *tstr = temp + 11;
		*(--tstr) = 0;

		value = i;
		RTR_NEW_DO_LONG_TO_STRING(quo,rem,value, tstr);
		len = (unsigned int)strlen(tstr);

		if (len < 7)
		{	// should always hit here
			// force to 6 + NULL - avoid ABR when using _intToStringTable
			mem = new char[7];
			memset(mem, 0, 7);
		}
		else
		{
			mem = new char[len + 1];
			memset(mem, 0, len + 1);
		}

		memcpy(mem, tstr, len);

		_intToStringTable[i] = mem;
		_intToStringTableLengths[i] = len;
	}

	_intToStringTableInit = true;
}

void rwfToJsonBase::uInt32ToString(RsslUInt32 value)
{
	register char *tstr;

	if (value <= RTR_RTMC_MAX_INT_TO_STR_TABLE)
	{
		tstr = _intToStringTable[value];
		if (verifyJsonMessageSize(_intToStringTableLengths[value]) == 0) return;
		__rtr_copy6orless(_pstr, tstr, _intToStringTableLengths[value]);
	}
	else
	{
		largeUInt32ToString(value);
	}
}

void rwfToJsonBase::int32ToString(RsslInt32 value)
{
	register char *tstr;
	register RsslUInt32 tval;

	if (value >= 0)
	{
		tval = value;
	}
	else
	{
		tval = -value;
		writeJsonString('-');
	}

	if (tval <= RTR_RTMC_MAX_INT_TO_STR_TABLE)
	{
		tstr = _intToStringTable[tval];
		if (verifyJsonMessageSize(_intToStringTableLengths[tval]) == 0) return;
		__rtr_copy6orless(_pstr, tstr, _intToStringTableLengths[tval]);
	}
	else
	{
		largeUInt32ToString(tval);
	}
}

void rwfToJsonBase::int32ToStringOffBuffer(RsslInt32 value)
{
	register char *tstr;
	register RsslUInt32 tval;

	if (value >= 0)
	{
		tval = value;
	}
	else
	{
		tval = -value;
		*_pstr++ = '-';
	}

	if (tval <= RTR_RTMC_MAX_INT_TO_STR_TABLE)
	{
		tstr = _intToStringTable[tval];
		__rtr_copy6orless(_pstr, tstr, _intToStringTableLengths[tval]);
	}
	else
	{
		largeUInt32ToString(tval, 0, false);
	}
}

void rwfToJsonBase::uInt64ToString(RsslUInt64 value)
{
	register char *tstr;

	if (value <= UINT_MAX)
	{
		register RsslUInt32 tval = (RsslUInt32)value;

		if (tval <= RTR_RTMC_MAX_INT_TO_STR_TABLE)
		{
			tstr = _intToStringTable[tval];
			if (verifyJsonMessageSize(_intToStringTableLengths[tval]) == 0) return;
			__rtr_copy6orless(_pstr, tstr, _intToStringTableLengths[tval]);
		}
		else
		{
			largeUInt32ToString(tval);
		}
	}
	else
	{
		largeUInt64ToString(value);
	}
}

void rwfToJsonBase::int64ToString(RsslInt64 value)
{
	register char *tstr;
	register RsslUInt64 tval;

	if (value >= 0)
	{
		tval = value;
	}
	else
	{
		tval = -value;
		writeJsonString('-');
	}

	if (tval <= UINT_MAX)
	{
		register RsslUInt32 tttval = (RsslUInt32)tval;

		if (tttval <= RTR_RTMC_MAX_INT_TO_STR_TABLE)
		{
			tstr = _intToStringTable[tttval];
			if (verifyJsonMessageSize(_intToStringTableLengths[tttval]) == 0) return;
			__rtr_copy6orless( _pstr, tstr, _intToStringTableLengths[tttval]);
		}
		else
		{
			largeUInt32ToString(tttval);
		}
	}
	else
	{
		largeUInt64ToString(tval);
	}
}

void rwfToJsonBase::largeUInt32ToString(RsslUInt32 value, int hint, bool verifyBufferSize)
{
	RsslUInt32 tquo, trem, len;
	char *tstr;

	__rtr_u32div(value,4,tquo,trem);

	RTPRECONDITION(trem < 10000);

	if (hint > 0)
		hint -= 4;

	if (tquo < RTR_RTMC_MAX_INT_TO_STR_TABLE)
	{
		tstr = _intToStringTable[tquo];
		len = _intToStringTableLengths[tquo];

		if (len < (RsslUInt32)hint)
		{
			hint -= len;
			if (verifyBufferSize && verifyJsonMessageSize(hint) == 0) return;
			memset(_pstr, 0x30, hint);
			_pstr += hint;
		}

		if (verifyBufferSize && verifyJsonMessageSize(len) == 0) return;
		__rtr_copy6orless(_pstr, tstr, len);
	}
	else
	{
		largeUInt32ToString(tquo, hint, verifyBufferSize);
	}

	tstr = _intToStringTable[trem];
	len = _intToStringTableLengths[trem];

	if (len < 4)
	{
		if (verifyBufferSize && verifyJsonMessageSize(4 - len) == 0) return;
		__rtr_copy4orless(_pstr, "0000", 4 - len);
	}

	if (verifyBufferSize && verifyJsonMessageSize(len) == 0) return;
	__rtr_copy4orless(_pstr, tstr, len);
}

void rwfToJsonBase::largeUInt64ToString(RsslUInt64 value, int hint)
{
	RsslUInt32 len;
	RsslUInt64 tquo, trem;
	char *tstr;

	tquo = value / 10000;
	trem = value - (tquo * 10000);

	RTPRECONDITION(trem < 10000);

	if (hint > 0)
		hint -= 4;

	if (tquo < RTR_RTMC_MAX_INT_TO_STR_TABLE)
	{
		tstr = _intToStringTable[tquo];
		len = _intToStringTableLengths[tquo];

		if (len < (RsslUInt32)hint)
		{
			hint -= len;
			if (verifyJsonMessageSize(hint) == 0) return;
			memset(_pstr, 0x30, hint);
			_pstr += hint;
		}

		if (verifyJsonMessageSize(len) == 0) return;
		__rtr_copy6orless(_pstr, tstr, len);
	}
	else if (value <= UINT_MAX)
	{
		largeUInt32ToString((RsslUInt32)tquo, hint);
	}
	else
	{
		largeUInt64ToString(tquo, hint);
	}

	tstr = _intToStringTable[trem];
	len = _intToStringTableLengths[trem];

	if (len < 4)
	{
		if (verifyJsonMessageSize(4 - len) == 0) return;
		__rtr_copy4orless(_pstr, "0000", 4 - len);
	}

	if (verifyJsonMessageSize(len) == 0) return;
	__rtr_copy4orless(_pstr, tstr, len);
}

void rwfToJsonBase::setSolicitedFlag(bool solicitedFlag)
{
	if (*_solicitedFlagPtr)
		*_solicitedFlagPtr = solicitedFlag ? '1' : '0';
}

void rwfToJsonBase::setStreamingState(int streamingState)
{
	static char streamStateChars[] = {'0', '1', '2', '3', '4', '5'};

	if (*_streamingStatePtr && streamingState <= 5)
			*_streamingStatePtr = streamStateChars[streamingState];


}

const RsslBuffer* rwfToJsonBase::getAjaxMsg(RsslUInt32 streamId, bool solicited)
{
	char streamIdString[20];
	int streamIdStringLen = 0;
	char *msgStart = 0;
	char *pstrSave = _pstr;
	int len;

	*_pstr++ = ')';
	*_pstr++ = ';';
	*_pstr = 0;		// Just in case someone prints it, won't get copied

	len = (int)(_pstr - _buf - MAX_MSG_PREQUEL);

	_pstr = streamIdString;
	uInt32ToString(streamId);
	streamIdStringLen = (int)(_pstr - streamIdString);

	len += AJAX_FIXED_PREQUEL + streamIdStringLen;
	msgStart = _buf + MAX_MSG_PREQUEL - (AJAX_FIXED_PREQUEL + streamIdStringLen);
	_pstr = msgStart;
	_buffer.data = msgStart;
	_buffer.length = len;

	if (_msgClass == RSSL_MC_REFRESH)
		*_solicitedFlagPtr = solicited ? '1' : '0';

	*_pstr++ = _ajaxMsgType[_msgClass];
	*_pstr++ = '(';
	writeOb();
	writeVar('b', false);
	writeOb();
	writeVar('s', false);
	doSimpleMemCopy(_pstr,streamIdString,streamIdStringLen);
	_pstr = pstrSave;
	return &_buffer;
}


bool rwfToJsonBase::buildJsonTerminateStream(RJCBuffer &oMsg, int errorCode, RJCString &text)
{
	char *pstrSaved = _pstr;
	u_32 sizeSaved = _size;

	_pstr = oMsg.to_c(1);
	_size = oMsg.capacity();

	writeValue("t(");
	writeOb();
	writeCharDQ('e');
	int32ToString(errorCode);
	writeComma();
	writeCharDQ('t');

	*_pstr++ = '"';
	for (int i = text.lower(); i <= text.upper(); i++)
	{
		switch(text[i])
		{
		case '\"':
			*_pstr++ = '\\';
			*_pstr++ = '\"';
			break;
		case '\\':
			*_pstr++ = '\\';
			*_pstr++ = '\\';
			break;
		default:
			if (text[i] < ' ' || text[i] == 0x7F )
			{
				_pstr += sprintf(_pstr, "\\u%04x", text[i]);
			}
			else
				*_pstr++ = text[i];
			break;
		}

	}
	*_pstr++ = '"';

	writeOe();
	writeValue(");");

	oMsg.set_count((int)(_pstr - oMsg.to_c(1)));

	_pstr = pstrSaved;
	_size = sizeSaved;

	return true;
}

//////////////////////////////////////////////////////////////////////
// Entry point for RWF messages to be converted to JSON
// We intentionally omit the opening bracket and streamId.
// these will be added as the message is extracted.
//////////////////////////////////////////////////////////////////////
int rwfToJsonBase::convertMsg(RsslMsg &iMsg)
{
	//See if we have a big enough buffer. If not try to resize it.
	int reqLen = estimateJsonLength(iMsg.msgBase.encDataBody.length);
	if(reqLen  > _maxLen)
	{
		//If resize fails return failure.
		if( resize(reqLen) == 0 )
			return 0;

		reset();
	}
	_reset = false;
	RsslDecodeIterator iter;
	rsslClearDecodeIterator(&iter);
	rsslSetDecodeIteratorBuffer(&iter, &iMsg.msgBase.encDataBody);
	if (!processMsg(&iter, iMsg))
	{
		_error = 1;
		return 0;
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
//
// Container Processors
//
//////////////////////////////////////////////////////////////////////
///////////////////////////////////
//
// Opaque Container Processor
//
///////////////////////////////////
int rwfToJsonBase::processOpaque(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag = true)
{
	if (encDataBufPtr->length > 0)
		encodeBase64((unsigned char*)encDataBufPtr->data, encDataBufPtr->length);
	else
		writeNull();
	return 1;
}
///////////////////////////////////
//
// XML Container Processor
//
///////////////////////////////////
int rwfToJsonBase::processXml(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag = true)
{
	writeSafeString(encDataBufPtr->data, encDataBufPtr->length);
	return 1;
}
///////////////////////////////////
//
// ANSI Page Container Processor
//
///////////////////////////////////
int rwfToJsonBase::processAnsiPage(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag = true)
{
	return 0;
}
///////////////////////////////////
//
// Message Container Processor
//
///////////////////////////////////
int rwfToJsonBase::processMsg(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag = true)
{
	RsslMsg msg;
	RsslRet ret;
	rsslClearMsg(&msg);

	if ((ret = rsslDecodeMsg(iterPtr, &msg)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}
	return processMsg(iterPtr, msg, false);
}

//////////////////////////////////////////////////////////////////////
//
// Base Type Functions
//
//////////////////////////////////////////////////////////////////////
int rwfToJsonBase::processPrimitive(RsslDecodeIterator *iterPtr, int primitiveType)
{
	if (primitiveType < RSSL_DT_SET_PRIMITIVE_MAX
		&& _primitiveHandlers[primitiveType] )
		return ((this->*_primitiveHandlers[primitiveType])(iterPtr));
	return 0;
}

int rwfToJsonBase::processPrimitive(int primitiveType, RsslBuffer *bufPtr)
{
	RsslDecodeIterator localIter;
	if (primitiveType < RSSL_DT_SET_PRIMITIVE_MAX &&
		_primitiveHandlers[primitiveType] &&
		bufPtr)
	{
		rsslClearDecodeIterator(&localIter);
		rsslSetDecodeIteratorBuffer(&localIter, bufPtr);
		return ((this->*_primitiveHandlers[primitiveType])(&localIter));
	}
	return 0;
}
///////////////////////////////////
//
// Unknown
//
///////////////////////////////////

int rwfToJsonBase::processUnknown(RsslDecodeIterator *iterPtr)
{
	return 0;
}
///////////////////////////////////
//
// Integer
//
///////////////////////////////////
int rwfToJsonBase::processInteger(RsslDecodeIterator *iterPtr)
{
	RsslInt intVal;
	RsslRet retVal;

	if ((retVal = rsslDecodeInt(iterPtr, &intVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
		writeNull();
	else
		int64ToString(intVal);
	return 1;
}
///////////////////////////////////
//
// Unsigned Integer
//
///////////////////////////////////
int rwfToJsonBase::processUnsignedInteger(RsslDecodeIterator *iterPtr)
{
	RsslUInt uIntVal;
	RsslRet retVal;

	if ((retVal = rsslDecodeUInt(iterPtr, &uIntVal)) <  RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
		writeNull();
	else
		uInt64ToString(uIntVal);
	return 1;
}
///////////////////////////////////
//
// Float
//
///////////////////////////////////
int rwfToJsonBase::processFloat(RsslDecodeIterator *iterPtr)
{
	RsslFloat floatVal;
	RsslRet retVal;

	if ((retVal = rsslDecodeFloat(iterPtr, &floatVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
		writeNull();
	else
		floatToStr(floatVal);
	return 1;
}
///////////////////////////////////
//
// Double
//
///////////////////////////////////
int rwfToJsonBase::processDouble(RsslDecodeIterator *iterPtr)
{
	RsslDouble doubleVal;
	RsslRet retVal;

	if ((retVal = rsslDecodeDouble(iterPtr, &doubleVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
		writeNull();
	else
		doubleToStr(doubleVal);

	return 1;
}
///////////////////////////////////
//
// QOS
//
///////////////////////////////////
int rwfToJsonBase::processQOS(RsslDecodeIterator *iterPtr)
{
	RsslQos qosVal;
	RsslRet retVal;

	rsslClearQos(&qosVal);

	if ((retVal = rsslDecodeQos(iterPtr, &qosVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
		return 1;
	}
	else
	{
		return processQOS(&qosVal);
	}

}
///////////////////////////////////
//
// State
//
///////////////////////////////////
int rwfToJsonBase::processState(RsslDecodeIterator *iterPtr)
{
	RsslState stateVal;
	RsslRet retVal;

	rsslClearState(&stateVal);

	if ((retVal = rsslDecodeState(iterPtr, &stateVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
		return 1;
	}
	else
	{
		return processState(&stateVal);
	}
}
///////////////////////////////////
//
// Enumeration
//
///////////////////////////////////
int rwfToJsonBase::processEnumeration(RsslDecodeIterator *iterPtr)
{
	RsslRet retVal;
	RsslEnum enumVal;

	if ((retVal = rsslDecodeEnum(iterPtr, &enumVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
		return 1;
	}

	uInt32ToString(enumVal);

	return 1;
}

///////////////////////////////////
//
// Buffer
//
///////////////////////////////////
int rwfToJsonBase::processBuffer(RsslDecodeIterator *iterPtr)
{


	RsslBuffer buffer;
	RsslRet retVal;


	if ((retVal = rsslDecodeBuffer(iterPtr, &buffer)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
		writeNull();
	else
		encodeBase64((unsigned char*)buffer.data, buffer.length);



	return 1;
}
///////////////////////////////////
//
// ASCII String
//
///////////////////////////////////
int rwfToJsonBase::processAsciiString(RsslDecodeIterator *iterPtr)
{
	RsslRet retVal;
	RsslBuffer buffer;

	if ( (retVal = rsslDecodeBuffer(iterPtr, &buffer)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA || buffer.length  == 0)
	{
		writeNull();
	}
	else
	{
		writeSafeString(buffer.data, buffer.length);
	}
	return 1;
}
///////////////////////////////////
//
// UTF8 String
//
///////////////////////////////////
int rwfToJsonBase::processUTF8String(RsslDecodeIterator *iterPtr)
{
	RsslRet retVal;
	RsslBuffer buffer;

	if ( (retVal = rsslDecodeBuffer(iterPtr, &buffer)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA || buffer.length  == 0)
	{
		writeNull();
	}
	else
	{
		writeSafeString(buffer.data, buffer.length);
	}
	return 1;
}
///////////////////////////////////
//
// RMTES String
//
///////////////////////////////////
int rwfToJsonBase::processRMTESString(RsslDecodeIterator *iterPtr)
{
	RsslBuffer buffer;
	RsslRet retVal;

	if ((retVal = rsslDecodeBuffer(iterPtr, &buffer)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA || buffer.length  == 0)
	{
		writeNull();
	}
	else
	{
		rmtesToUtf8(buffer);
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
//
// Utility Functions
//
//////////////////////////////////////////////////////////////////////

void rwfToJsonBase::encodeBase64(const unsigned char *in, int length)
{

	static char b64EncodeTable[] = {'A','B','C','D','E','F','G','H',
					'I','J','K','L','M','N','O','P',
					'Q','R','S','T','U','V','W','X',
					'Y','Z','a','b','c','d','e','f',
					'g','h','i','j','k','l','m','n',
					'o','p','q','r','s','t','u','v',
					'w','x','y','z','0','1','2','3',
					'4','5','6','7','8','9','+','/'};

	char aChar;
	int bytesLeft = length;
	int numBytes = 0;
	unsigned char numLoops;

	if (verifyJsonMessageSize((length*3)/4 + 3) == 0) return; // Assumes worst case scenario

	*_pstr++ = '\"';
	while(bytesLeft)
	{
		numBytes = bytesLeft;
		switch( bytesLeft)
		{
		case 1:
			bytesLeft -= 1;
			numLoops = 2;
			break;
		case 2:
			bytesLeft -= 2;
			numLoops = 3;
			break;
		default:
			bytesLeft -= 3;
			numLoops = 4;
			break;
		}
		for (int i = 0; i < numLoops; i++)
		{
			switch (i)
			{
			case 0:
				aChar = in[0] >> 2;
				break;
			case 1:
			  	aChar = (in[0] & 0x03) << 4;
				if (numBytes > 1)
					aChar |= (in[1] >> 4) ;
				break;
			case 2:
				aChar = (in[1] & 0x0F) << 2;
				if (numBytes > 2)
					aChar |= (in[2] >> 6);
				break;
			case 3:
				aChar = in[2] & 0x3F;
				in += 3;	// May have more to encode
				break;

			}
			*_pstr++ = b64EncodeTable[aChar];
		}
	}

	// Pad with '=' characters to even 4 bytes
	if (numLoops < 4)
	{
		for (int i = 0; i < 4 - numLoops; i++)
			*_pstr++ = '=';
	}

	*_pstr++ = '\"';
}


void rwfToJsonBase::rmtesToUtf8(const RsslBuffer &buffer)
{
	int rmtesLen = buffer.length;
	int ucs2Len = 0;
	int utf8Len = 0;

	/* If an RMTES strings contains only displayable Ascii characters, skip conversion to UTF8. */
	RsslUInt32 i;
	for (i = 0; i < buffer.length; ++i)
	{
		if (buffer.data[i] < ' ' || buffer.data[i] == 0x7F)
			break;
	}

	if (i == buffer.length)
	{
		writeSafeString(buffer.data, buffer.length);
		return;
	}

	//In the case of brokerage characters (_r,_w,_t,_p,_u), each one
	//requires an additional ucs2 wide-character. To handle the extreme case of every
	//character is a brokerage character, ucs2Buf needs to be allocated
	//rmtesLen*2 wide-characters. Add 1 more wide-character for NULL termination
	//in _cutil rmtes to ucs2_ conversion routine

	int ucs2BufSz = rmtesLen*2 + 1;
	RsslRmtesCacheBuffer rmtesCacheBuf;
	RsslBuffer			rutf8Buff;

	if (ucs2BufSz >  _ucs2BufSz)
	{
		// Reallocate Buffer
		delete [] _ucs2Buf;
		_ucs2Buf = new RsslUInt16[ucs2BufSz];
		_ucs2BufSz = ucs2BufSz;
	}


	rmtesCacheBuf.data = buffer.data;
	rmtesCacheBuf.length = rmtesLen;
	rmtesCacheBuf.allocatedLength = rmtesLen;

	// +1 for the NULL byte added by the _cutil ucs2 to utf8_ conversion routine
	int  utf8BufSz = rmtesLen*MaxUTF8Bytes + 1;
	if ( utf8BufSz > _utf8BufSz )
	{
		// Reallocate buffer
		delete [] _utf8Buf;
		_utf8Buf = new char[utf8BufSz];
		_utf8BufSz = utf8BufSz;
	}

	rutf8Buff.data = _utf8Buf;
	rutf8Buff.length = _utf8BufSz;

	RsslRet retCode = rsslRMTESToUTF8( &rmtesCacheBuf, &rutf8Buff );

	while(retCode == RSSL_RET_BUFFER_TOO_SMALL)
	{
		delete [] _utf8Buf;
		_utf8BufSz += _utf8BufSz;
		_utf8Buf = new char[_utf8BufSz];

		rutf8Buff.data = _utf8Buf;
		rutf8Buff.length = _utf8BufSz;
		retCode = rsslRMTESToUTF8( &rmtesCacheBuf, &rutf8Buff );
	}

	if (retCode == RSSL_RET_SUCCESS && rutf8Buff.length > 0)
	{
		char* fromPtr = rutf8Buff.data;
		int i = 0;

		if (verifyJsonMessageSize(rutf8Buff.length*3+2) == 0) return;
		
		*_pstr++ = '\"';
		for(i = 0; i < (int)rutf8Buff.length; i++)
		{
			if ( !(*fromPtr & 0x80))
			{
				switch (*fromPtr)
				{
				case '\"':
					*_pstr++ = '\\';
					*_pstr++ = '\"';
					break;
				case '\\':
					*_pstr++ = '\\';
					*_pstr++ = '\\';
					break;
				default:
					if (*fromPtr < ' ' || *fromPtr == 0x7F )
					{
						_pstr += sprintf(_pstr, "\\u%04x", *fromPtr);
					}
					else
						*_pstr++ = *fromPtr;
					break;
				}
			}
			else
				*_pstr++ = *fromPtr;
			fromPtr++;
		}
		*_pstr++ = '\"';
	}
	else
		writeNull(); /* Write a null, same as if original Rmtes string was blank. */
	// }
	// else
		// writeNull(); /* Write a null, same as if original Rmtes string was blank. */
}

void rwfToJsonBase::floatToStr(RsslFloat value)
{
	char buf[50];
	snprintf(buf, sizeof(buf), "%.6G", value);

	bool isNeg = false;
	char* ptr = buf;
	if (*ptr == '-')
	{
		isNeg = true;
		ptr++;
	}

	//Standardize Inf, -Inf and NaN formats
#if defined(_MSC_VER) && (_MSC_VER == 1700)
	// VS2012 expected 1.#INF0000 & 1.#QNAN format
	switch(*(ptr + 3))
	{
		case 'I':
			if (isNeg)
				writeString("-Inf");
			else
				writeString("Inf");
			break;
		case 'Q':
			writeString("NaN");
			break;
		default: //For everything else
			writeValue(buf);
			break;
	}

#elif defined(_MSC_VER) && (_MSC_VER == 1800)
	// VS2013 expected 1.#INF0000 & 1.#IND format
	switch(*(ptr + 5))
	{
	case 'F':
		if (isNeg)
			writeString("-Inf");
		else
			writeString("Inf");
		break;
	case 'D':
		writeString("NaN");
		break;
	default: //For everything else
		writeValue(buf);
		break;
}

#else
	switch(*ptr)
	{
		case 'I':
			if (isNeg)
				writeString("-Inf");
			else
				writeString("Inf");
			break;
		case 'N':
			writeString("NaN");
			break;
		default: //For everything else
			writeValue(buf);
			break;
	}
#endif
}

void rwfToJsonBase::doubleToStr(RsslDouble value)
{
	char buf[50];
	snprintf(buf, sizeof(buf), "%.15G", value);

	bool isNeg = false;
	char* ptr = buf;
	if (*ptr == '-')
	{
		isNeg = true;
		ptr++;
	}

	//Standardize Inf, -Inf and NaN formats
#if defined(_MSC_VER) && (_MSC_VER == 1700)
	// VS2012 expected 1.#INF0000 & 1.#QNAN format
	switch(*(ptr + 3))
	{
		case 'I':
			if (isNeg)
				writeString("-Inf");
			else
				writeString("Inf");
			break;
		case 'Q':
			writeString("NaN");
			break;
		default: //For everything else
			writeValue(buf);
			break;
	}
#elif defined(_MSC_VER) && (_MSC_VER == 1800)
	// VS2013 expected 1.#INF0000 & 1.#IND format
	switch(*(ptr + 5))
	{
	case 'F':
		if (isNeg)
			writeString("-Inf");
		else
			writeString("Inf");
		break;
	case 'D':
		writeString("NaN");
		break;
	default: //For everything else
		writeValue(buf);
		break;
	}
#else
	switch(*ptr)
	{
		case 'I':
			if (isNeg)
				writeString("-Inf");
			else
				writeString("Inf");
			break;
		case 'N':
			writeString("NaN");
			break;
		default: //For everything else
			writeValue(buf);
			break;
	}
#endif
}

