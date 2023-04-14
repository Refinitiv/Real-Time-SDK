/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2023 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */


#ifndef __rtr_rwfToJsonConverter
#define __rtr_rwfToJsonConverter
#include "rtr/rwfToJsonBase.h"

class rwfToJsonConverter : public rwfToJsonBase
{
public:
	// Constructor
	rwfToJsonConverter(int bufSize, u_16 convFlags);

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

#endif
