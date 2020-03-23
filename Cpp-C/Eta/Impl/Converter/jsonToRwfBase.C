/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */


#include <limits.h>
#include <math.h>

#include <stdlib.h>

#include "rtr/rtratoi.h"

#include "rtr/jsonToRwfBase.h"
#include "rtr/jsonSimpleDefs.h"

#include <float.h>
#include <ctype.h>

/* Suppress warning C4756: overflow in constant arithmetic that occurs only on VS2013 */
#if defined(WIN32) &&  _MSC_VER == 1800
#pragma warning( disable : 4056)
#endif

#ifdef WIN32
	#ifndef INFINITY
		#define INFINITY HUGE_VAL
		#ifndef NEG_INFINITY
			#define NEG_INFINITY -HUGE_VAL
		#endif
	#else
		#ifndef NEG_INFINITY
			#define NEG_INFINITY -INFINITY		
		#endif
	#endif
	#ifndef NAN
        static const unsigned long __nan[2] = {0xffffffff, 0x7fffffff};
        #define NAN (*(const double *) __nan)
    #endif
#else
	#define NEG_INFINITY -INFINITY
#endif


const int jsonToRwfBase::_posExponentTable[] = {14,15,16,17,18,19,20,21,22,23,23};
const int jsonToRwfBase::_negExponentTable[] = {0,13,12,11,10,9,8,7,6,5,4,3,2,1,0};

//////////////////////////////////////////////////////////////////////
// Primitive Handlers
//////////////////////////////////////////////////////////////////////

const jsonToRwfBase::primitiveHandlerPtr jsonToRwfBase::_primitiveHandlers[] =
	{
		&jsonToRwfBase::processUnknown,
		0, // Reserved
		0, // Reserved
		&jsonToRwfBase::processInteger,
		&jsonToRwfBase::processUnsignedInteger,
		&jsonToRwfBase::processFloat,
		&jsonToRwfBase::processDouble,
		0, // Reserved
		&jsonToRwfBase::processReal,
		&jsonToRwfBase::processDate,
		&jsonToRwfBase::processTime,
		&jsonToRwfBase::processDateTime,
		&jsonToRwfBase::processQOS,
		&jsonToRwfBase::processState,
		&jsonToRwfBase::processEnumeration,
		&jsonToRwfBase::processArray,
		&jsonToRwfBase::processBuffer,
		&jsonToRwfBase::processAsciiString,
		&jsonToRwfBase::processUTF8String,
		&jsonToRwfBase::processRMTESString,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 20 - 63 Reserved
		&jsonToRwfBase::processInteger,	// 64
		&jsonToRwfBase::processUnsignedInteger,
		&jsonToRwfBase::processInteger,
		&jsonToRwfBase::processUnsignedInteger,
		&jsonToRwfBase::processInteger,
		&jsonToRwfBase::processUnsignedInteger,
		&jsonToRwfBase::processInteger,
		&jsonToRwfBase::processUnsignedInteger,
		&jsonToRwfBase::processFloat,
		&jsonToRwfBase::processDouble,
		&jsonToRwfBase::processReal,
		&jsonToRwfBase::processReal,
		&jsonToRwfBase::processDate,
		&jsonToRwfBase::processTime,
		&jsonToRwfBase::processTime,
		&jsonToRwfBase::processDateTime,
		&jsonToRwfBase::processDateTime	// 80
	};

const int jsonToRwfBase::_primitiveEncodeTypeTable[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	3,4,3,4,3,4,3,4,5,6,8,8,9,10,10,11,
	11};

//////////////////////////////////////////////////////////////////////
// Container Handlers
//////////////////////////////////////////////////////////////////////

const jsonToRwfBase::containerHandlerFuncPtr jsonToRwfBase::_containerHandlers[] =
	{
		0,	// Reserved
		0,	// Reserved
		&jsonToRwfBase::processOpaque,
		&jsonToRwfBase::processXML,
		&jsonToRwfBase::processFieldList,
		&jsonToRwfBase::processElementList,
		&jsonToRwfBase::processAnsiPage,
		&jsonToRwfBase::processFilterList,
		&jsonToRwfBase::processVector,
		&jsonToRwfBase::processMap,
		&jsonToRwfBase::processSeries,
		0,	// Reserved
		0,	// Reserved
		&jsonToRwfBase::processMsg,
		&jsonToRwfBase::processJson
	};

//////////////////////////////////////////////////////////////////////
// Message Handlers
//////////////////////////////////////////////////////////////////////
const jsonToRwfBase::msgHandlerFuncPtr jsonToRwfBase::_msgHandlers[] =
	{
		0, // Not Valid
		&jsonToRwfBase::processRequestMsg,
		&jsonToRwfBase::processRefreshMsg,
		&jsonToRwfBase::processStatusMsg,
		&jsonToRwfBase::processUpdateMsg,
		&jsonToRwfBase::processCloseMsg,
		&jsonToRwfBase::processAckMsg,
		&jsonToRwfBase::processGenericMsg,
		&jsonToRwfBase::processPostMsg
	};

jsonToRwfBase::jsonToRwfBase(int bufSize, unsigned int flags, int numTokens, int incSize) :
	_rsslServiceNameToIdCallback(0),
	_closure(0),
	_error(false),
	_numTokens(numTokens),
	_incSize(incSize),
	_nullUserName(0),
	_errorCode(NO_ERROR_CODE),
	_errorToken(0),
	_expectedTokenType(JSMN_PRIMITIVE),
	_jsmnError(JSMN_SUCCESS),
	_rsslRet(0),
	_errorLineNum(0),
	_errorTextLen(0),
	_bufSize(bufSize),
	_flags(flags),
	_dictionaryList(0),
	_dictionaryCount(0)
{
	rsslBlankTime(&_timeVar);

	_outBuf.data = 0;
	_outBuf.length = 0;
	_errorText.data = 0;
	if ((_outBuf.data = (char*)malloc(bufSize)) == 0)
	{
		_error = true;
		return;
	}
	_outBuf.length = _bufSize;
	if ((_tokens = (jsmntok_t*)malloc(numTokens * sizeof(jsmntok_t))) == NULL)
	{
		_error = true;
		return;
	}

	if ((_errorText.data = (char*)malloc(ERROR_TEXT_MAX)) == 0)
	{
		_error = true;
		return;
	}
	_errorTextLen = ERROR_TEXT_MAX;

	memset(_tokens, 0, _numTokens * sizeof(jsmntok_t));
}

jsonToRwfBase::~jsonToRwfBase()
{
	if(_outBuf.data)
		free(_outBuf.data);
	if(_tokens)
		free(_tokens);
	if(_errorText.data)
		free(_errorText.data);

}

void jsonToRwfBase::reset()
{
	_outBuf.length = 0;
	_error = false;
	_errorFile = 0;
	_errorCode = NO_ERROR_CODE;
	_errorToken = 0;
	_expectedTokenType = JSMN_PRIMITIVE;
	_rsslRet = 0;
	_jsmnError = JSMN_SUCCESS;
	_errorLineNum = 0;
	_errorTextLen = 0;
}

RsslBuffer* jsonToRwfBase::errorText()
{
	if (_error == false)
		return 0;

	//memset(_errorText.data,'0',_errorTextLen);
	_errorText.length = 0;

	switch (_errorCode)
	{
		case MEM_ALLOC_FAILURE:
			_errorText.length = snprintf(_errorText.data,
										ERROR_TEXT_MAX,
										"JSON Converter memory allocation Error: %s Line %d",
										_errorFile, _errorLineNum);
			break;
		case JSMN_PARSE_ERROR:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON parser error: %d", _jsmnError);
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " %s Line %d.", _errorFile, _errorLineNum);
			break;
		case INVALID_TOKEN_TYPE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Converter Token Type error: ");
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Expected ");
			switch ( _expectedTokenType)
			{
				case JSMN_PRIMITIVE :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'PRIMITIVE' ");
					break;
				case JSMN_OBJECT :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'OBJECT' ");
					break;
				case JSMN_ARRAY :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'ARRAY' ");
					break;
				case JSMN_STRING :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'STRING' ");
					break;
			}
			if (_errorParentKey.data)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "for key '%s'", _errorParentKey.data);
			}
			if (_errorToken)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received ");
				switch ( _errorToken->type)
				{
					case JSMN_PRIMITIVE :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'PRIMITIVE' ");
						break;
					case JSMN_OBJECT :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'OBJECT' ");
						break;
					case JSMN_ARRAY :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'ARRAY' ");
						break;
					case JSMN_STRING :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'STRING' ");
						break;
				}
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Msg Offset: %d ", _errorToken->start);
			}
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " %s Line %d.", _errorFile, _errorLineNum);
			break;
		case UNEXPECTED_VALUE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Unexpected Value.");
			if (_errorToken)
			{
				int tokenLength = _errorToken->end - _errorToken->start;
				char* singleToken = (char*)malloc((size_t)tokenLength * sizeof(char)+1);
				memset(singleToken, 0, tokenLength+1);
				memcpy(singleToken, &_jsonMsg[_errorToken->start], tokenLength);
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received '%s' at", singleToken);
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Msg Offset: %d ", _errorToken->start);
				free(singleToken);
			}
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " %s Line %d.", _errorFile, _errorLineNum);
			break;
		case INVALID_PRIMITIVE_TYPE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON invalid primitive type.");
			if (_errorToken)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received ");
				switch ( _errorToken->type)
				{
					case JSMN_PRIMITIVE :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'PRIMITIVE' ");
						break;
					case JSMN_OBJECT :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'OBJECT' ");
						break;
					case JSMN_ARRAY :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'ARRAY' ");
						break;
					case JSMN_STRING :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'STRING' ");
						break;
				}
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Msg Offset: %d ", _errorToken->start);
			}
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " %s Line %d.", _errorFile, _errorLineNum);
			break;
		case INVALID_CONTAINER_TYPE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON invalid container type.");
			if (_errorToken)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received ");
				switch ( _errorToken->type)
				{
					case JSMN_PRIMITIVE :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'PRIMITIVE' ");
						break;
					case JSMN_OBJECT :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'OBJECT' ");
						break;
					case JSMN_ARRAY :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'ARRAY' ");
						break;
					case JSMN_STRING :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " 'STRING' ");
						break;
				}
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Msg Offset: %d ", _errorToken->start);
			}
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " %s Line %d.", _errorFile, _errorLineNum);
			break;
		case SET_DEFINITION_ERROR:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Set Definition error.");
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " %s Line %d.", _errorFile, _errorLineNum);
			break;
		case RSSL_ENCODE_ERROR:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON RSSL Conversion Error. RSSL error code : %d",_rsslRet);
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " %s Line %d.", _errorFile, _errorLineNum);
			break;
		case RSSL_DICT_NOT_INIT:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON RSSL Conversion Error. RsslDataDictionary is not initialized with error code: %d", _rsslRet);
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " %s Line %d.", _errorFile, _errorLineNum);
			break;
		case NO_MSG_BASE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON message with no message Base.");
			break;
		case UNSUPPORTED_MSG_TYPE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "Unsupported Message Type");
			break;
		case NO_ERROR_CODE:
		default:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "No error code.");
			break;
		}
		return &_errorText;
}

bool jsonToRwfBase::encodeRsslMsg(RsslMsg *rsslMsgPtr, jsmntok_t ** const msgTok, jsmntok_t *dataTokPtr, jsmntok_t *attribTokPtr, jsmntok_t *reqKeyattrib)
{
	if (rsslMsgPtr->msgBase.msgClass >= RSSL_MC_REQUEST &&
		rsslMsgPtr->msgBase.msgClass <= RSSL_MC_POST  &&
		(this->*_msgHandlers[rsslMsgPtr->msgBase.msgClass])(msgTok, rsslMsgPtr, &dataTokPtr, &attribTokPtr, &reqKeyattrib))
	{
		if (dataTokPtr == 0 && attribTokPtr == 0)
		{
			if ((_rsslRet = rsslEncodeMsg(&_iter, rsslMsgPtr)) < RSSL_RET_SUCCESS)
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
			_outBuf.length = rsslGetEncodedBufferLength(&_iter);
			return true;
		}
		if ((_rsslRet = rsslEncodeMsgInit(&_iter, rsslMsgPtr, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		if (_rsslRet == RSSL_RET_ENCODE_MSG_KEY_OPAQUE)
		{
			if (processContainer(&attribTokPtr, rsslMsgPtr->msgBase.msgKey.attribContainerType, 0) == false)
			{
				return false;
			}

			if ((_rsslRet = rsslEncodeMsgKeyAttribComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
		}

		if (_rsslRet == RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB)
		{
			// need to determine dataFormat from specific message since it's not in msg Base
			// Only valid for some message types

			RsslUInt8 reqKeyFormat;
			switch (rsslMsgPtr->msgBase.msgClass)
			{
			case RSSL_MC_REFRESH:
				reqKeyFormat = rsslMsgPtr->refreshMsg.reqMsgKey.attribContainerType;
				break;

			case RSSL_MC_STATUS:
				reqKeyFormat = rsslMsgPtr->statusMsg.reqMsgKey.attribContainerType;
				break;

			case RSSL_MC_GENERIC:
				reqKeyFormat = rsslMsgPtr->genericMsg.reqMsgKey.attribContainerType;
				break;

			default:
				// Invalid return failure
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}

			if (processContainer(&attribTokPtr, reqKeyFormat, 0) == false)
			{
				return false;
			}

			if ((_rsslRet = rsslEncodeMsgReqKeyAttribComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
		}

		if (_rsslRet ==  RSSL_RET_ENCODE_CONTAINER)
		{
			if (encodeMsgPayload(rsslMsgPtr, dataTokPtr) == false)
				return false;

			if ((_rsslRet = rsslEncodeMsgComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
			_outBuf.length = rsslGetEncodedBufferLength(&_iter);
		}
		return true;
	}
	return false;
}

bool jsonToRwfBase::encodeMsgPayload(RsslMsg *rsslMsgPtr, jsmntok_t *dataTokPtr)
{
	int msgLen = rsslGetEncodedBufferLength(&_iter);
	rsslMsgPtr->msgBase.encDataBody.data = _outBuf.data + msgLen;

	if (dataTokPtr)
	{
		if (processContainer(&dataTokPtr, rsslMsgPtr->msgBase.containerType, 0) == false)
		{
			rsslClearBuffer(&rsslMsgPtr->msgBase.encDataBody);
			return false;
		}
		rsslMsgPtr->msgBase.encDataBody.length = rsslGetEncodedBufferLength(&_iter) - msgLen ;
	}
	return true;
}

int jsonToRwfBase::parseJsonBuffer(const RsslBuffer *bufPtr, int offset)
{
	jsmnerr_t ret;
	jsmn_parser jsmnParser;

	if (bufPtr && bufPtr->data && bufPtr->length > 0)
	{
	 	_jsonMsg = bufPtr->data + offset;
		while (true)
		{
			jsmn_init(&jsmnParser);
			ret = jsmn_parse(&jsmnParser, _jsonMsg,  bufPtr->length, _tokens, _numTokens);
			if ( ret < JSMN_SUCCESS)
			{
				if ( ret == JSMN_ERROR_NOMEM )
				{
				  	free(_tokens);
					_numTokens += _incSize;
					if ((_tokens = (jsmntok_t*)malloc(_numTokens * sizeof(jsmntok_t))) == NULL)
					{
						_error = true;

						error(MEM_ALLOC_FAILURE, __LINE__, __FILE__);
						return -1;
					}
					continue;
				}
				else
				{
					error(JSMN_PARSE_ERROR, __LINE__, __FILE__);
					_jsmnError = ret;
					_error = true;
					return ret;
				}
			}
			else
			{
				_tokensEndPtr = _tokens + jsmnParser.toknext;

				/* If the root element is an array, the messages are in each element of the array.
				 * Move to the first message. */
				if (_tokens->type == JSMN_ARRAY)
					_curMsgTok = _tokens + 1;
				else
					_curMsgTok = _tokens;


				return RSSL_RET_SUCCESS;
			}
		}
	}
	return -1;
}

int jsonToRwfBase::decodeJsonMsg(RsslJsonMsg &jsonMsg)
{
	if (_curMsgTok == _tokensEndPtr)
		return 0;

	reset();

	rsslClearJsonMsg(&jsonMsg);
	rsslClearEncodeIterator(&_iter);
	_outBuf.length = _bufSize;
	rsslSetEncodeIteratorBuffer(&_iter, &_outBuf);
	jsonMsg.msgBase.jsonMsgBuffer.data = &_jsonMsg[_curMsgTok->start];
	jsonMsg.msgBase.jsonMsgBuffer.length = _curMsgTok->end - _curMsgTok->start;
	int tokenEnd = _curMsgTok->end;
	if (processMessage(&_curMsgTok, &jsonMsg))
	{
		_outBuf.length = rsslGetEncodedBufferLength(&_iter);
		return tokenEnd;
	}
	_error = true;
	return -1;
}

bool jsonToRwfBase::processContainer(jsmntok_t ** const tokPtr,
									 RsslUInt8 containerType,
									 void* setDb)
{
	if (containerType > RSSL_DT_NO_DATA &&
		containerType <= RSSL_DT_CONTAINER_TYPE_MAX &&
		_containerHandlers[containerType - RSSL_DT_CONTAINER_TYPE_MIN])
	{
		return (this->*_containerHandlers[containerType - RSSL_DT_CONTAINER_TYPE_MIN])(tokPtr, setDb);
	}

	error(INVALID_CONTAINER_TYPE, __LINE__, __FILE__);
	_errorToken = *tokPtr;
	return false;
}


bool jsonToRwfBase::processPrimitive(int primitiveType,
									 jsmntok_t ** const tokPtr,
									 RsslBuffer ** const bufPtr,
									 void** const  ptrVoidPtr)
{
	if (primitiveType < RSSL_DT_SET_PRIMITIVE_MAX && _primitiveHandlers[primitiveType] )
		return ((this->*_primitiveHandlers[primitiveType])(tokPtr,bufPtr, ptrVoidPtr));


	error(INVALID_PRIMITIVE_TYPE, __LINE__, __FILE__);
	_errorToken = *tokPtr;
	return false;
}

bool jsonToRwfBase::processOpaque(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslBuffer opaqueBuffer = RSSL_INIT_BUFFER;
	RsslBuffer *opaqueBufPtr = &opaqueBuffer;
	RsslBuffer buffer = RSSL_INIT_BUFFER;
	
	if ((*tokPtr)->type != JSMN_STRING)
	{
		unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_OPAQUE);
		return false;
	}

	if (processBuffer(tokPtr, &opaqueBufPtr, 0))
	{
		_rsslRet = rsslEncodeNonRWFDataTypeInit(&_iter, &buffer);
		if ( opaqueBuffer.length > buffer.length)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
		memcpy(buffer.data, opaqueBuffer.data, opaqueBuffer.length);
		buffer.length = opaqueBuffer.length;

		if ((_rsslRet = rsslEncodeNonRWFDataTypeComplete(&_iter, &buffer, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
		return true;
	}
	return false;
}

bool jsonToRwfBase::processXML(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslBuffer buffer = RSSL_INIT_BUFFER;
	RsslBuffer xmlBuffer = RSSL_INIT_BUFFER;
	RsslBuffer *xmlBufPtr = &xmlBuffer;
	
	if ((*tokPtr)->type != JSMN_STRING)
	{
		unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_XML);
		return false;
	}

	if (processAsciiString(tokPtr, &xmlBufPtr, 0))
	{
		xmlBuffer.data[xmlBuffer.length] = 0;

		if (( _rsslRet = rsslEncodeBuffer(&_iter, &buffer)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
		_rsslRet = rsslEncodeNonRWFDataTypeInit(&_iter, &buffer);
		if ( xmlBuffer.length > buffer.length)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
		memcpy(buffer.data, xmlBuffer.data, xmlBuffer.length);
		buffer.length = xmlBuffer.length;

		if ((_rsslRet = rsslEncodeNonRWFDataTypeComplete(&_iter, &buffer, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
		return true;
	}
	return false;
}

bool jsonToRwfBase::processAnsiPage(jsmntok_t ** const tokPtr, void* setDb)
{
	return false;
}

bool jsonToRwfBase::processMsg(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslJsonMsg jsonMsg;
	rsslClearJsonMsg(&jsonMsg);

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_MESSAGE);
		return false;
	}
	if (!processMessage(tokPtr, &jsonMsg))
		return false;

	if (jsonMsg.msgBase.msgClass != RSSL_JSON_MC_RSSL_MSG)
	{
		/* RSSL containers cannot contain non-RSSL messages. */
		error(INVALID_CONTAINER_TYPE, __LINE__, __FILE__);
		return false;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////
// Primitives
//////////////////////////////////////////////////////////////////////
bool jsonToRwfBase::processUnknown(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	return processBuffer(tokPtr, ptrBufPtr, ptrVoidPtr);
}

bool jsonToRwfBase::processInteger(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	if ((*tokPtr)->type != JSMN_PRIMITIVE)
	{
		unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	if (_jsonMsg[(*tokPtr)->start] != 'n')
	{
		_intVar = rtr_atoll_size(&_jsonMsg[(*tokPtr)->start],
					&_jsonMsg[(*tokPtr)->end]);

		*ptrVoidPtr = &_intVar;
		*ptrBufPtr = 0;
	}
	else
	{
		*ptrVoidPtr = 0;
  		_bufVar.length = 0;
		_bufVar.data = 0;
		*ptrBufPtr = &_bufVar;
	}
	(*tokPtr)++;
	return true;
}

bool jsonToRwfBase::processUnsignedInteger(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	if ((*tokPtr)->type != JSMN_PRIMITIVE)
	{
		unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	if (_jsonMsg[(*tokPtr)->start] != 'n')
	{
		_uintVar = rtr_atoull_size(&_jsonMsg[(*tokPtr)->start],
					 &_jsonMsg[(*tokPtr)->end]);

		*ptrVoidPtr = &_uintVar;
		*ptrBufPtr = 0;
	}
	else
	{
		*ptrVoidPtr = 0;
  		_bufVar.length = 0;
		_bufVar.data = 0;
		*ptrBufPtr = &_bufVar;
	}

	(*tokPtr)++;
	return true;
}

bool jsonToRwfBase::processFloat(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{

	switch ((*tokPtr)->type)
	{
		case JSMN_STRING:
			if (compareStrings(*tokPtr, JSON_INFINITY))
			{
					_floatVar = INFINITY;
					*ptrVoidPtr = &_floatVar;
					*ptrBufPtr = 0;
					(*tokPtr)++;
					return true;
			}
			else if (compareStrings(*tokPtr, JSON_NEG_INFINITY))
			{
					_floatVar = -INFINITY;
					*ptrVoidPtr = &_floatVar;
					*ptrBufPtr = 0;
					(*tokPtr)++;
					return true;
			}
			else if (compareStrings(*tokPtr, JSON_NAN))
			{
					_floatVar = NAN;
					*ptrVoidPtr = &_floatVar;
					*ptrBufPtr = 0;
					(*tokPtr)++;
					return true;
			}

			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;

		case JSMN_PRIMITIVE:
		{
			if (_jsonMsg[(*tokPtr)->start] != 'n')
			{
#if defined(WIN32) &&  _MSC_VER == 1700
				_floatVar = (float)strtod(&_jsonMsg[(*tokPtr)->start], NULL);
#else
				_floatVar = strtof(&_jsonMsg[(*tokPtr)->start], NULL);
#endif

				*ptrVoidPtr = &_floatVar;
				*ptrBufPtr = 0;
			}
			else
			{
				*ptrVoidPtr = 0;
				_bufVar.length = 0;
				_bufVar.data = 0;
				*ptrBufPtr = &_bufVar;
			}
			(*tokPtr)++;
			break;
		}

		default:
			unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
			return false;
	}

	return true;
}

bool jsonToRwfBase::processDouble(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{

	switch ((*tokPtr)->type)
	{
		case JSMN_STRING:
			if (compareStrings(*tokPtr, JSON_INFINITY))
			{
					_doubleVar = INFINITY;
					*ptrVoidPtr = &_doubleVar;
					*ptrBufPtr = 0;
					(*tokPtr)++;
					return true;
			}
			else if (compareStrings(*tokPtr, JSON_NEG_INFINITY))
			{
					_doubleVar = -INFINITY;
					*ptrVoidPtr = &_doubleVar;
					*ptrBufPtr = 0;
					(*tokPtr)++;
					return true;
			}
			else if (compareStrings(*tokPtr, JSON_NAN))
			{
					_doubleVar = NAN;
					*ptrVoidPtr = &_doubleVar;
					*ptrBufPtr = 0;
					(*tokPtr)++;
					return true;
			}

			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;

		case JSMN_PRIMITIVE:
		{
			if (_jsonMsg[(*tokPtr)->start] != 'n')
			{
				_doubleVar = strtod(&_jsonMsg[(*tokPtr)->start], NULL);

				*ptrVoidPtr = &_doubleVar;
				*ptrBufPtr = 0;
			}
			else
			{
				*ptrVoidPtr = 0;
				_bufVar.length = 0;
				_bufVar.data = 0;
				*ptrBufPtr = &_bufVar;
			}
			(*tokPtr)++;
			break;
		}

		default:
			unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
			return false;
	}

	return true;
}

bool jsonToRwfBase::processQOS(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	*ptrBufPtr = 0;
	*ptrVoidPtr = 0;

	switch ((*tokPtr)->type)
	{
	case JSMN_PRIMITIVE:
	{
		if (_jsonMsg[(*tokPtr)->start] != 'n')
		{
			unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_QOS);
			(*tokPtr)++;
			return false;
		}
  		_bufVar.length = 0;
		_bufVar.data = 0;
		*ptrBufPtr = &_bufVar;
		(*tokPtr)++;
		return true;
	}
	case JSMN_OBJECT:
	{
		*ptrVoidPtr = &_qosVar;
		rsslClearQos(&_qosVar);
		return populateQos(tokPtr, &_qosVar);
	}
	default:
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_QOS);
		return false;
	}
}

bool jsonToRwfBase::processState(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	*ptrBufPtr = 0;
	*ptrVoidPtr = 0;

	switch ((*tokPtr)->type)
	{
	case JSMN_PRIMITIVE:
	{
		if (_jsonMsg[(*tokPtr)->start] != 'n')
		{
			unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_STATE);
			(*tokPtr)++;
			return false;
		}
  		_bufVar.length = 0;
		_bufVar.data = 0;
		*ptrBufPtr = &_bufVar;
		(*tokPtr)++;
		return true;
	}
	case JSMN_OBJECT:
	{
		*ptrVoidPtr = &_stateVar;
		rsslClearState(&_stateVar);
		return populateState(tokPtr, &_stateVar);
	}
	default:
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_STATE);
		return false;
	}
}

bool jsonToRwfBase::processEnumeration(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	return processInteger(tokPtr, ptrBufPtr, ptrVoidPtr);
}

bool jsonToRwfBase::processBuffer(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	// RFC4648 Compliant Base64 Encoder
	// Not compatible with IDN (MarketFeed) encoder/decoder

  static char b64DecodeTable[] = { -1, -1, -1, -1, -1, -1, -1, -1, /* 0 - 7 */
									-1, -1, -1, -1, -1, -1, -1, -1,	/* 8 - 15 */
									-1, -1, -1, -1, -1, -1, -1, -1, /* 16 - 23 */
									-1, -1, -1, -1, -1, -1, -1, -1, /* 24 - 31 */
									-1, -1, -1, -1, -1, -1, -1, -1, /* 32 - 39 */
									-1, -1, -1, 62, -1, -1, -1, 63, /* 40 - 47 */
									52, 53, 54, 55, 56, 57, 58, 59, /* 48 - 53 */
									60, 61, -1, -1, -1, -1, -1, -1, /* 56 - 63 */
									-1,  0,  1,  2,  3,  4,  5,  6, /* 64 - 71 */
									 7,  8,  9, 10, 11, 12, 13, 14, /* 72 - 79 */
									15, 16, 17, 18, 19, 20, 21, 22, /* 80 - 87 */
									23, 24, 25, -1, -1, -1, -1, -1, /* 88 - 95 */
									-1, 26, 27, 28, 29, 30, 31, 32, /* 96 - 103 */
									33, 34, 35, 36, 37, 38, 39, 40, /* 104 - 111 */
									41, 42, 43, 44, 45, 46, 47, 48, /* 112 - 119 */
									49, 50, 51}; /* 120 - 122 */


	(*ptrBufPtr)->length = 0;
	(*ptrBufPtr)->data = &_jsonMsg[(*tokPtr)->start];
	unsigned char *out = (unsigned char*)&_jsonMsg[(*tokPtr)->start];
	unsigned char *in = (unsigned char*)&_jsonMsg[(*tokPtr)->start];

	int encodedLength = (*tokPtr)->end - (*tokPtr)->start;

	if ((*tokPtr)->type == JSMN_PRIMITIVE)
	{
		if (_jsonMsg[(*tokPtr)->start] == 'n')
		{
			*ptrVoidPtr = 0;
			_bufVar.length = 0;
			_bufVar.data = 0;
			*ptrBufPtr = &_bufVar;
			(*tokPtr)++;
			return true;
		}
		unexpectedParameter(*tokPtr, __LINE__, __FILE__);
		return false;
	}

	if ((*tokPtr)->type != JSMN_STRING)
		return false;

	if (encodedLength == 0)
	{
		(*ptrBufPtr)->data = 0;
		return true;
	}

	// Decoded buffer will ALWAYS be smaller than the encoded string
	// So we'll use the memory the string was in for decoding to.

	int byte = 1;	// WHich byte are we decoding 1-3
	int i = 0;	// Input index
	int o = 0;	// Output index

	while( (i < encodedLength) && (in[i+1] != '='))
	{
		if ( (in[i+1] > 122) || (b64DecodeTable[in[i+1]] == -1))
		{
			/* Invalid character in compressed string
			 */
			return false;
		}
		switch (byte)
		{
		case 1:
			if ( (in[i] > 122) || (b64DecodeTable[in[i]] == -1))
			{
				/* Invalid character in compressed string
				 */
				return false;
			}
			in[i] = b64DecodeTable[in[i]];
			in[i+1] = b64DecodeTable[in[i+1]];
			out[o] = (unsigned char ) (in[i] << 2 | in[i+1] >> 4);
			byte = 2;
			i++;
			break;
		case 2:
			in[i+1] = b64DecodeTable[in[i+1]];
			out[o] = (unsigned char ) (in[i] << 4 | in[i+1] >> 2);
			byte = 3;
			i++;
			break;
		case 3:
			in[i+1] = b64DecodeTable[in[i+1]];
			out[o] = (unsigned char ) (in[i] << 6 | in[i+1]);
			byte = 1;
			i+=2;
			break;
		}
		o++;
		(*ptrBufPtr)->length++;
	}
	(*tokPtr)++;
	return true;
}

bool jsonToRwfBase::processAsciiString(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	if ((*tokPtr)->type == JSMN_PRIMITIVE)
	{
		if (_jsonMsg[(*tokPtr)->start] == 'n')
		{
			*ptrVoidPtr = 0;
			_bufVar.length = 0;
			_bufVar.data = 0;
			*ptrBufPtr = &_bufVar;
			(*tokPtr)++;
			return true;
		}
		unexpectedParameter(*tokPtr, __LINE__, __FILE__);
		return false;
	}
	else if ((*tokPtr)->type != JSMN_STRING)
	{
		unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
		return false;
	}
	char *toAddr = &_jsonMsg[(*tokPtr)->start];
	char *fromAddr = toAddr;
	for (int i = 0; i <  (*tokPtr)->end - (*tokPtr)->start; i++, toAddr++, fromAddr++ )
	{
		if ( *fromAddr == '\\' )
		{
			fromAddr++;
			i++;
		}
		*toAddr = *fromAddr;
	}

	(*ptrBufPtr)->length = (rtrUInt32)(toAddr - &_jsonMsg[(*tokPtr)->start]);
	(*ptrBufPtr)->data = &_jsonMsg[(*tokPtr)->start];
	(*tokPtr)++;
	return true;
}
bool jsonToRwfBase::processUTF8String(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	return processAsciiString(tokPtr, ptrBufPtr, ptrVoidPtr);
}
bool jsonToRwfBase::processRMTESString(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	return processAsciiString(tokPtr, ptrBufPtr, ptrVoidPtr);
}
//////////////////////////////////////////////////////////////////////
//
// Helper functions
//
//////////////////////////////////////////////////////////////////////

bool jsonToRwfBase::skipObject(jsmntok_t ** const objTok)
{
	switch ((*objTok)->type)
	{
		case JSMN_OBJECT:
		case JSMN_ARRAY:
		{
			int end = (*objTok)->end;
			while( (*objTok) < _tokensEndPtr && (*objTok)->start < end)
				(*objTok)++;
			return true;
		}
		case JSMN_STRING:
		case JSMN_PRIMITIVE:
		{
			(*objTok)++;
			return true;
		}
	}
	return false;
}

bool jsonToRwfBase::skipArray(jsmntok_t ** const objTok)
{
	if ( (*objTok)->type != JSMN_ARRAY)
		return false;
	int end = (*objTok)->end;
	while((*objTok) < _tokensEndPtr && (*objTok)->start < end)
		(*objTok)++;
	return true;
}

const char *jsonToRwfBase::errorFile()
{
	if (_error == false)
		return 0;

	switch (_errorCode)
	{
		case MEM_ALLOC_FAILURE:
		case JSMN_PARSE_ERROR:
		case INVALID_TOKEN_TYPE:
		case UNEXPECTED_VALUE:
		case INVALID_PRIMITIVE_TYPE:
		case INVALID_CONTAINER_TYPE:
		case SET_DEFINITION_ERROR:
		case RSSL_ENCODE_ERROR:
		case UNEXPECTED_KEY:
		case UNEXPECTED_FID:
		case MISSING_KEY:
		case TYPE_MISMATCH:
			return _errorFile;
			break;
		case NO_MSG_BASE:
		case UNSUPPORTED_MSG_TYPE:
		case NO_ERROR_CODE:
		default:
			break;
		}
		return 0;
}

int *jsonToRwfBase::errorLineNum()
{
	if (_error == false)
		return 0;

	switch (_errorCode)
	{
		case MEM_ALLOC_FAILURE:
		case JSMN_PARSE_ERROR:
		case INVALID_TOKEN_TYPE:
		case UNEXPECTED_VALUE:
		case INVALID_PRIMITIVE_TYPE:
		case INVALID_CONTAINER_TYPE:
		case SET_DEFINITION_ERROR:
		case RSSL_ENCODE_ERROR:
		case UNEXPECTED_KEY:
		case MISSING_KEY:
		case UNEXPECTED_FID:
		case TYPE_MISMATCH:
			return &_errorLineNum;
			break;
		case NO_MSG_BASE:
		case UNSUPPORTED_MSG_TYPE:
			_errorText.length = sprintf(_errorText.data, "Unsupported Message Type");
			break;
		case NO_ERROR_CODE:
		default:
			break;
		}
		return 0;
}

jsmntok_t	*jsonToRwfBase::errorToken()
{
	if (_error == false)
		return 0;

	switch (_errorCode)
	{
		case INVALID_TOKEN_TYPE:
		case UNEXPECTED_VALUE:
		case INVALID_PRIMITIVE_TYPE:
		case INVALID_CONTAINER_TYPE:
		case UNEXPECTED_KEY:
		case TYPE_MISMATCH:
		case UNEXPECTED_FID:
			if(_errorToken) {
				return _errorToken;
			}
			break;
		default:
			break;
		}
		return 0;
}

char *jsonToRwfBase::jsonMsg()
{
	if (_error == false)
		return 0;

	if(_jsonMsg) {
		return _jsonMsg;
	}

	return 0;
}
