/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtr_jsonToRwfSimple
#define __rtr_jsonToRwfSimple
#include "jsonToRwfBase.h"

class EnumTableDefinition; // forward declaration

class jsonToRwfSimple : public jsonToRwfBase
{
 public:
	jsonToRwfSimple(int bufSize, unsigned int flags, int numTokens = DEFAULT_NUM_TOKENS, int incSize = DEFAULT_NUM_TOKENS);
	~jsonToRwfSimple();
	void reset();
	RsslBuffer *errorText();

	void setDefaultServiceId(RsslUInt16 serviceId) { _defaultServiceId = serviceId; _isDefaultServiceId = true; }

	inline void setUseDefaultDynamicQos(RsslBool useDefault)
	{
		if (useDefault) _flags |= JSON_FLAG_DEFAULT_QOS;
		else _flags &= ~JSON_FLAG_DEFAULT_QOS;
	}

  inline void setCatchUnknownJsonKeys(RsslBool catchKeys)
	{
		if (catchKeys) _flags |= JSON_FLAG_CATCH_UNEXPECTED_KEYS;
		else _flags &= ~JSON_FLAG_CATCH_UNEXPECTED_KEYS;
	}

  inline void setCatchUnknownJsonFids(RsslBool catchFids)
	{
		if (catchFids) _flags |= JSON_FLAG_CATCH_UNEXPECTED_FIDS;
		else _flags &= ~JSON_FLAG_CATCH_UNEXPECTED_FIDS;
	}

  inline bool setAllowEnumDisplayStrings(RsslBool allow)
  {
	  if (allow) 
	  {
		  _flags |= JSON_FLAG_ALLOW_ENUM_DISPLAY_STRINGS;
		  
		  if (initializeEnumTableDefinition() != RSSL_RET_SUCCESS)
		  {
			  return false;
		  }
	  }
	  else
	  {
		  _flags &= ~JSON_FLAG_ALLOW_ENUM_DISPLAY_STRINGS;
	  }

	  return true;
  }

  bool rmtesToUtf8(const RsslBuffer &buffer, RsslBuffer& pOutBuffer);

 private:
	RsslUInt16		_defaultServiceId;
	bool			_isDefaultServiceId;
	jsmntok_t *		_viewTokPtr;
	jsmntok_t *		_batchReqTokPtr;
	jsmntok_t *		_batchCloseTokPtr;
	EnumTableDefinition**		_enumTableDefinition;
	const RsslDictionaryEntry*	_pDictionaryEntry;

	// Buffers used for RMTES to UTF8 conversion
	char* _utf8Buf;
	int _utf8BufSz;

	inline bool isTokenTrue(jsmntok_t *tok)
	{
		switch (tok->end - tok->start)
		{
		case 4:
			if (strncmp(&_jsonMsg[tok->start], "true", 4) == 0)
				return true;
			break;
		case 1:
			if (_jsonMsg[tok->start] == '1')
				return true;
			break;
		default:
			break;
		}
		return false;
	};

 protected:

	bool encodeMsgPayload(RsslMsg *rsslMsgPtr, jsmntok_t *dataTokPtr);
	bool encodeBatchView(RsslMsg *rsslMsgPtr);
	// Message Processers
	bool processMessage(jsmntok_t ** const msgTok, RsslJsonMsg * const jsonMsgPtr);
	bool processRequestMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processRefreshMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processStatusMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processUpdateMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processCloseMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processAckMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processGenericMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processPostMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);

	// Container Handlers
	// 	bool processContainer(jsmntok_t** const , RsslUInt8 containerType, void*);
	bool processFieldList(jsmntok_t ** const , void*);
	bool processElementList(jsmntok_t ** const , void*);
	bool processFilterList(jsmntok_t ** const , void*);
	bool processVector(jsmntok_t ** const , void*);
	bool processMap(jsmntok_t ** const , void*);
	bool processSeries(jsmntok_t ** const , void*);
	bool processMsg(jsmntok_t ** const , void*);
	bool processJson(jsmntok_t ** const , void*);

	// Primitive Handlers
	//	bool processPrimitive(int primitiveType, jsmntok_t** const , RsslBuffer** const , void** const );
	bool processReal(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processDateTime(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processArray(jsmntok_t** const , RsslBuffer** const , void** const );

	// Helper Functions
	bool processKey(jsmntok_t ** const keyTok, RsslMsgKey *keyPtr, RsslUInt8 domain,jsmntok_t ** const attribTokPtr);
	bool populateQos(jsmntok_t ** const qosTok, RsslQos *qosPtr);
	bool populateState(jsmntok_t ** const , RsslState*);
	bool processEnumeration(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr);
	bool populatePostUserInfo(jsmntok_t ** const , RsslPostUserInfo*);
	bool processDate(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processTime(jsmntok_t** const , RsslBuffer** const , void** const );
	bool getDataType(jsmntok_t * tok, RsslContainerType* formatPtr);
	bool getContainerType(jsmntok_t* tok, RsslContainerType* formatPtr);
	bool getVectorAction(jsmntok_t* tok, RsslUInt8* action);
	bool skipEntriesAndGetPayloadType(jsmntok_t ** const objTok, RsslContainerType* formatPtr);
	//	bool getDataFormat(jsmntok_t * tok, RsslContainerType* formatPtr);

	RsslRet initializeEnumTableDefinition();
};

#endif
