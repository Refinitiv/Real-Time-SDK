/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2023 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtr_jsonToRwfBase
#define __rtr_jsonToRwfBase
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslJsonConverter.h"
#include "jsmn.h"

#define POS_EXP_MIN 0
#define POS_EXP_MAX 7
#define NEG_EXP_MIN 1
#define NEG_EXP_MAX 14
#define ERROR_TEXT_MAX 250

#define JSON_FLAGS_DEFAULT 0
#define JSON_FLAG_DEFAULT_QOS 0x01
#define JSON_FLAG_CATCH_UNEXPECTED_KEYS 0x02
#define JSON_FLAG_CATCH_UNEXPECTED_FIDS 0x04
#define JSON_FLAG_ALLOW_ENUM_DISPLAY_STRINGS	0x08

static const RsslBuffer JSON_INFINITY = { 3, (char*)"Inf"};
static const RsslBuffer JSON_NEG_INFINITY = { 4, (char*)"-Inf"};
static const RsslBuffer JSON_NAN = { 3, (char*)"NaN"};

class RJCSmallString;
typedef RsslRet RsslJsonServiceNameToIdCallback(RsslBuffer *pServiceName, void *closure, RsslUInt16 *pServiceId);

class jsonToRwfBase
{
 public:

	void setDictionaryList(RsslDataDictionary **pDictionaryList, RsslUInt32 dictionaryCount)
	{
		_dictionaryList = pDictionaryList;
		_dictionaryCount = dictionaryCount;
	}

	jsonToRwfBase(int bufSize, unsigned int flags, int numTokens, int incSize);
	virtual ~jsonToRwfBase();
	int decodeJsonMsg(RsslJsonMsg &jsonMsg);
	int parseJsonBuffer(const RsslBuffer *bufPtr, int offset);
	virtual RsslBuffer* errorText();
	virtual void reset();
	void setRsslServiceNameToIdCallback(void *closure, RsslJsonServiceNameToIdCallback *callback)
	{ _closure = closure; _rsslServiceNameToIdCallback = callback; }

	const char *errorFile();
	int *errorLineNum();
	jsmntok_t	*errorToken();
	char *jsonMsg();

	RsslBuffer _outBuf;	// need to move
	bool _error;
	const char *_errorFile;
	char *_jsonMsg;

	jsmntok_t *_tokens;
	jsmntok_t *_tokensEndPtr;
	jsmntok_t *_curMsgTok;
	int _numTokens;
	int _incSize;

	int _bufSize;
	RsslEncodeIterator _iter;

	// Error reporting information
	enum errorCodes{
		NO_ERROR_CODE = 0,	// No error code was set - default
		MEM_ALLOC_FAILURE = 1,	// Unable to allocate memory
		JSMN_PARSE_ERROR = 2,	// Error was detected in parser - see _jsmnError
		INVALID_TOKEN_TYPE = 3,	// An unexpected token type was encountered in json message - see _errorToken & _expectedTokenType
		UNEXPECTED_VALUE = 4,	// An unexpected value was encountered in a token. - see _errorToken
		INVALID_PRIMITIVE_TYPE = 5,
		INVALID_CONTAINER_TYPE = 6,
		SET_DEFINITION_ERROR = 7,
		RSSL_ENCODE_ERROR = 8,	// see RSSL return code
		NO_MSG_BASE = 9,
		UNSUPPORTED_MSG_TYPE = 10,
		UNSUPPORTED_DATA_TYPE = 11,
		MISSING_KEY = 12, // Missing Required Key
		TYPE_MISMATCH = 13, // Array Type Mismatch
		UNEXPECTED_KEY = 14, // Unexpected Key
		UNEXPECTED_FID = 15, // Unexpected FID
		RSSL_DICT_NOT_INIT = 16, // RsslDataDictionary is not initialized.
		EMPTY_MSG = 17 // Empty JSON
	};
	errorCodes	_errorCode;
	jsmnerr_t	_jsmnError;
	jsmntype_t	_expectedTokenType;
	RsslBuffer _errorParentKey;
	RsslBuffer _errorMissingKey;
	jsmntok_t	*_errorToken;
	jsmntype_t _errorFirstType;
	jsmntype_t _errorSecondType;

	int			_rsslRet;
	int			_errorLineNum;
	RsslBuffer	_errorText;
	int			_errorTextLen;

 protected:

	RsslDataDictionary **_dictionaryList;
	RsslUInt32	_dictionaryCount;

	RsslJsonServiceNameToIdCallback *_rsslServiceNameToIdCallback;
	void *_closure;

	unsigned int _flags;
	// Message Processers
	bool encodeRsslMsg(RsslMsg *rsslMsgPtr, jsmntok_t ** const msgTok, jsmntok_t *dataTokPtr, jsmntok_t *attribTokPtr, jsmntok_t *reqKeyattrib);
	virtual bool encodeMsgPayload(RsslMsg *rsslMsgPtr, jsmntok_t *dataTokPtr);
	//	virtual bool encodeRsslData(RsslMsg *rsslMsgPtr, jsmntok_t *dataTokPtr);
	typedef  bool (jsonToRwfBase::*msgHandlerFuncPtr)(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	static const msgHandlerFuncPtr _msgHandlers[];
	virtual bool processMessage(jsmntok_t ** const msgTok, RsslJsonMsg * const jsonMsgPtr) = 0;
	virtual bool processRequestMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib) = 0;
	virtual bool processRefreshMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib) = 0;
	virtual bool processStatusMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib) = 0;
	virtual bool processUpdateMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib) = 0;
	virtual bool processCloseMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib) = 0;
	virtual bool processAckMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib) = 0;
	virtual bool processGenericMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib) = 0;
	virtual bool processPostMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib) = 0;

	// Primitive Handlers.
	bool processPrimitive(int primitiveType, jsmntok_t** const , RsslBuffer** const , void** const );
	typedef bool (jsonToRwfBase::*primitiveHandlerPtr)(jsmntok_t** const , RsslBuffer** const , void** const );
	static const primitiveHandlerPtr _primitiveHandlers[];
	bool processUnknown(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processInteger(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processUnsignedInteger(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processFloat(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processDouble(jsmntok_t** const , RsslBuffer** const , void** const );
	virtual bool processReal(jsmntok_t** const , RsslBuffer** const , void** const ) = 0;
	virtual bool processDate(jsmntok_t** const , RsslBuffer** const , void** const ) = 0;
	virtual bool processTime(jsmntok_t** const , RsslBuffer** const , void** const ) = 0;
	virtual bool processDateTime(jsmntok_t** const , RsslBuffer** const , void** const ) = 0;
	bool processQOS(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processState(jsmntok_t** const , RsslBuffer** const , void** const );
	virtual bool processEnumeration(jsmntok_t** const , RsslBuffer** const , void** const );
	virtual bool processArray(jsmntok_t** const , RsslBuffer** const , void** const ) = 0;
	bool processBuffer(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processAsciiString(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processUTF8String(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processRMTESString(jsmntok_t** const , RsslBuffer** const , void** const );

	inline bool compareStrings(jsmntok_t *tok, const RsslBuffer& buf)
	{
		if ((tok->end - tok->start) == buf.length && strncmp(&_jsonMsg[tok->start], buf.data, buf.length) == 0)
			return true;
		return false;
	};

	inline void unexpectedTokenType(jsmntype_t expectedTokenType, jsmntok_t *token, int lineNum, const char* file, const RsslBuffer *errorParentKey = 0)
	{
		if(!errorParentKey)
		{
			_errorParentKey.length = 0;
			_errorParentKey.data = 0;
		}
		else
		{
			_errorParentKey.length = errorParentKey->length;
			_errorParentKey.data = errorParentKey->data;
		}
		_errorCode = INVALID_TOKEN_TYPE;
		_expectedTokenType = expectedTokenType;
		_errorLineNum = lineNum;
		_errorFile = file;
		_errorToken = token;
	};

	inline void unexpectedParameter(jsmntok_t *token, int lineNum, const char* file, const RsslBuffer *errorParentKey = 0)
	{
		if(!errorParentKey)
		{
			_errorParentKey.length = 0;
			_errorParentKey.data = 0;
		}
		else
		{
			_errorParentKey.length = errorParentKey->length;
			_errorParentKey.data = errorParentKey->data;
		}
		_errorCode = UNEXPECTED_VALUE;
		_errorLineNum = lineNum;
		_errorFile = file;
		_errorToken = token;
	};

	inline void missingKey(RsslBuffer errorMissingKey, int lineNum, const char* file, const RsslBuffer *errorParentKey = 0)
	{
		if(!errorParentKey)
		{
			_errorParentKey.length = 0;
			_errorParentKey.data = 0;
		}
		else
		{
			_errorParentKey.length = errorParentKey->length;
			_errorParentKey.data = errorParentKey->data;
		}
		_errorCode = MISSING_KEY;
		_errorLineNum = lineNum;
		_errorFile = file;
		_errorMissingKey = errorMissingKey;
	};

	inline void unexpectedKey(jsmntok_t *token, int lineNum, const char* file, const RsslBuffer *errorParentKey = 0)
	{
		if(!errorParentKey)
		{
			_errorParentKey.length = 0;
			_errorParentKey.data = 0;
		}
		else
		{
			_errorParentKey.length = errorParentKey->length;
			_errorParentKey.data = errorParentKey->data;
		}
		_errorCode = UNEXPECTED_KEY;
		_errorLineNum = lineNum;
		_errorFile = file;
		_errorToken = token;
	};

	inline void unexpectedFid(jsmntok_t *token, int lineNum, const char* file, const RsslBuffer *errorParentKey = 0)
	{
		if(!errorParentKey)
		{
			_errorParentKey.length = 0;
			_errorParentKey.data = 0;
		}
		else
		{
			_errorParentKey.length = errorParentKey->length;
			_errorParentKey.data = errorParentKey->data;
		}
		_errorCode = UNEXPECTED_FID;
		_errorLineNum = lineNum;
		_errorFile = file;
		_errorToken = token;
	};

	inline void typeMismatch(jsmntok_t *token, jsmntype_t firstType, jsmntype_t secondType, int lineNum, const char* file, const RsslBuffer *errorParentKey = 0)
	{
		if(!errorParentKey)
		{
			_errorParentKey.length = 0;
			_errorParentKey.data = 0;
		}
		else
		{
			_errorParentKey.length = errorParentKey->length;
			_errorParentKey.data = errorParentKey->data;
		}
		_errorCode = TYPE_MISMATCH;
		_errorFirstType = firstType;
		_errorSecondType = secondType;
		_errorLineNum = lineNum;
		_errorFile = file;
		_errorToken = token;
	};

	inline void error(errorCodes code, int lineNum, const char* file)
	{
		_errorCode = code;
		_errorLineNum = lineNum;
		_errorFile = file;
	};

	// Container Handlers
	bool processContainer(jsmntok_t** const , RsslUInt8 containerType, void*);
	typedef  bool (jsonToRwfBase::*containerHandlerFuncPtr)(jsmntok_t ** const , void*);
	static const containerHandlerFuncPtr _containerHandlers[];
	bool processOpaque(jsmntok_t ** const , void*);
	bool processXML(jsmntok_t ** const , void*);
	bool processAnsiPage(jsmntok_t ** const , void*);
	bool processMsg(jsmntok_t ** const , void*);

	virtual bool processFieldList(jsmntok_t ** const , void*) = 0;
	virtual bool processElementList(jsmntok_t ** const , void*) = 0;
	virtual bool processFilterList(jsmntok_t ** const , void*) = 0;
	virtual bool processVector(jsmntok_t ** const , void*) = 0;
	virtual bool processMap(jsmntok_t ** const , void*) = 0;
	virtual bool processSeries(jsmntok_t ** const , void*) = 0;
	virtual bool processJson(jsmntok_t ** const , void*) = 0;



	// Helper Functions
	//	virtual bool processKey(jsmntok_t ** const keyTok, RsslMsgKey *keyPtr, jsmntok_t ** const attribTokPtr) = 0;
	bool skipObject(jsmntok_t ** const objTok);
	bool skipArray(jsmntok_t ** const objTok);
	virtual bool populateQos(jsmntok_t ** const qosTok, RsslQos *qosPtr) = 0;
	virtual bool populateState(jsmntok_t ** const , RsslState*) = 0;
	virtual bool populatePostUserInfo(jsmntok_t ** const , RsslPostUserInfo*) = 0;

	// Vars used for encoding primitives
	RsslBuffer _bufVar;
	RsslUInt _uintVar;
	RsslInt _intVar;
	RsslReal _realVar;
	RsslQos _qosVar;
	RsslState _stateVar;
	RsslDate _dateVar;
	RsslTime _timeVar;
	RsslDateTime _dateTimeVar;
	RsslFloat _floatVar;
	RsslDouble _doubleVar;

	char _nullUserName;

	static const int _posExponentTable[];
	static const int _negExponentTable[];
	static const int _primitiveEncodeTypeTable[];
};

#endif
