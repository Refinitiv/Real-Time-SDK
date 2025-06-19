/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtr_jsonToRwfConverter
#define __rtr_jsonToRwfConverter
#include "jsonToRwfBase.h"
class jsonToRwfConverter : public jsonToRwfBase
{
public:
	jsonToRwfConverter(int bufSize, unsigned int flags, int numTokens = 500, int incSize = 500);
	~jsonToRwfConverter();
	RsslBuffer* errorText();

	//private:
 protected:
	// Message Processers
	bool processMessage(jsmntok_t ** const msgTok, RsslJsonMsg * const jsonMsgPtr);
	//	typedef  bool (jsonToRwfConverter::*msgHandlerFuncPtr)(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	//	static const msgHandlerFuncPtr _msgHandlers[];
	bool processRequestMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processRefreshMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processStatusMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processUpdateMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processCloseMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processAckMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processGenericMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);
	bool processPostMsg(jsmntok_t ** const msg, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib);

	// Container Handlers
 	//bool processContainer(jsmntok_t** const , RsslUInt8 containerType, void*);
	//	typedef  bool (jsonToRwfConverter::*containerHandlerFuncPtr)(jsmntok_t ** const , void*);
	//	static const containerHandlerFuncPtr _containerHandlers[];
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
	//	typedef bool (jsonToRwfBase::*primitiveHandlerPtr)(jsmntok_t** const , RsslBuffer** const , void** const );
	//	static const primitiveHandlerPtr _primitiveHandlers[];
	bool processReal(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processDateTime(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processArray(jsmntok_t** const , RsslBuffer** const , void** const );

	// Helper Functions
	bool processKey(jsmntok_t ** const keyTok, RsslMsgKey *keyPtr, jsmntok_t ** const attribTokPtr);
	bool processStandardFieldEntries(jsmntok_t *, RsslFieldList*);
	bool processStandardElementEntries(jsmntok_t *, RsslElementList*);
	bool processSetFieldEntries(jsmntok_t *, RsslFieldList*, RsslLocalFieldSetDefDb *);
	bool processSetElementEntries(jsmntok_t *, RsslElementList*, RsslLocalElementSetDefDb *);
	bool populateQos(jsmntok_t ** const qosTok, RsslQos *qosPtr);
	bool populateState(jsmntok_t ** const , RsslState*);
	bool populatePostUserInfo(jsmntok_t ** const , RsslPostUserInfo*);
	bool populateDate(jsmntok_t ** const , RsslDate*);
	bool populateTime(jsmntok_t ** const , RsslTime*);
	bool processDate(jsmntok_t** const , RsslBuffer** const , void** const );
	bool processTime(jsmntok_t** const , RsslBuffer** const , void** const );

	bool processFieldSetDefs(jsmntok_t ** const );
	bool processElementSetDefs(jsmntok_t ** const );

	// Set Definition Databases
	RsslLocalFieldSetDefDb* _fieldSetDefDbs[RSSL_ITER_MAX_LEVELS];
	RsslLocalFieldSetDefDb *getFieldSetDefDb();
	RsslLocalElementSetDefDb* _elementSetDefDbs[RSSL_ITER_MAX_LEVELS];
	RsslLocalElementSetDefDb* getElementSetDefDb();
	RsslLocalFieldSetDefDb* processFieldSetDef(jsmntok_t *);
	RsslLocalElementSetDefDb* processElementSetDef(jsmntok_t *);
};
#endif
