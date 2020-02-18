/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


#ifndef __rtr_rwfToJsonSimple
#define __rtr_rwfToJsonSimple
#include "rtr/rwfToJsonBase.h"

#define JSON_FIXED_SIMPLFIED_PREQUEL 6  //   {"Id":
#define MAX_MSG_SIMPLIFIED_PREQUEL 16 // FIXED PREQUEL + 10 char possible streamId

class rwfToJsonSimple : public rwfToJsonBase
{
public:
	// Constructor
	rwfToJsonSimple(int bufSize, u_16 convFlags = 0);

	// Destructor
	~rwfToJsonSimple();

	const RsslBuffer* getJsonMsg(RsslInt32 streamId, bool solicited = true, bool close = false);
	void reset();
	const RsslBuffer *generateErrorMessage(char *_errorText, const char *_errorFile, RsslInt32 *_errorLine, RsslInt32 *_errorOffset, RsslBuffer *_errorOriginalMessage, RsslInt32 streamId);

	inline void setExpandEnumFields(RsslBool expandEnumFields)
	{
		if (expandEnumFields) _convFlags |= EnumExpansionFlag;
		else _convFlags &= EnumExpansionFlag;
	}

 protected:

	// Message Handlers
	int processMsg(RsslDecodeIterator*, RsslMsg &, bool first=true);
	int processRequestMsg(RsslDecodeIterator*, RsslMsg &);
	int processRefreshMsg(RsslDecodeIterator*, RsslMsg &);
	int processStatusMsg(RsslDecodeIterator*, RsslMsg &);
	int processUpdateMsg(RsslDecodeIterator*, RsslMsg &);
	int processCloseMsg(RsslDecodeIterator*, RsslMsg &);
	int processAckMsg(RsslDecodeIterator*, RsslMsg &);
	int processGenericMsg(RsslDecodeIterator*, RsslMsg &);
	int processPostMsg(RsslDecodeIterator*, RsslMsg &);

	int processMsgKey(const RsslMsgKey*, RsslDecodeIterator*, RsslUInt8 domain, bool wantServiceName = true);
	int processQOS(const RsslQos*);
	int processState(const RsslState*);
	int processPostUserInfo(RsslPostUserInfo*);
	
	typedef struct
	{
		char* 		setDefDbMem[15];	/* Array of Memory Blocks */
		int 		inUse;			/* Amount of Blocks in use / Last In-use block */
	} SetDefDbMem;
	
	static DEV_THREAD_LOCAL SetDefDbMem _setDefDbMem;

	// Container Handlers
	int processContainer(RsslUInt8 containerType, RsslDecodeIterator *iterPtr , const RsslBuffer* encDataBuf, void * setDb, bool writeTag);
	int processOpaque(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processXml(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag);
	int processFieldList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processElementList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processAnsiPage(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processFilterList(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processVector(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processMap(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processSeries(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processMsg(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 
	int processJson(RsslDecodeIterator*, const RsslBuffer*, void *, bool writeTag); 

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
	int processEnumerationExpansion(RsslDecodeIterator *, const RsslDictionaryEntry *def);
	int processArray(RsslDecodeIterator *);
	int processBuffer(RsslDecodeIterator *);
	int processAsciiString(RsslDecodeIterator *);
	int processUTF8String(RsslDecodeIterator *);
	int processRMTESString(RsslDecodeIterator *);

	int estimateJsonLength(RsslUInt32 rwfLength);
	void writeStringVar(const char* value, bool comma);
	void writeBufVar(const RsslBuffer *buf, bool comma);
	void writeBufString(const RsslBuffer *buf);
	void writeJsonBoolString(bool isTrue);
	void writeJsonErrorMessage(RsslBuffer *message);
};

inline void rwfToJsonSimple::writeStringVar(const char* value, bool comma)
{
	if (verifyJsonMessageSize((int)strlen(value) + 4) == 0) return;
	
	if (comma)
		*_pstr++ = _COMMA_CHAR;
	else
		*_pstr++ = _DOUBLE_QUOTE_CHAR;

	while (*_pstr = *value)
		_pstr++, value++;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = _COLON_CHAR;
}

inline void rwfToJsonSimple::writeBufVar(const RsslBuffer *buf, bool comma)
{
	if (verifyJsonMessageSize(buf->length + 4) == 0) return;
	
	if (comma)
		*_pstr++ = _COMMA_CHAR;
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	for (int i = 0; i < (int)buf->length; i++) 
	{
		*_pstr++ = buf->data[i]; 
	}
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	*_pstr++ = _COLON_CHAR;
}

inline void rwfToJsonSimple::writeBufString(const RsslBuffer *buf)
{
	if (verifyJsonMessageSize(buf->length + 2) == 0) return;

	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	for (int i = 0; i < (int)buf->length; i++) 
	{
		*_pstr++ = buf->data[i]; 
	}
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}

inline void rwfToJsonSimple::writeJsonBoolString(bool isTrue)
{
	if (isTrue)
	{
		writeJsonString("true", 4);
	}
	else
	{
		writeJsonString("false", 5);
	}
}

inline void rwfToJsonSimple::writeJsonErrorMessage(RsslBuffer *message)
{
	if (verifyJsonMessageSize(message->length * 6 + 2) == 0) return; // Assumes worst case scenario

	*_pstr++ = _DOUBLE_QUOTE_CHAR;
	for(int i = 0; i < (int)message->length; i++) {
	  switch(message->data[i])
	  {
	  case '\"':
		*_pstr++ = '\\';
		*_pstr++ = '\"';
		break;
	  case '\\':
	  	*_pstr++ = '\\';
	  	*_pstr++ = '\\';
	  	break;
	  case '\n':
	  	*_pstr++ = '\\';
	  	*_pstr++ = 'n';
	  	break;
	  case '\t':
	  	*_pstr++ = '\\';
	  	*_pstr++ = 't';
	  	break;
	  default:
	  	if (message->data[i] < ' ' || message->data[i] == 0x7F )
	  	{
	  		_pstr += sprintf(_pstr, "\\u%04x", message->data[i]);
	  	}
	  	else
	  		*_pstr++ = message->data[i];
	  	break;
	  }
	}
	*_pstr++ = _DOUBLE_QUOTE_CHAR;
}

#endif
