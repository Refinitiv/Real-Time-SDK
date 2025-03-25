/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2023, 2025 LSEG. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "rtr/rsslDataPackage.h"
#include "rtr/rtrdiv.h"
#include "rtr/rtrdiv10.h"

#include "rtr/rwfToJsonConverter.h"

//Use 1 to 3 byte variable UTF encoding
//#define MaxUTF8Bytes 3

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////////
rwfToJsonConverter::rwfToJsonConverter(int bufSize, RsslUInt16 convFlags)
	: rwfToJsonBase(bufSize, MAX_MSG_PREQUEL, convFlags, DEFAULT_NUM_TOKENS, DEFAULT_NUM_TOKENS)
{
	if ((_fieldSetDefDbMem = (char*)malloc(4096)) == 0)
		_error = 1 ;
	if ((_elementSetDefDbMem = (char*)malloc(4096)) == 0)
		_error = 1 ;
}
//////////////////////////////////////////////////////////////////////
//
// Destructor
//
//////////////////////////////////////////////////////////////////////
rwfToJsonConverter::~rwfToJsonConverter()
{
	if (_fieldSetDefDbMem)
	{
		free(_fieldSetDefDbMem);
		_fieldSetDefDbMem = 0;
	}
	if (_elementSetDefDbMem)
	{
		free(_elementSetDefDbMem);
		_elementSetDefDbMem = 0;
	}
}
const RsslBuffer *rwfToJsonConverter::getJsonMsg(RsslInt32 streamId, bool solicited, bool close)
{
	char streamIdString[20];
	int streamIdStringLen = 0;
	char *msgStart = 0;
	char *pstrSave = _pstr;
	int len;

	*_pstr = 0;		// Just in case someone prints it, won't get copied

	len = (int)(_pstr - _buf - MAX_MSG_PREQUEL); 

	_pstr = streamIdString;
	int32ToStringOffBuffer(streamId);
	streamIdStringLen = (int)(_pstr - streamIdString);

	len += JSON_FIXED_PREQUEL + streamIdStringLen;
	msgStart = _buf + MAX_MSG_PREQUEL - (JSON_FIXED_PREQUEL + streamIdStringLen);
	_pstr = msgStart;
	_buffer.data = msgStart;
	_buffer.length = len;


	if (_msgClass == RSSL_MC_REFRESH)
		*_solicitedFlagPtr = solicited ? '1' : '0';

	writeOb();
	writeVar('b', false);
	writeOb();
	writeVar('s', false);
	doSimpleMemCopy(_pstr,streamIdString,streamIdStringLen);
	_pstr = pstrSave;
	return &_buffer;
}

//////////////////////////////////////////////////////////////////////
// 
// Message Processors
//
//////////////////////////////////////////////////////////////////////
///////////////////////////////////
// 
// Messgae Process Entry Point
//
///////////////////////////////////
int rwfToJsonConverter::processMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg, bool first)
{
	// For the outermost message we leave out the beginning of the message up to the streamId
	// This will be populated when the message is extracted.
	if (!first)
	{
		_firstMsg = false;
		// For embedded messages include the entire message base
		writeOb();
		writeVar('b', false);
		writeOb();
		writeVar('s', false);
		uInt32ToString(iMsg.msgBase.streamId);
	}
	else
		_msgClass =  iMsg.msgBase.msgClass;  // Used for AJAX message extraction

	writeVar('c', true);
	uInt32ToString(iMsg.msgBase.msgClass);
	writeVar('t', true);
	uInt32ToString(iMsg.msgBase.domainType);
	writeVar('f', true);
		
	uInt32ToString(iMsg.msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN);
	writeOe();
	// Call the appropriate message handler based on the msgClass
	if ((this->*_msgHandlers[iMsg.msgBase.msgClass])(iterPtr, iMsg))
	{
		writeOe();
		return 1;
	}
	return 0;
}
///////////////////////////////////
// 
// Request Msg Processor
//
///////////////////////////////////
int rwfToJsonConverter::processRequestMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	writeVar('k', true);
	if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr))
		return 0;

	if (!(rsslRequestMsgCheckStreaming(&iMsg.requestMsg)))
	{
		// Streaming is the default, only send if 0
		writeVar('s', true);
		uInt32ToString(0);
	}
	if (iMsg.requestMsg.flags != RSSL_RQMF_NONE)
	{
		if (rsslRequestMsgCheckHasPriority(&iMsg.requestMsg))
		{
			writeVar('p', true);
			writeOb();
			writeVar('c', false);
			uInt32ToString(iMsg.requestMsg.priorityClass);
			writeVar('n', true);
			uInt32ToString(iMsg.requestMsg.priorityCount);
			writeOe();
		}
		if (rsslRequestMsgCheckMsgKeyInUpdates(&iMsg.requestMsg))
		{
			writeVar('i', true);
			uInt32ToString(1);
		}
		if ((rsslRequestMsgCheckConfInfoInUpdates(&iMsg.requestMsg)))
		{
			writeVar('c', true);
			uInt32ToString(1);
		}
		if (rsslRequestMsgCheckNoRefresh(&iMsg.requestMsg))
		{
			writeVar('n', true);
			uInt32ToString(1);
		}
		if (rsslRequestMsgCheckHasQoS(&iMsg.requestMsg))
		{
			writeVar('q', true);
			if (!processQOS(&iMsg.requestMsg.qos))
				return 0;
		}
		if (rsslRequestMsgCheckHasWorstQoS(&iMsg.requestMsg))
		{
			writeVar('w', true);
			if (!processQOS(&iMsg.requestMsg.worstQos))
				return 0;
		}
		if (rsslRequestMsgCheckPrivateStream(&iMsg.requestMsg))
		{
			writeVar('u', true);
			uInt32ToString(1);
		}
		if (rsslRequestMsgCheckQualifiedStream(&iMsg.requestMsg))
		{
			writeVar('o', true);
			uInt32ToString(1);
		}
		if (rsslRequestMsgCheckPause(&iMsg.requestMsg))
		{
			writeVar('h', true);
			uInt32ToString(1);
		}
		if (rsslRequestMsgCheckHasView(&iMsg.requestMsg))
		{
			writeVar('v', true);
			uInt32ToString(1);
		}
		if (rsslRequestMsgCheckHasBatch(&iMsg.requestMsg))
		{
			writeVar('g', true);
			uInt32ToString(1);
		}
		if (rsslRequestMsgCheckHasExtendedHdr(&iMsg.requestMsg))
		{
			writeVar('e', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.requestMsg.extendedHeader))
				return 0;
		}
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeVar('d', true);
		if (iMsg.msgBase.encDataBody.length > 0)
			return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, true);
		else
			writeString("{}");
	}
	return 1;
}
///////////////////////////////////
// 
// Refresh Msg Processor
//
///////////////////////////////////
int rwfToJsonConverter::processRefreshMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	const RsslMsgKey *pKey;
	
	if ( pKey = rsslGetMsgKey(&iMsg))
	{
		writeVar('k', true);
		if (!processMsgKey(pKey, iterPtr))
			return 0;
	}

	writeVar('s', true);
	processState(&iMsg.refreshMsg.state);

	if (iMsg.refreshMsg.groupId.length > 0)
	{
		writeVar('g', true);
		if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.refreshMsg.groupId))
			return 0;
	}
	
	/* Always send solicited flag.
	 * Saving the offset (for the first/outer message only) allows us to change it for fanout
	 */
	writeVar('a', true);
	if (_firstMsg)
		_solicitedFlagPtr = _pstr;
	rsslRefreshMsgCheckSolicited(&iMsg.refreshMsg) ? 
		uInt32ToString(1) : uInt32ToString(0);

	if (iMsg.refreshMsg.flags != RSSL_RFMF_NONE)
	{
		if (iMsg.refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY)
		{
			writeVar('r', true);
			if (!processMsgKey(&iMsg.refreshMsg.reqMsgKey, iterPtr))
				return 0;
		}

		if (rsslRefreshMsgCheckHasExtendedHdr(&iMsg.refreshMsg))
		{
			writeVar('e', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.refreshMsg.extendedHeader))
				return 0;
		}
		if (rsslRefreshMsgCheckHasPermData(&iMsg.refreshMsg))
		{
			writeVar('p', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.refreshMsg.permData))
				return 0;

		}
		if (rsslRefreshMsgCheckHasSeqNum(&iMsg.refreshMsg))
		{
			writeVar('n', true);
			uInt32ToString(iMsg.refreshMsg.seqNum);
		}
		if (!(rsslRefreshMsgCheckRefreshComplete(&iMsg.refreshMsg)))
		{
			/* Default is 1 only send if not complete
			 */
			writeVar('c', true);
			uInt32ToString(0);
		}
		if (rsslRefreshMsgCheckHasQoS(&iMsg.refreshMsg))
		{
			writeVar('q', true);
			if (!processQOS(&iMsg.refreshMsg.qos))
				return 0;
		}
		if (rsslRefreshMsgCheckClearCache(&iMsg.refreshMsg))
		{
			writeVar('t', true);
			uInt32ToString(1);
		}
		if (rsslRefreshMsgCheckDoNotCache(&iMsg.refreshMsg))
		{
			writeVar('x', true);
			uInt32ToString(1);
		}
		if (rsslRefreshMsgCheckPrivateStream(&iMsg.refreshMsg))
		{
			writeVar('u', true);
			uInt32ToString(1);
		}
		if (rsslRefreshMsgCheckQualifiedStream(&iMsg.refreshMsg))
		{
			writeVar('o', true);
			uInt32ToString(1);
		}
		if (rsslRefreshMsgCheckHasPostUserInfo(&iMsg.refreshMsg))
		{
			writeVar('i', true);
			if (!processPostUserInfo(&iMsg.refreshMsg.postUserInfo))
				return 0;
		}
		if (rsslRefreshMsgCheckHasPartNum(&iMsg.refreshMsg))
		{
			writeVar('m', true);
			uInt32ToString(iMsg.refreshMsg.partNum);
		}
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeVar('d', true);
		if (iMsg.msgBase.encDataBody.length > 0)
			return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, true);
		else
			writeString("{}");
	}
	return 1;
}
///////////////////////////////////
// 
// Status Msg Processor
//
///////////////////////////////////
int rwfToJsonConverter::processStatusMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	if (iMsg.statusMsg.flags != RSSL_STMF_NONE)
	{
		if (rsslStatusMsgCheckHasMsgKey(&iMsg.statusMsg))
		{
			writeVar('k', true);
			if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr))
				return 0;
		}
		if (iMsg.statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY)
		{
			writeVar('r', true);
			if (!processMsgKey(&iMsg.statusMsg.reqMsgKey, iterPtr))
				return 0;
		}
		if (rsslStatusMsgCheckHasGroupId(&iMsg.statusMsg))
		{
			writeVar('g', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.statusMsg.groupId))
				return 0;
		}
		if (rsslStatusMsgCheckHasState(&iMsg.statusMsg))
		{
			writeVar('s', true);
			if (!processState(&iMsg.statusMsg.state))
				return 0;

		}
		if (rsslStatusMsgCheckClearCache(&iMsg.statusMsg))
		{
			writeVar('t', true);
			uInt32ToString(1);			
		}
		if (rsslStatusMsgCheckPrivateStream(&iMsg.statusMsg))
		{
			writeVar('u', true);
			uInt32ToString(1);			
		}
		if (rsslStatusMsgCheckQualifiedStream(&iMsg.statusMsg))
		{
			writeVar('o', true);
			uInt32ToString(1);
		}
		if (rsslStatusMsgCheckHasPostUserInfo(&iMsg.statusMsg))
		{
			writeVar('i', true);
			if (!processPostUserInfo(&iMsg.statusMsg.postUserInfo))
				return 0;
		}
		if (rsslStatusMsgCheckHasPermData(&iMsg.statusMsg))
		{
			writeVar('p', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.statusMsg.permData))
				return 0;
		}
		if (rsslStatusMsgCheckHasExtendedHdr(&iMsg.statusMsg))
		{
			writeVar('e', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.statusMsg.extendedHeader))
				return 0;
		}
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeVar('d', true);
		if (iMsg.msgBase.encDataBody.length > 0)
			return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, true);
		else
			writeString("{}");
	}
	return 1;
}
///////////////////////////////////
// 
// Update Msg Processor
//
///////////////////////////////////
int rwfToJsonConverter::processUpdateMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{

	writeVar('u', true);
	uInt32ToString(iMsg.updateMsg.updateType);
	if (rsslUpdateMsgCheckDoNotConflate(&iMsg.updateMsg))
	{
		writeVar('f', true);
		uInt32ToString(1);
	}
	if (rsslUpdateMsgCheckDoNotRipple(&iMsg.updateMsg))
	{
		writeVar('r', true);
		uInt32ToString(1);
	}
	if (iMsg.updateMsg.flags & RSSL_UPMF_DISCARDABLE)	// Missing RSSL Helper function
	{
		writeVar('o', true);
		uInt32ToString(1);
	}
	if (rsslUpdateMsgCheckDoNotCache(&iMsg.updateMsg))
	{
		writeVar('x', true);
		uInt32ToString(1);
	}
	if (rsslUpdateMsgCheckHasMsgKey(&iMsg.updateMsg))
	{
		writeVar('k', true);
		if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr))
			return 0;
	}
	if (rsslUpdateMsgCheckHasPostUserInfo(&iMsg.updateMsg))
	{
		writeVar('i', true);
		if (!processPostUserInfo(&iMsg.updateMsg.postUserInfo))
			return 0;
	}
	if (rsslUpdateMsgCheckHasPermData(&iMsg.updateMsg))
	{
		writeVar('p', true);
		processPrimitive(RSSL_DT_BUFFER, &iMsg.updateMsg.permData);
	}
	if (rsslUpdateMsgCheckHasSeqNum(&iMsg.updateMsg))
	{
		writeVar('n', true);
		uInt32ToString(iMsg.updateMsg.seqNum);
	}
	if (rsslUpdateMsgCheckHasConfInfo(&iMsg.updateMsg))
	{
		writeVar('c', true);
		writeOb();
		writeVar('c', false);
		uInt32ToString(iMsg.updateMsg.conflationCount);
		writeVar('t', true);
		uInt32ToString(iMsg.updateMsg.conflationTime);
		writeOe();
	}
	if (rsslUpdateMsgCheckHasExtendedHdr(&iMsg.updateMsg))
	{
		writeVar('e', true);
		if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.updateMsg.extendedHeader))
			return 0;
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeVar('d', true);
		if (iMsg.msgBase.encDataBody.length > 0)
			return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, true);
		else
			writeString("{}");
	}
	return 1;
}
///////////////////////////////////
// 
// Close Msg Processor
//
///////////////////////////////////
int rwfToJsonConverter::processCloseMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	if (rsslCloseMsgCheckAck(&iMsg.closeMsg))
	{
		writeVar('a', true);
		uInt32ToString(1);
	}
	if (rsslCloseMsgCheckHasExtendedHdr(&iMsg.closeMsg))
	{
		writeVar('e', true);
		if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.closeMsg.extendedHeader))
			return 0;
	}
	if (rsslCloseMsgCheckHasBatch(&iMsg.closeMsg))   //Supports Batch Close
	{
		writeVar('g', true);
		uInt32ToString(1);
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeVar('d', true);
		if (iMsg.msgBase.encDataBody.length > 0)
			return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, true);
		else
			writeString("{}");
	}
	return 1;
}
///////////////////////////////////
// 
// Ack Msg Processor
//
///////////////////////////////////
int rwfToJsonConverter::processAckMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	writeVar('i', true);
	uInt32ToString(iMsg.ackMsg.ackId);

	if (iMsg.ackMsg.flags != RSSL_AKMF_NONE)
	{
		if (rsslAckMsgCheckHasNakCode(&iMsg.ackMsg))
		{
			writeVar('c', true);
			uInt32ToString(iMsg.ackMsg.nakCode);
		}
		if (rsslAckMsgCheckHasText(&iMsg.ackMsg))
		{
			writeVar('t', true);
			writeString(iMsg.ackMsg.text.data, iMsg.ackMsg.text.length);
		}
		if (rsslAckMsgCheckPrivateStream(&iMsg.ackMsg))
		{
			writeVar('u', true);
			uInt32ToString(1);
		}
		if (rsslAckMsgCheckHasSeqNum(&iMsg.ackMsg))
		{
			writeVar('n', true);
			uInt32ToString(iMsg.ackMsg.seqNum);
		}
		if (rsslAckMsgCheckHasMsgKey(&iMsg.ackMsg))
		{
			writeVar('k', true);
			if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr))
				return 0;
		}
		if (rsslAckMsgCheckHasExtendedHdr(&iMsg.ackMsg))
		{
			writeVar('e', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.ackMsg.extendedHeader))
				return 0;
		}
		if (rsslAckMsgCheckQualifiedStream(&iMsg.ackMsg))
		{
			writeVar('o', true);
			uInt32ToString(1);
		}
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeVar('d', true);
		if (iMsg.msgBase.encDataBody.length > 0)
			return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, true);
		else
			writeString("{}");
	}
	return 1;
}
///////////////////////////////////
// 
// Generic Msg Processor
//
///////////////////////////////////
int rwfToJsonConverter::processGenericMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	if (iMsg.genericMsg.flags != RSSL_GNMF_NONE)
	{
		if (rsslGenericMsgCheckHasMsgKey(&iMsg.genericMsg))
		{
			writeVar('k', true);
			if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr))
				return 0;
		}
		if (iMsg.genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY)
		{
			writeVar('r', true);
			if (!processMsgKey(&iMsg.genericMsg.reqMsgKey, iterPtr))
				return 0;
		}
		if (rsslGenericMsgCheckHasSeqNum(&iMsg.genericMsg))
		{
			writeVar('n', true);
			uInt32ToString(iMsg.genericMsg.seqNum);
		}
		if (rsslGenericMsgCheckHasSecondarySeqNum(&iMsg.genericMsg))
		{
			writeVar('s', true);
			uInt32ToString(iMsg.genericMsg.secondarySeqNum);
		}
		if (rsslGenericMsgCheckHasPartNum(&iMsg.genericMsg))
		{
			writeVar('m', true);
			uInt32ToString(iMsg.genericMsg.partNum);
		}
		if (rsslGenericMsgCheckHasPermData(&iMsg.genericMsg))
		{
			writeVar('p', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.genericMsg.permData))
				return 0;
		}
		if (rsslGenericMsgCheckHasExtendedHdr(&iMsg.genericMsg))
		{
			writeVar('e', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.genericMsg.extendedHeader))
				return 0;
		}
	}

	if (!(rsslGenericMsgCheckMessageComplete(&iMsg.genericMsg)))
	{
		// Default is complete only send if incomplete
		writeVar('c', true);
		uInt32ToString(0);
	}

	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeVar('d', true);
		if (iMsg.msgBase.encDataBody.length > 0)
			return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, true);
		else
			writeString("{}");
	}
	return 1;
}
///////////////////////////////////
// 
// Post Msg Processor
//
///////////////////////////////////
int rwfToJsonConverter::processPostMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{

	writeVar('i', true);
	if (!processPostUserInfo(&iMsg.postMsg.postUserInfo))
		return 0;

	if (iMsg.postMsg.flags != RSSL_PSMF_NONE)
	{
		if (rsslPostMsgCheckHasPostId(&iMsg.postMsg))
		{
			writeVar('u', true);
			uInt32ToString(iMsg.postMsg.postId);
		}
		if (rsslPostMsgCheckHasMsgKey(&iMsg.postMsg))
		{
			writeVar('k', true);
			if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr))
				return 0;
		}
		if (rsslPostMsgCheckHasSeqNum(&iMsg.postMsg))
		{
			writeVar('n', true);
			uInt32ToString(iMsg.postMsg.seqNum);
		}
		if (rsslPostMsgCheckAck(&iMsg.postMsg))
		{
			writeVar('a', true);
			uInt32ToString(1);
		}
		if (rsslPostMsgCheckHasPermData(&iMsg.postMsg))
		{
			writeVar('p', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.postMsg.permData))
				return 0;
		}
		if (rsslPostMsgCheckHasPartNum(&iMsg.postMsg))
		{
			writeVar('m', true);
			uInt32ToString(iMsg.postMsg.partNum);
		}
		if (rsslPostMsgCheckHasPostUserRights(&iMsg.postMsg))
		{
			writeVar('r', true);
			uInt32ToString(iMsg.postMsg.postUserRights);
		}
		if (rsslPostMsgCheckHasExtendedHdr(&iMsg.postMsg))
		{
			writeVar('e', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.postMsg.extendedHeader))
				return 0;
		}
	}

	if (!rsslPostMsgCheckPostComplete(&iMsg.postMsg))
	{
		// Default is complete only send if incomplete
		writeVar('c', true);
		uInt32ToString(0);
	}

	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeVar('d', true);
		if (iMsg.msgBase.encDataBody.length > 0)
			return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, true);
		else
			writeString("{}");
	}
	return 1;
}
//////////////////////////////////////////////////////////////////////
//
// Base Type Processors
//
//////////////////////////////////////////////////////////////////////
///////////////////////////////////
//
// Key Processor
//
///////////////////////////////////
int rwfToJsonConverter::processMsgKey(const RsslMsgKey *keyPtr, RsslDecodeIterator *iterPtr)
{
	bool comma = false;

	if (keyPtr->flags == RSSL_MKF_NONE)
	{
		writeOb();
		writeOe();
		return 1;
	}
	writeOb();
	if (rsslMsgKeyCheckHasServiceId((RsslMsgKey*)keyPtr))
	{
		writeVar('s', comma);
		uInt32ToString(keyPtr->serviceId);
		if (!comma)
			comma = true;

	}
	if (rsslMsgKeyCheckHasNameType((RsslMsgKey*)keyPtr))
	{
		writeVar('t', comma);
		uInt32ToString(keyPtr->nameType);
		if (!comma)
			comma = true;
	}
	if (rsslMsgKeyCheckHasName((RsslMsgKey*)keyPtr))
	{
	  	// workaround for RSSL requiring name if nametype is provided
		// Skip the name if nameType is COOKIE and name is NULL,
		if (!(keyPtr->nameType == RDM_LOGIN_USER_COOKIE &&
		      keyPtr->name.length == 1 &&
		      *keyPtr->name.data == 0x0))
		{
			writeVar('n', comma);
			writeSafeString(keyPtr->name.data, keyPtr->name.length);
			if (!comma)
				comma = true;
		}
	}
	if (rsslMsgKeyCheckHasFilter((RsslMsgKey*)keyPtr))
	{
		writeVar('x', comma);
		uInt32ToString(keyPtr->filter);
		if (!comma)
			comma = true;
	}
	if (rsslMsgKeyCheckHasIdentifier((RsslMsgKey*)keyPtr))
	{
		writeVar('i', comma);
		uInt32ToString(keyPtr->identifier);
		if (!comma)
			comma = true;
	}
	if (rsslMsgKeyCheckHasAttrib((RsslMsgKey*)keyPtr))
	{
		writeVar('a', comma);
		writeOb();		// Begin Key Attrib
		writeVar('f', false);
		uInt32ToString(keyPtr->attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN);
		writeVar('d', true);
		if (keyPtr->encAttrib.length > 0)
		{

			RsslDecodeIterator dIter;
			RsslBuffer buf;
			buf.data = keyPtr->encAttrib.data;
			buf.length = keyPtr->encAttrib.length;
			rsslClearDecodeIterator(&dIter);
			rsslSetDecodeIteratorBuffer(&dIter, &buf);
			if (!processContainer(keyPtr->attribContainerType, &dIter, &keyPtr->encAttrib, true))
				return 0;
		}
		else
			writeString("null");
		writeOe(); // End Key Attrib
	}
	writeOe();
	return 1;
}
///////////////////////////////////
//
// QOS Processor
//
///////////////////////////////////

int rwfToJsonConverter::processQOS(const RsslQos* qosPtr)
{
	writeOb();
	writeVar('t', false);
	uInt32ToString(qosPtr->timeliness);
	writeVar('r', true);
	uInt32ToString(qosPtr->rate);
	if (qosPtr->dynamic)
	{
		writeVar('d', true);
		uInt32ToString(1);
	}
	if (qosPtr->timeliness == RSSL_QOS_TIME_DELAYED)
	{
		writeVar('i', true);
		uInt32ToString(qosPtr->timeInfo);
	}
	if (qosPtr->rate  == RSSL_QOS_RATE_TIME_CONFLATED)
	{
		writeVar('s', true);
		uInt32ToString(qosPtr->rateInfo);
	}
	writeOe();
	return 1;
}
///////////////////////////////////
//
// Post User Info  Processor
//
///////////////////////////////////
int rwfToJsonConverter::processPostUserInfo(RsslPostUserInfo* poiPtr)
{
	writeOb();
	writeVar('a', false);
	uInt32ToString(poiPtr->postUserAddr);
	writeVar('u', true);
	uInt32ToString(poiPtr->postUserId);
	writeOe();
	return 1;
}

int rwfToJsonConverter::processState(const RsslState* statePtr)
{
	writeOb();
	writeVar('s', false);
	if (_firstMsg)
		_streamingStatePtr = _pstr;
	uInt32ToString(statePtr->streamState);
	writeVar('d', true);
	uInt32ToString(statePtr->dataState);

	if (statePtr->code > RSSL_SC_NONE)
	{
		writeVar('c', true);
		uInt32ToString(statePtr->code);
	}

	if (statePtr->text.length > 0)
	{
		writeVar('t', true);
		*_pstr++ = '"';
		for(int i = 0; i < (int)statePtr->text.length; i++) 
		{
			switch(statePtr->text.data[i])
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
				if (statePtr->text.data[i] < ' ' || statePtr->text.data[i] == 0x7F )
				{
					_pstr += sprintf(_pstr, "\\u%04x", statePtr->text.data[i]);
				}
				else
					*_pstr++ = statePtr->text.data[i];
				break;
			}
		}
		*_pstr++ = '"';

	}
	writeOe();
	return 1;
}

//////////////////////////////////////////////////////////////////////
// 
// Container Processors
//
//////////////////////////////////////////////////////////////////////
int rwfToJsonConverter::processContainer(RsslUInt8 containerType, RsslDecodeIterator *iterPtr,const RsslBuffer *encDataBufPtr, bool writeTag, void* setDefPtr)
{
	if (containerType > RSSL_DT_NO_DATA &&
		containerType <= RSSL_DT_CONTAINER_TYPE_MAX &&
		_containerHandlers[containerType - RSSL_DT_CONTAINER_TYPE_MIN])
	{
		return (this->*_containerHandlers[containerType - RSSL_DT_CONTAINER_TYPE_MIN])(iterPtr, encDataBufPtr, setDefPtr, writeTag);
	}
	return 0;
}
///////////////////////////////////
//
// Field List Format Processor
//
///////////////////////////////////
int rwfToJsonConverter::processFieldList(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslLocalFieldSetDefDb *fieldSetDefDb = (RsslLocalFieldSetDefDb*)setDb ;
	const RsslDictionaryEntry *def;
	RsslFieldList fieldList;
	RsslFieldEntry field;


 	RsslRet retVal = 0;
	bool comma = false;
	

	rsslClearFieldList(&fieldList);

	if ((retVal = rsslDecodeFieldList(iterPtr, &fieldList, fieldSetDefDb)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}

	writeOb();	// Begin of Field List
	if (rsslFieldListCheckHasInfo(&fieldList))
	{
		writeVar('i', false);
		writeOb();
		writeVar('d', false);
		int32ToString(fieldList.dictionaryId);
		writeVar('n', true);
		int32ToString(fieldList.fieldListNum);
		writeOe();
		comma = true;
	}

	if (rsslFieldListCheckHasSetData(&fieldList))
	{
		RsslUInt8 dataType;
		writeVar('s', comma);
		writeOb();	// Begin of Set Data
		if (!comma)
			comma = true;
		int inner = false;

		writeVar('i', false);
		int32ToString(fieldList.setId);

		writeVar('d', true);
		writeAb();
		int i = 0;
		while( i < fieldSetDefDb->definitions[fieldList.setId].count
			   && (retVal = rsslDecodeFieldEntry(iterPtr, &field)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (inner)
				writeComma();
		
			dataType = fieldSetDefDb->definitions[fieldList.setId].pEntries[i].dataType;
		
			if (dataType < RSSL_DT_SET_PRIMITIVE_MAX)
			{
				if (!processPrimitive(iterPtr, dataType))
					return 0;
			}
			else 
			{
				if (!processContainer(dataType, iterPtr, &field.encData, false))
					return 0;
			}
			if (!inner)
				inner = true;
			i++;
		}
		writeAe();
		writeOe();	// End of Set Data

		if (retVal == RSSL_RET_END_OF_CONTAINER)
		{
			// We hit END_OF_CONTAINER
			// There were less fields in the list than in the setDef so we're done here.
			// CLose the fieldList and return
			writeOe();  // End of Field List
			return 1;
		}

	}
	if (rsslFieldListCheckHasStandardData(&fieldList))
	{
		writeVar('d', comma);
		writeOb();	// Begin of Standard Data
		if (!comma)
			comma = true;

		bool inner = false;

		while ((retVal = rsslDecodeFieldEntry(iterPtr, &field)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (retVal < RSSL_RET_SUCCESS)
			{
				return 0;
			}
			else
			{
				def = _dictionaryList[0]->entriesArray[field.fieldId];
				if (def)
				{
					writeFieldId(field.fieldId, inner);
					if (!inner)
						inner = true;
					if (def->rwfType < RSSL_DT_SET_PRIMITIVE_MAX)
					{
						if (!processPrimitive(iterPtr, def->rwfType))
							return 0;
					}
					else
					{
						if (!processContainer(def->rwfType, iterPtr, &field.encData, false))
							return 0;
					}
				}
			}
			if (!inner && def)
				inner = true;
		}
		writeOe(); // End of Standard Data
	}
	else
	{
		// We only had setData, OR we didn't have either setData or standardData, then we've never seen RSSL_RET_END_OF_CONTAINER
		// We need to reset the iterator manually
		rsslFinishDecodeEntries(iterPtr);
	}

	writeOe();	// End of Field List
	return 1;
}
///////////////////////////////////
//
// Element List Container Processor
//
///////////////////////////////////
int rwfToJsonConverter::processElementList(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslElementList				elementList;
	RsslElementEntry			elementEntry;
	RsslLocalElementSetDefDb	*elementSetDefDb = (RsslLocalElementSetDefDb*)setDb;
	bool comma = false;
 	RsslRet retVal = 0;
	int inner = false;
	
	rsslClearElementList(&elementList);

	if ((retVal = rsslDecodeElementList(iterPtr, &elementList, elementSetDefDb)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}
	writeOb();
	if (rsslElementListHasInfo(&elementList))
	{
		writeVar('i', false);
		writeOb();
		writeVar('n', false);
		int32ToString(elementList.elementListNum);
		writeOe();
		comma = true;
	}

	if (rsslElementListCheckHasSetData(&elementList) )
	{
		RsslUInt8 dataType;
		writeVar('s', comma);
		comma = true;
		writeOb();	// Begin of Set Data
		
		
		if(rsslElementListCheckHasSetId(&elementList))
		{
			writeVar('i', false);
			int32ToString(elementList.setId);
			writeComma();
		}

		writeVar('d', false);
		inner = false;
		writeAb();
		int i = 0;
		while( i < elementSetDefDb->definitions[elementList.setId].count
			   && (retVal = rsslDecodeElementEntry(iterPtr, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (inner)
				writeComma();

			dataType = elementSetDefDb->definitions[elementList.setId].pEntries[i].dataType;

			if (dataType < RSSL_DT_SET_PRIMITIVE_MAX)
			{
				if (!processPrimitive(iterPtr, dataType))
					return 0;
			}
			else
			{
				if (!processContainer(dataType, iterPtr, &elementEntry.encData, false))
					return 0;
			}
			inner = true;
			i++;
		}

		writeAe();
		writeOe();	// End of Set Data

		if (retVal == RSSL_RET_END_OF_CONTAINER)
		{
			// We hit END_OF_CONTAINER.
			// There were less elements in the list than in the setDef so we're done here.
			// CLose the elementList and return
			writeOe();  // End of ElementList
			return 1;
		}
	}

	inner = false;
	if (rsslElementListCheckHasStandardData(&elementList))
	{
		writeVar('d', comma);
		writeAb();	// Begin of Standard Data
		comma = true;
		
		while ((retVal = rsslDecodeElementEntry(iterPtr, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (retVal < RSSL_RET_SUCCESS)
			{
				return 0;
			}
			else
			{
				if (inner)
					writeComma();
				else
					inner = true;
				writeOb();	// Start of Element

				writeVar('n', false);
				writeString(elementEntry.name.data, elementEntry.name.length);
				writeVar('t', true);
				int32ToString(elementEntry.dataType);
				writeVar('d', true);
				if (elementEntry.dataType < RSSL_DT_SET_PRIMITIVE_MAX)
				{
					if (!processPrimitive(iterPtr, elementEntry.dataType))
						return 0;
				}
				else if (elementEntry.dataType > RSSL_DT_NO_DATA)
				{
					if (!processContainer(elementEntry.dataType, iterPtr, &elementEntry.encData, false))
						return 0;
				}
				writeOe();	// End of Element
			}
		}

		writeAe();	// End of Standard Data
	}
	else
	{
		// We only had setData, OR we didn't have either setData or standardData, then we've never seen RSSL_RET_END_OF_CONTAINER
		// We need to reset the iterator manually
		rsslFinishDecodeEntries(iterPtr);
	}

	writeOe();  // End of ElementList
	return 1;
}
///////////////////////////////////
//
// Filter List Container Processor
//
///////////////////////////////////
int rwfToJsonConverter::processFilterList(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslFilterList filterList;
	RsslFilterEntry filterEntry;
	RsslUInt8 entryContainerType;
 	RsslRet retVal = 0;
	bool comma = false;

	rsslClearFilterList(&filterList);
	if ((retVal = rsslDecodeFilterList(iterPtr, &filterList)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}
	
	writeOb();	// Begin of FilterList
	writeVar('f', false);
	int32ToString(filterList.containerType - RSSL_DT_CONTAINER_TYPE_MIN);
	
	if (rsslFilterListCheckHasTotalCountHint(&filterList))
	{
		writeVar('c', true);
		int32ToString(filterList.totalCountHint);
	}
	if (rsslFilterListCheckHasPerEntryPermData(&filterList))
	{
		writeVar('p', true);
		int32ToString(1);
	}
	// Data
	if (filterList.encEntries.length == 0)
	{
		// Valid FilterList without any payload
		writeOe();  // End of FilterList (No data)
		return 1;
	}
	writeVar('d', true);
	writeAb();
	while ((retVal = rsslDecodeFilterEntry(iterPtr, &filterEntry)) != RSSL_RET_END_OF_CONTAINER)
	{

		if (retVal < RSSL_RET_SUCCESS)
		{
			return 0;
		}

		entryContainerType = filterList.containerType;
		if (comma)
			writeComma();
		else
			comma = true;

		writeOb();

		writeVar('i', false);
		int32ToString(filterEntry.id);

		writeVar('a', true);
		int32ToString(filterEntry.action);

		if (rsslFilterEntryCheckHasContainerType(&filterEntry))
		{
			writeVar('f', true);
			int32ToString(filterEntry.containerType - RSSL_DT_CONTAINER_TYPE_MIN);
			entryContainerType = filterEntry.containerType;
		}
		if (rsslFilterEntryCheckHasPermData(&filterEntry))
		{
			writeVar('p', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &filterEntry.permData))
				return 0;
		}
		if (entryContainerType > RSSL_DT_NO_DATA)
		{
			writeVar('d', true);
			if (!processContainer(entryContainerType, iterPtr, &filterEntry.encData, true))
				return 0;
		}
		writeOe();  // End of FilterEntry
	}
	writeAe();	// End of Data
	writeOe();	// End of FilterList
	return 1;
}
///////////////////////////////////
//
// Vector Container Processor
//
///////////////////////////////////
int rwfToJsonConverter::processVector(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslVector vector;
	RsslVectorEntry vectorEntry;
	void *localSetDb = 0;
	RsslLocalFieldSetDefDb	 fieldSetDb;
	RsslLocalElementSetDefDb elementSetDb;
 	RsslRet retVal = 0;
	bool comma = false;

	rsslClearVector(&vector);

	if ((retVal = rsslDecodeVector(iterPtr, &vector)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}
	writeOb();	// Begin of Vector
	writeVar('f', false);
	int32ToString(vector.containerType - RSSL_DT_CONTAINER_TYPE_MIN);

	if (rsslVectorCheckHasTotalCountHint(&vector))
	{
		writeVar('c', true);
		int32ToString(vector.totalCountHint);
	}
	if (rsslVectorCheckSupportsSorting(&vector))
	{
		writeVar('o', true);
		int32ToString(1);
	}
	if (rsslVectorCheckHasPerEntryPermData(&vector))
	{
		writeVar('p', true);
		int32ToString(1);
	}
	if (rsslVectorCheckHasSetDefs(&vector))
	{
		writeVar('l', true);
		if (vector.containerType == RSSL_DT_FIELD_LIST)
		{
			rsslClearLocalFieldSetDefDb(&fieldSetDb);
			fieldSetDb.entries.data = _fieldSetDefDbMem;
			fieldSetDb.entries.length = 4096;
			if (rsslDecodeLocalFieldSetDefDb(iterPtr, &fieldSetDb) < RSSL_RET_SUCCESS)
				return 0;
			localSetDb = &fieldSetDb;
			if (!processFieldListSetdb(fieldSetDb))
				return 0;
		}
		else
		{
			rsslClearLocalElementSetDefDb(&elementSetDb);
			elementSetDb.entries.data = _elementSetDefDbMem;
			elementSetDb.entries.length = 4096;
			if (rsslDecodeLocalElementSetDefDb(iterPtr, &elementSetDb) < RSSL_RET_SUCCESS)
				return 0;
			localSetDb = &elementSetDb;
			if (!processElementListSetdb(elementSetDb))
				return 0;
		}
	}
	if (rsslVectorCheckHasSummaryData(&vector))
	{
		writeVar('s', true);
		if (vector.encSummaryData.length > 0)
		{
			if (!processContainer(vector.containerType, iterPtr, &vector.encSummaryData, false, localSetDb))
				return 0;
		}
		else
			writeNull();
	}

	// Data
	if (vector.encEntries.length == 0)
	{
		// Valid vector without any payload
		writeOe();  // End of Vector (No data)
		return 1;
	}

	writeVar('d', true);
	writeAb();
	while ((retVal = rsslDecodeVectorEntry(iterPtr, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retVal < RSSL_RET_SUCCESS)
		{
			return 0;
		}

		if (comma)
			writeComma();
		else
			comma = true;

		writeOb();
		writeVar('i', false);
		uInt32ToString(vectorEntry.index);

		writeVar('a', true);
		uInt32ToString(vectorEntry.action);

		if (rsslVectorEntryCheckHasPermData(&vectorEntry))
		{
			writeVar('p', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &vectorEntry.permData))
				return 0;
		}
		if (vector.containerType > RSSL_DT_NO_DATA && vectorEntry.encData.length > 0)
		{
			writeVar('d', true);
			if (!processContainer(vector.containerType, iterPtr, &vectorEntry.encData, true, localSetDb))
				return 0;
		}

		writeOe();  // End of Vector Entry
	}
	writeAe();	// End of Data

	writeOe();  // End of Vector
	return 1;
}
///////////////////////////////////
//
// Map Container Processor
//
///////////////////////////////////
int rwfToJsonConverter::processMap(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslMap map;
	RsslMapEntry mapEntry;
	void *localSetDb = 0;
	RsslLocalFieldSetDefDb	 fieldSetDb;
	RsslLocalElementSetDefDb elementSetDb;
 	RsslRet retVal = 0;
	bool comma = false;

	rsslClearMap(&map);

	if ((retVal = rsslDecodeMap(iterPtr, &map)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}

	writeOb();	// Begin of Map
	writeVar('k', false);
	int32ToString(map.keyPrimitiveType);
	writeVar('f', true);
	int32ToString(map.containerType - RSSL_DT_CONTAINER_TYPE_MIN);

	if (!(map.flags & RSSL_MPF_NONE))
	{
		if (rsslMapCheckHasSetDefs(&map))
		{
			writeVar('l', true);
			if (map.containerType == RSSL_DT_FIELD_LIST)
			{
				rsslClearLocalFieldSetDefDb(&fieldSetDb);
				fieldSetDb.entries.data = _fieldSetDefDbMem;
				fieldSetDb.entries.length = 4096;
				if (rsslDecodeLocalFieldSetDefDb(iterPtr, &fieldSetDb) < RSSL_RET_SUCCESS)
					return 0;
				localSetDb = &fieldSetDb;
				if (!processFieldListSetdb(fieldSetDb))
					return 0;
			}
			else
			{
				rsslClearLocalElementSetDefDb(&elementSetDb);
				elementSetDb.entries.data = _elementSetDefDbMem;
				elementSetDb.entries.length = 4096;
				if (rsslDecodeLocalElementSetDefDb(iterPtr, &elementSetDb) < RSSL_RET_SUCCESS)
					return 0;
				localSetDb = &elementSetDb;
				if (!processElementListSetdb(elementSetDb))
					return 0;
			}
		}
		if (rsslMapCheckHasSummaryData(&map))
		{
			if (map.containerType > RSSL_DT_NO_DATA)
			{
				writeVar('s', true);
				if (!processContainer(map.containerType, iterPtr, &map.encSummaryData, false, localSetDb))
					return 0;
			}
		}
		if (rsslMapCheckHasPerEntryPermData(&map))
		{
			writeVar('p', true);
			int32ToString(1);
		}
		if (rsslMapCheckHasTotalCountHint(&map))
		{
			writeVar('h', true);
			int32ToString(map.totalCountHint);			
		}
		if (rsslMapCheckHasKeyFieldId(&map))
		{
			writeVar('i', true);
			int32ToString(map.keyFieldId);
		}
	}
	if (map.encEntries.length == 0)
	{
		// Valid Map without any payload
		writeOe();  // End of Map (No data)
		return 1;
	}

	writeVar('d', true);
	writeAb();	// Begin of Data
	while ((retVal = rsslDecodeMapEntry(iterPtr, &mapEntry, 0)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retVal < RSSL_RET_SUCCESS)
		{
			return 0;
		}
		if (comma)
			writeComma();
		else
			comma = true;
		writeOb();  // Begin of Map Entry

		writeVar('a', false);
		uInt32ToString(mapEntry.action);

		writeVar('k', true);
		if (map.keyPrimitiveType < RSSL_DT_SET_PRIMITIVE_MAX)
			if (!processPrimitive(map.keyPrimitiveType, &mapEntry.encKey))
				return 0;

		if (rsslMapEntryCheckHasPermData(&mapEntry))
		{
			writeVar('p', true);
			if (!processPrimitive(RSSL_DT_BUFFER, &mapEntry.permData))
				return 0;
		}
		
		writeVar('d', true);
		if (mapEntry.encData.length > 0)
		{
			if (!processContainer(map.containerType, iterPtr, &mapEntry.encData, true, localSetDb))
				return 0;
		}
		else
			writeNull();
		writeOe();  // End of Map Entry
	}
	writeAe();	// End of Data
	writeOe();	// End of Map
	return 1;
}
///////////////////////////////////
//
// Series Container Processor
//
///////////////////////////////////
int rwfToJsonConverter::processSeries(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslSeries series;
	RsslSeriesEntry seriesEntry;
	void *localSetDb = 0;
	RsslLocalFieldSetDefDb	 fieldSetDb;
	RsslLocalElementSetDefDb elementSetDb;
 	RsslRet retVal = 0;
	bool comma = false;

	rsslClearSeries(&series);

	if ((retVal = rsslDecodeSeries(iterPtr, &series)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}
	writeOb();	// Begin of Series
	writeVar('f', false);
	int32ToString(series.containerType - RSSL_DT_CONTAINER_TYPE_MIN);
	
	if (!(series.flags & RSSL_SRF_NONE))
	{

		if (rsslSeriesCheckHasTotalCountHint(&series))
		{
			writeVar('c', true);
			int32ToString(series.totalCountHint);
		}

		if (rsslSeriesCheckHasSetDefs(&series))
		{
			writeVar('l', true);
			if (series.containerType == RSSL_DT_FIELD_LIST)
			{
				rsslClearLocalFieldSetDefDb(&fieldSetDb);
				fieldSetDb.entries.data = _fieldSetDefDbMem;
				fieldSetDb.entries.length = 4096;
				if (rsslDecodeLocalFieldSetDefDb(iterPtr, &fieldSetDb) < RSSL_RET_SUCCESS)
					return 0;
				localSetDb = &fieldSetDb;
				if (!processFieldListSetdb(fieldSetDb))
					return 0;
			}
			else
			{
				rsslClearLocalElementSetDefDb(&elementSetDb);
				elementSetDb.entries.data = _elementSetDefDbMem;
				elementSetDb.entries.length = 4096;
				if (rsslDecodeLocalElementSetDefDb(iterPtr, &elementSetDb) < RSSL_RET_SUCCESS)
					return 0;
				localSetDb = &elementSetDb;
				if (!processElementListSetdb(elementSetDb))
					return 0;
			}
		}
		if (rsslSeriesCheckHasSummaryData(&series))
		{
			if (series.containerType > RSSL_DT_NO_DATA && series.encSummaryData.length > 0)
			{
				writeVar('s', true);
				if (!processContainer(series.containerType, iterPtr, &series.encSummaryData, false, localSetDb))
					return 0;
			}
		}
	}

	if (series.encEntries.length == 0)
	{
		// Valid Series without any payload
		writeOe();  // End of Series (No data)
		return 1;
	}

	writeVar('d', true);
	writeAb();	// Begin of Data

	while ((retVal = rsslDecodeSeriesEntry(iterPtr, &seriesEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retVal < RSSL_RET_SUCCESS)
		{
			return 0;
		}
		if (comma)
			writeComma();
		else
			comma = true;

		if (seriesEntry.encData.length > 0)
		{
			if (!processContainer(series.containerType, iterPtr, &seriesEntry.encData, false, localSetDb))
				return 0;
		}
		else
			writeNull();
	}

	writeAe();	// End of Data
	writeOe();	// End of Series
	return 1;
}

///////////////////////////////////
//
// Real
//
///////////////////////////////////
int rwfToJsonConverter::processReal(RsslDecodeIterator *iterPtr)
{
	RsslReal realVal;
	RsslRet retVal;

	rsslClearReal(&realVal);

	if ((retVal = rsslDecodeReal(iterPtr, &realVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA || realVal.isBlank)
	{
		writeNull();
		return 1;
	}

	if ( ((_convFlags & EncodeRealAsPrimitive) == 0) || (realVal.hint > RSSL_RH_MIN_DIVISOR))
	{
		writeOb();
		writeVar('h',false);
		uInt32ToString(realVal.hint);
		writeVar('v',true);
		int64ToString(realVal.value);
		writeOe();

	}
	else
	{
		int64ToString(realVal.value);
		if( realVal.hint != RSSL_RH_EXPONENT0)
		{
			*_pstr++ = 'e';
			int32ToString(_exponentTable[realVal.hint]);
		}
	}
	return 1;	
}

///////////////////////////////////
//
// Date
//
///////////////////////////////////
int rwfToJsonConverter::processDate(RsslDecodeIterator *iterPtr)
{
	RsslDate dateVal;
	RsslRet retVal;

	rsslClearDate(&dateVal);

	if ((retVal = rsslDecodeDate(iterPtr, &dateVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
	}
	else
	{
		writeOb();
		writeVar('d', false);
		uInt32ToString(dateVal.day);
		writeVar('m', true);
		uInt32ToString(dateVal.month);
		writeVar('y', true);
		uInt32ToString(dateVal.year);
		writeOe();
	}
	return 1;
}
///////////////////////////////////
//
// Time
//
///////////////////////////////////
int rwfToJsonConverter::processTime(RsslDecodeIterator *iterPtr)
{
	RsslTime timeVal;
	RsslRet retVal;

	rsslClearTime(&timeVal);

	if ((retVal = rsslDecodeTime(iterPtr, &timeVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
	}
	else
	{
		writeOb();
		writeVar('h', false);
		uInt32ToString(timeVal.hour);
		writeVar('m', true);
		uInt32ToString(timeVal.minute);
		writeVar('s', true);
		uInt32ToString(timeVal.second);
		writeVar('x', true);
		uInt32ToString(timeVal.millisecond);
		writeOe();
	}
	return 1;
}
///////////////////////////////////
//
// DateTime
//
///////////////////////////////////
int rwfToJsonConverter::processDateTime(RsslDecodeIterator *iterPtr)
{
	RsslDateTime dateTimeVal;
	RsslRet retVal;

	rsslClearDateTime(&dateTimeVal);

	if ((retVal = rsslDecodeDateTime(iterPtr, &dateTimeVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
	}
	else
	{
		writeOb();
		writeVar('d',false);
		writeOb();
		writeVar('d', false);
		uInt32ToString(dateTimeVal.date.day);
		writeVar('m', true);
		uInt32ToString(dateTimeVal.date.month);
		writeVar('y', true);
		uInt32ToString(dateTimeVal.date.year);
		writeOe();
		writeVar('t', true);
		writeOb();
		writeVar('h', false);
		uInt32ToString(dateTimeVal.time.hour);
		writeVar('m', true);
		uInt32ToString(dateTimeVal.time.minute);
		writeVar('s', true);
		uInt32ToString(dateTimeVal.time.second);
		writeVar('x', true);
		uInt32ToString(dateTimeVal.time.millisecond);
		writeOe();
		writeOe();
	}
	return 1;
}
///////////////////////////////////
//
// Field List Set Db
//
///////////////////////////////////
int rwfToJsonConverter::processFieldListSetdb(RsslLocalFieldSetDefDb& db)
{

	int setDefCnt = 0;

 	for (int i = 0; i <= RSSL_FIELD_SET_MAX_LOCAL_ID; ++i) 
		if(db.definitions[i].setId != RSSL_FIELD_SET_BLANK_ID)
			setDefCnt++;
	if(setDefCnt == 0)
	{
		writeString("null");
		return 1;
	}
	writeOb();	// Set Defs
	writeVar('c', false);
	int32ToString(setDefCnt);
	writeVar('d', true);
	writeAb();	
	
	bool comma1 = false;
 	for (int i = 0; i <= RSSL_FIELD_SET_MAX_LOCAL_ID; ++i) 
		if(db.definitions[i].setId != RSSL_FIELD_SET_BLANK_ID)
		{
			if (comma1)
				writeComma();
			else
				comma1 = true;
			writeOb();
			writeVar('i', false);
			int32ToString(db.definitions[i].setId);
			writeVar('c', true);
			int32ToString(db.definitions[i].count);
			writeVar('s', true);
			writeAb();
			bool comma2 = false;
			for (int ii = 0; ii < db.definitions[i].count; ++ii)
			{
				if (comma2)
					writeComma();
				else
					comma2 =true;
				writeOb();
				writeVar('f', false);
				int32ToString(db.definitions[i].pEntries[ii].fieldId);
				writeVar('t', true);
				int32ToString(db.definitions[i].pEntries[ii].dataType);
				writeOe();
			}

			writeAe();
			writeOe();
		}
	writeAe();
	writeOe(); // set Defs
	return 1;
}
///////////////////////////////////
//
// Element List Set Db
//
///////////////////////////////////
int rwfToJsonConverter::processElementListSetdb(RsslLocalElementSetDefDb& db)
{

	int setDefCnt = 0;

 	for (int i = 0; i <= RSSL_ELEMENT_SET_MAX_LOCAL_ID; ++i) 
		if(db.definitions[i].setId != RSSL_ELEMENT_SET_BLANK_ID)
			setDefCnt++;
	
	if(setDefCnt == 0)
	{
		writeString("null");
		return 1;
	}
	writeOb();	// Set Defs
	writeVar('c', false);
	int32ToString(setDefCnt);
	writeVar('d', true);
	writeAb();	
	bool comma1 = false;
 	for (int i = 0; i <= RSSL_ELEMENT_SET_MAX_LOCAL_ID; ++i) 
		if(db.definitions[i].setId != RSSL_ELEMENT_SET_BLANK_ID)
		{
			if (comma1)
				writeComma();
			else
				comma1 = true;
			writeOb();
			writeVar('i', false);
			int32ToString(db.definitions[i].setId);
			writeVar('c', true);
			int32ToString(db.definitions[i].count);
			writeVar('s', true);
			writeAb();
			bool comma2 = false;
			for (int ii = 0; ii < db.definitions[i].count; ++ii)
			{
				if (comma2)
					writeComma();
				else
					comma2 =true;
				writeOb();
				writeVar('n', false);
				writeString(db.definitions[i].pEntries[ii].name.data, 
							db.definitions[i].pEntries[ii].name.length);
				writeVar('t', true);
				int32ToString(db.definitions[i].pEntries[ii].dataType);
				writeOe();
			}

			writeAe();
			writeOe();
		}
	writeAe();
	writeOe(); // set Defs
	return 1;
}
///////////////////////////////////
//
// Array
//
///////////////////////////////////
int rwfToJsonConverter::processArray(RsslDecodeIterator *iterPtr)
{

	RsslArray arrayVal;
	RsslBuffer buf;
	RsslRet retVal;
	bool comma = false;

	rsslClearArray(&arrayVal);

	if ((retVal = rsslDecodeArray(iterPtr, &arrayVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
		return 1;
	}
	
	writeOb();
	if (arrayVal.itemLength > 0)
	{
		writeVar('l',comma);
		uInt32ToString(arrayVal.itemLength);
		comma = true;
	}
	writeVar('t', comma);
	if (arrayVal.primitiveType > RSSL_DT_BASE_PRIMITIVE_MAX)
		uInt32ToString(_setToBaseType[arrayVal.primitiveType - RSSL_DT_SET_PRIMITIVE_MIN]);
	else		
		uInt32ToString(arrayVal.primitiveType);
		
	comma = false;
	while ((retVal = rsslDecodeArrayEntry(iterPtr, &buf)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (comma == false)
		{
			comma = true;
			writeVar('d', true);
			writeAb();
		}
		else
		{
			writeComma();
		}
		if (arrayVal.primitiveType < RSSL_DT_SET_PRIMITIVE_MAX)
			if (!processPrimitive(iterPtr, arrayVal.primitiveType))
				return 0;
	}
	if (comma)
		writeAe();
	writeOe();
	return 1;
}

int rwfToJsonConverter::estimateJsonLength(RsslUInt32 rwfLength)
{
	return ((rwfLength * 6) + 300);
}

int rwfToJsonConverter::processJson(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{	
	return 0;
}

