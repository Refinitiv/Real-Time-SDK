/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */


#include <limits.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "rtr/rsslDataPackage.h"
#include "rtr/rtrdiv.h"
#include "rtr/rtrdiv10.h"
#include "rtr/rjcsmallstr.h"

#include "rtr/rwfToJsonSimple.h"
#include "rtr/jsonSimpleDefs.h"

static RsslDouble powHints[] = {0.00000000000001, 0.0000000000001, 0.000000000001, 0.00000000001, 0.0000000001, 0.000000001, 0.00000001, 0.0000001, 0.000001, 0.00001, 0.0001, 0.001, 0.01, 0.1, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 1, 0.5, 0.25, 0.125, 0.0625, 0.03125, 0.015625, 0.0078125, 0.00390625 };

DEV_THREAD_LOCAL rwfToJsonSimple::SetDefDbMem rwfToJsonSimple::_setDefDbMem;

//Use 1 to 3 byte variable UTF encoding
//#define MaxUTF8Bytes 3

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
//////////////////////////////////////////////////////////////////////
rwfToJsonSimple::rwfToJsonSimple(int bufSize, u_16 convFlags)
	: rwfToJsonBase(bufSize, MAX_MSG_SIMPLIFIED_PREQUEL, convFlags)
{
}
//////////////////////////////////////////////////////////////////////
//
// Destructor
//
//////////////////////////////////////////////////////////////////////
rwfToJsonSimple::~rwfToJsonSimple()
{
}

void rwfToJsonSimple::reset()
{
	rwfToJsonBase::reset();
}

const RsslBuffer *rwfToJsonSimple::getJsonMsg(RsslInt32 streamId, bool solicited, bool close)
{
	char streamIdString[20];
	int streamIdStringLen = 0;
	char *msgStart = 0;
	char *pstrSave = _pstr;
	int len;


	*_pstr = 0;		// Just in case someone prints it, won't get copied

	if (!close)
	{
		len = (int)(_pstr - _buf - MAX_MSG_SIMPLIFIED_PREQUEL);
		_pstr = streamIdString;
		int32ToStringOffBuffer(streamId);
		streamIdStringLen = (int)(_pstr - streamIdString);
		_pstr = streamIdString;

		len += JSON_FIXED_SIMPLFIED_PREQUEL + streamIdStringLen;

		msgStart = _buf + MAX_MSG_SIMPLIFIED_PREQUEL - ( JSON_FIXED_SIMPLFIED_PREQUEL + streamIdStringLen);

		_pstr = msgStart;
		_buffer.data = msgStart;
		_buffer.length = len;

		writeOb();
		writeBufVar(&JSON_ID, false);
	
		if (verifyJsonMessageSize(streamIdStringLen) == 0) 
			return 0;
		doSimpleMemCopy(_pstr,streamIdString,streamIdStringLen);
	}
	else // close message, don't alter buffer since stream id(s) already added
	{
		_buffer.data = _buf + MAX_MSG_SIMPLIFIED_PREQUEL;
		_buffer.length = (RsslUInt32)(_pstr - _buffer.data);
		writeOb();
	}
	_pstr = pstrSave;

	return &_buffer;
}

//////////////////////////////////////////////////////////////////////
//
// Message Processors
//
//////////////////////////////////////////////////////////////////////
int rwfToJsonSimple::processMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg, bool first)
{
	if (iMsg.msgBase.msgClass != RSSL_MC_CLOSE)
	{
		if (!first)
		{
			_firstMsg = false;
			// For embedded messages include the entire message base
			writeOb();
			writeBufVar(&JSON_ID, false);
			uInt32ToString(iMsg.msgBase.streamId);
		}
		writeBufVar(&JSON_TYPE, true);

		if (const char* msgType = rsslMsgClassToOmmString(iMsg.msgBase.msgClass))
		{
			writeString(msgType);
		}
		if (iMsg.msgBase.domainType != RSSL_DMT_MARKET_PRICE)
		{
			writeBufVar(&JSON_DOMAIN, true);
			if (const char* domain = rsslDomainTypeToOmmString(iMsg.msgBase.domainType))
				writeString(domain);
			else
				uInt32ToString(iMsg.msgBase.domainType);
		}
	}
	else // close message, everything written by processCloseMsg except writeOb
	{
		writeOb();
	}
	// Call the appropriate message handler based on the msgClass
	if ((this->*_msgHandlers[iMsg.msgBase.msgClass])(iterPtr, iMsg))
	{
		writeOe();
		return 1;
	}
	return 0;

}

int rwfToJsonSimple::processMsg(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslMsg msg;
	RsslRet ret;
	rsslClearMsg(&msg);

	if (writeTag)
		writeBufVar(&JSON_MESSAGE, false);

	if ((ret = rsslDecodeMsg(iterPtr, &msg)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}
	return processMsg(iterPtr, msg, false);
}

///////////////////////////////////
//
// Request Msg Processors
//
///////////////////////////////////
int rwfToJsonSimple::processRequestMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	bool comma = false;

	if (!(rsslRequestMsgCheckStreaming(&iMsg.requestMsg)))
	{
		// Streaming is the default, only send if 0
		writeBufVar(&JSON_STREAMING, true);
		writeJsonBoolString(false);
	}
	if (iMsg.requestMsg.flags != RSSL_RQMF_NONE)
	{
		if (rsslRequestMsgCheckHasPriority(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_PRIORITY, true);
			writeOb();
			writeBufVar(&JSON_CLASS, false);
			uInt32ToString(iMsg.requestMsg.priorityClass);
			writeBufVar(&JSON_COUNT, true);
			uInt32ToString(iMsg.requestMsg.priorityCount);
			writeOe();
		}
		if (!rsslRequestMsgCheckMsgKeyInUpdates(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_KEYINUPDATES, true);
			writeJsonBoolString(false);
		}
		if ((rsslRequestMsgCheckConfInfoInUpdates(&iMsg.requestMsg)))
		{
			writeBufVar(&JSON_CONFINFOINUPDATES, true);
			writeJsonBoolString(true);
		}
		if (rsslRequestMsgCheckNoRefresh(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_REFRESH, true);
			writeJsonBoolString(false);
		}
		if (rsslRequestMsgCheckHasQoS(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_QOS, true);
			if (!processQOS(&iMsg.requestMsg.qos))
				return 0;
		}
		if (rsslRequestMsgCheckHasWorstQoS(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_WORSTQOS, true);
			if (!processQOS(&iMsg.requestMsg.worstQos))
				return 0;
		}
		if (rsslRequestMsgCheckPrivateStream(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_PRIVATE, true);
			writeJsonBoolString(true);
		}
		if (rsslRequestMsgCheckQualifiedStream(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_QUALIFIED, true);
			writeJsonBoolString(true);
		}
		if (rsslRequestMsgCheckPause(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_PAUSE, true);
			writeJsonBoolString(true);
		}
		if (rsslRequestMsgCheckHasExtendedHdr(&iMsg.requestMsg))
		{
			writeBufVar(&JSON_EXTHDR, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.requestMsg.extendedHeader))
				return 0;
		}
	}

	if (!rsslRequestMsgCheckHasBatch(&iMsg.requestMsg))
	{
		writeBufVar(&JSON_KEY, true);
		if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr, iMsg.msgBase.domainType))
			return 0;
		if (!rsslRequestMsgCheckHasView(&iMsg.requestMsg))
		{
			if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
			{
				writeComma();
				return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, 0, true);
			}
			return 1;
		}
	}

	if (iMsg.msgBase.containerType != RSSL_DT_ELEMENT_LIST || iMsg.msgBase.encDataBody.length == 0)
		return 0;

	// If we got here then we have a batch and/or view
	RsslRet retVal = RSSL_RET_FAILURE;
	RsslElementList				elementList;
	RsslElementEntry			elementItem;

	RsslUInt64 viewType = 0;
	RsslBuffer viewDataBuf = RSSL_INIT_BUFFER;
	RsslBuffer itemListBuf = RSSL_INIT_BUFFER;
	RsslBuffer viewTypeBuf = RSSL_INIT_BUFFER;

	rsslClearElementList(&elementList);

	if ((retVal = rsslDecodeElementList(iterPtr, &elementList, 0)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}
	while ((retVal = rsslDecodeElementEntry(iterPtr, &elementItem)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retVal < RSSL_RET_SUCCESS)
		{
			return 0;
		}
		else
		{
			if (strncmp(RSSL_ENAME_VIEW_TYPE.data, elementItem.name.data, elementItem.name.length) == 0) // :ViewType
			{
				rsslDecodeUInt(iterPtr, &viewType);
				viewTypeBuf.data = elementItem.encData.data;
				viewTypeBuf.length = elementItem.encData.length;
			}
			else if (strncmp(RSSL_ENAME_VIEW_DATA.data, elementItem.name.data, elementItem.name.length) == 0) // :ViewData
			{
				viewDataBuf.data = elementItem.encData.data;
				viewDataBuf.length = elementItem.encData.length;
			}
			else if (strncmp(RSSL_ENAME_BATCH_ITEM_LIST.data, elementItem.name.data, elementItem.name.length) == 0)	// :ItemList
			{
				itemListBuf.data = elementItem.encData.data;
				itemListBuf.length = elementItem.encData.length;
			}
		}
	}
	if (rsslRequestMsgCheckHasBatch(&iMsg.requestMsg))
	{
		if (itemListBuf.length == 0)
			return 0;

		// Now do the request key
		writeBufVar(&JSON_KEY, true);
		if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr, iMsg.msgBase.domainType))
			return 0;
		// Back up to remove the closing bracket on the key
		_pstr--;

		RsslDecodeIterator aIter;
		rsslClearDecodeIterator(&aIter);
		rsslSetDecodeIteratorBuffer(&aIter, &itemListBuf);
		RsslArray rsslArray = RSSL_INIT_ARRAY;
		RsslBuffer arrayItem = RSSL_INIT_BUFFER;

		retVal = rsslDecodeArray(&aIter, &rsslArray);
		if (retVal < RSSL_RET_SUCCESS || retVal == RSSL_RET_BLANK_DATA || retVal == RSSL_RET_NO_DATA)
			return 0;

		if (*(_pstr - 1) != '{')
			writeComma();
		writeBufVar(&JSON_NAME, false);
		comma = false;
		writeAb();
		for (int i = 0; true; i++)
		{
			retVal = rsslDecodeArrayEntry(&aIter, &arrayItem);
			if (retVal == RSSL_RET_END_OF_CONTAINER)
				break;
			if (comma)
				writeComma();
			else
				comma = true;
			writeValueDQ(arrayItem.data, arrayItem.length);
		}
		writeAe();  // End of Batch
		writeOe();  // End of Key
	}
	// See if we have a view
	if (viewDataBuf.length == 0)
		return 1;

	// Need both viewType and view
	if (viewTypeBuf.length == 0)
		return 0;

	if (viewType == RDM_VIEW_TYPE_ELEMENT_NAME_LIST)
	{
		writeBufVar(&JSON_VIEWTYPE, true);
		writeBufString(&JSON_VIEW_NAME_LIST);
	}

	RsslDecodeIterator aIter;
	rsslClearDecodeIterator(&aIter);
	rsslSetDecodeIteratorBuffer(&aIter, &viewDataBuf);
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslBuffer arrayItem = RSSL_INIT_BUFFER;

	retVal = rsslDecodeArray(&aIter, &rsslArray);
	if (retVal < RSSL_RET_SUCCESS || retVal == RSSL_RET_BLANK_DATA || retVal == RSSL_RET_NO_DATA)
		return 0;

	writeBufVar(&JSON_VIEW, true);
	comma = false;
	writeAb();
	for (int i = 0; true; i++)
	{
		retVal = rsslDecodeArrayEntry(&aIter, &arrayItem);
		if (retVal == RSSL_RET_END_OF_CONTAINER)
			break;
		if (comma)
			writeComma();
		else
			comma = true;
		if (viewType == RDM_VIEW_TYPE_ELEMENT_NAME_LIST)
			writeValueDQ(arrayItem.data, arrayItem.length);
		else
		{
			RsslInt fid;
			rsslDecodeInt(&aIter, &fid);
			int32ToString((RsslInt32)fid);
		}
	}
	writeAe();  // End of View
	return 1;
}
///////////////////////////////////
//
// Refresh Msg Processors
//
///////////////////////////////////
int rwfToJsonSimple::processRefreshMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	const RsslMsgKey *pKey;

	if ( pKey = rsslGetMsgKey(&iMsg))
	{
		writeBufVar(&JSON_KEY, true);
		if (!processMsgKey(pKey, iterPtr, iMsg.msgBase.domainType))
			return 0;
	}

	// Send state
	writeBufVar(&JSON_STATE, true);
	processState(&iMsg.refreshMsg.state);

	if (iMsg.refreshMsg.flags != RSSL_RFMF_NONE)
	{
		if (iMsg.refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY)
		{
			writeBufVar(&JSON_REQKEY, true);
			if (!processMsgKey(&iMsg.refreshMsg.reqMsgKey, iterPtr, iMsg.msgBase.domainType))
				return 0;
		}
		if (rsslRefreshMsgCheckHasExtendedHdr(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_EXTHDR, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.refreshMsg.extendedHeader))
				return 0;
		}
		if (!(rsslRefreshMsgCheckRefreshComplete(&iMsg.refreshMsg)))
		{
			/* Default is 1 only send false if not complete
			 */
			writeBufVar(&JSON_COMPLETE, true);
			writeJsonBoolString(false);
		}
		if (rsslRefreshMsgCheckHasQoS(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_QOS, true);
			if (!processQOS(&iMsg.refreshMsg.qos))
				return 0;
		}
		if (!rsslRefreshMsgCheckClearCache(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_CLEARCACHE, true);
			writeJsonBoolString(false);
		}
		if (rsslRefreshMsgCheckDoNotCache(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_DONOTCACHE, true);
			writeJsonBoolString(true);
		}
		if (rsslRefreshMsgCheckPrivateStream(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_PRIVATE, true);
			writeJsonBoolString(true);
		}
		if (rsslRefreshMsgCheckQualifiedStream(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_QUALIFIED, true);
			writeJsonBoolString(true);
		}
		if (rsslRefreshMsgCheckHasPostUserInfo(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_POSTUSERINFO, true);
			if (!processPostUserInfo(&iMsg.refreshMsg.postUserInfo))
				return 0;
		}
		if (rsslRefreshMsgCheckHasPartNum(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_PARTNUMBER, true);
			uInt32ToString(iMsg.refreshMsg.partNum);
		}
		if (rsslRefreshMsgCheckHasPermData(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_PERMDATA, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.refreshMsg.permData))
				return 0;
		}
		if (rsslRefreshMsgCheckHasSeqNum(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_SEQNUM, true);
			uInt32ToString(iMsg.refreshMsg.seqNum);
	}
		if (!rsslRefreshMsgCheckSolicited(&iMsg.refreshMsg))
		{
			writeBufVar(&JSON_SOLICITED, true);
			writeJsonBoolString(false);
		}
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeComma();
		return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, 0, true);
	}
	return 1;
}
///////////////////////////////////
//
// Status Msg Processor
//
///////////////////////////////////
int rwfToJsonSimple::processStatusMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	if (iMsg.statusMsg.flags != RSSL_STMF_NONE)
	{
		if (rsslStatusMsgCheckHasMsgKey(&iMsg.statusMsg))
		{
			writeBufVar(&JSON_KEY, true);
			if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr, iMsg.msgBase.domainType))
				return 0;
		}
		if (iMsg.statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY)
		{
			writeBufVar(&JSON_REQKEY, true);
			if (!processMsgKey(&iMsg.statusMsg.reqMsgKey, iterPtr, iMsg.msgBase.domainType))
				return 0;
		}
		if (rsslStatusMsgCheckHasState(&iMsg.statusMsg))
		{
			writeBufVar(&JSON_STATE, true);
			if (!processState(&iMsg.statusMsg.state))
				return 0;
		}
		if (rsslStatusMsgCheckClearCache(&iMsg.statusMsg))
		{
			writeBufVar(&JSON_CLEARCACHE, true);
			writeJsonBoolString(true);

		}
		if (rsslStatusMsgCheckPrivateStream(&iMsg.statusMsg))
		{
			writeBufVar(&JSON_PRIVATE, true);
			writeJsonBoolString(true);
		}
		if (rsslStatusMsgCheckQualifiedStream(&iMsg.statusMsg))
		{
			writeBufVar(&JSON_QUALIFIED, true);
			writeJsonBoolString(true);
		}
		if (rsslStatusMsgCheckHasPostUserInfo(&iMsg.statusMsg))
		{
			writeBufVar(&JSON_POSTUSERINFO, true);
			if (!processPostUserInfo(&iMsg.statusMsg.postUserInfo))
				return 0;
		}
		if (rsslStatusMsgCheckHasExtendedHdr(&iMsg.statusMsg))
		{
			writeBufVar(&JSON_EXTHDR, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.statusMsg.extendedHeader))
				return 0;
		}
		if (rsslStatusMsgCheckHasPermData(&iMsg.statusMsg))
		{
			writeBufVar(&JSON_PERMDATA, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.statusMsg.permData))
				return 0;
	}
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeComma();
		return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, 0, true);
	}
	return 1;
}
///////////////////////////////////
//
// Update Msg Processor
//
///////////////////////////////////
int rwfToJsonSimple::processUpdateMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	writeBufVar(&JSON_UPDATETYPE, true);
	if (const char* updateType = rsslRDMUpdateEventTypeToOmmString(iMsg.updateMsg.updateType))
			writeString(updateType);
	else
		uInt32ToString(iMsg.updateMsg.updateType);

	if (rsslUpdateMsgCheckDoNotConflate(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_DONOTCONFLATE, true);
		writeJsonBoolString(true);
	}
	if (rsslUpdateMsgCheckDoNotRipple(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_DONOTRIPPLE, true);
		writeJsonBoolString(true);
	}
	if (iMsg.updateMsg.flags & RSSL_UPMF_DISCARDABLE)	// Missing RSSL Helper function
	{
		writeBufVar(&JSON_DISCARDABLE, true);
		writeJsonBoolString(true);
	}
	if (rsslUpdateMsgCheckDoNotCache(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_DONOTCACHE, true);
		writeJsonBoolString(true);
	}
	if (rsslUpdateMsgCheckHasMsgKey(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_KEY, true);
		if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr, iMsg.msgBase.domainType))
			return 0;
	}
	if (rsslUpdateMsgCheckHasPostUserInfo(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_POSTUSERINFO, true);
		if (!processPostUserInfo(&iMsg.updateMsg.postUserInfo))
			return 0;
	}
	if (rsslUpdateMsgCheckHasPermData(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_PERMDATA, true);
		if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.updateMsg.permData))
			return 0;
	}
	if (rsslUpdateMsgCheckHasSeqNum(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_SEQNUM, true);
		uInt32ToString(iMsg.updateMsg.seqNum);
	}
	if (rsslUpdateMsgCheckHasConfInfo(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_CONFINFO, true);
		writeOb();
		writeBufVar(&JSON_COUNT, false);
		uInt32ToString(iMsg.updateMsg.conflationCount);
		writeBufVar(&JSON_TIME, true);
		uInt32ToString(iMsg.updateMsg.conflationTime);
		writeOe();
	}
	if (rsslUpdateMsgCheckHasExtendedHdr(&iMsg.updateMsg))
	{
		writeBufVar(&JSON_EXTHDR, true);
		if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.updateMsg.extendedHeader))
			return 0;
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeComma();
		return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, 0, true);
	}
	return 1;
}
///////////////////////////////////
//
// Close Msg Processor
//
///////////////////////////////////
int rwfToJsonSimple::processCloseMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	bool comma = false;

	if (!rsslCloseMsgCheckHasBatch(&iMsg.closeMsg))
	{
		writeBufVar(&JSON_ID, false);
		uInt32ToString(iMsg.msgBase.streamId);
	}
	else // batch close
	{
		// determine if we have element list container type for batch close
		if (iMsg.msgBase.containerType != RSSL_DT_ELEMENT_LIST || iMsg.msgBase.encDataBody.length == 0)
			return 0;

		// If we got here then we have a batch
		RsslRet retVal = RSSL_RET_FAILURE;
		RsslElementList elementList;
		RsslElementEntry elementItem;

		RsslBuffer itemListBuf = RSSL_INIT_BUFFER;

		rsslClearElementList(&elementList);

		if ((retVal = rsslDecodeElementList(iterPtr, &elementList, 0)) < RSSL_RET_SUCCESS)
		{
			return 0;
	}
		while ((retVal = rsslDecodeElementEntry(iterPtr, &elementItem)) != RSSL_RET_END_OF_CONTAINER)
	{
			if (retVal < RSSL_RET_SUCCESS)
			{
				return 0;
	}
			else
	{
				if (strncmp(RSSL_ENAME_BATCH_STREAMID_LIST.data, elementItem.name.data, elementItem.name.length) == 0)	// :StreamIdList
				{
					itemListBuf.data = elementItem.encData.data;
					itemListBuf.length = elementItem.encData.length;
				}
			}
		}

		if (itemListBuf.length == 0)
			return 0;

		RsslDecodeIterator aIter;
		rsslClearDecodeIterator(&aIter);
		rsslSetDecodeIteratorBuffer(&aIter, &itemListBuf);
		RsslArray rsslArray = RSSL_INIT_ARRAY;
		RsslBuffer arrayItem = RSSL_INIT_BUFFER;

		retVal = rsslDecodeArray(&aIter, &rsslArray);
		if (retVal < RSSL_RET_SUCCESS || retVal == RSSL_RET_BLANK_DATA || retVal == RSSL_RET_NO_DATA)
			return 0;

		if (*(_pstr - 1) != '{')
		writeComma();
		writeBufVar(&JSON_ID, false);
		comma = false;
		writeAb();
		for (int i = 0; true; i++)
		{
			retVal = rsslDecodeArrayEntry(&aIter, &arrayItem);
			if (retVal == RSSL_RET_END_OF_CONTAINER)
				break;
			if (comma)
				writeComma();
			else
				comma = true;
			RsslInt fid;
			rsslDecodeInt(&aIter, &fid);
			int32ToString((RsslInt32)fid);
	}
		writeAe();  // End of Batch
	}
	writeBufVar(&JSON_TYPE, true);

	if (const char* msgType = rsslMsgClassToOmmString(iMsg.msgBase.msgClass))
		writeString(msgType);
	else
		uInt32ToString(iMsg.msgBase.msgClass);

	if (iMsg.msgBase.domainType != RSSL_DMT_MARKET_PRICE)
	{
		writeBufVar(&JSON_DOMAIN, true);
		if (const char* domain = rsslDomainTypeToOmmString(iMsg.msgBase.domainType))
			writeString(domain);
		else
			uInt32ToString(iMsg.msgBase.domainType);
	}

	if (rsslCloseMsgCheckAck(&iMsg.closeMsg))
	{
		writeBufVar(&JSON_ACK, true);
		writeJsonBoolString(true);
	}
	if (rsslCloseMsgCheckHasExtendedHdr(&iMsg.closeMsg))
	{
		writeBufVar(&JSON_EXTHDR, true);
		if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.closeMsg.extendedHeader))
			return 0;
	}

	if (!rsslCloseMsgCheckHasBatch(&iMsg.closeMsg) && iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeComma();
		return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, 0, true);
	}

	return 1;
}
///////////////////////////////////
//
// Ack Msg Processor
//
///////////////////////////////////
int rwfToJsonSimple::processAckMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	writeBufVar(&JSON_ACKID, true);
	uInt32ToString(iMsg.ackMsg.ackId);

	if (iMsg.ackMsg.flags != RSSL_AKMF_NONE)
	{
		if (rsslAckMsgCheckHasNakCode(&iMsg.ackMsg))
		{
			writeBufVar(&JSON_NAKCODE, true);
			if (const char* nakCodeString = rsslNakCodeToOmmString(iMsg.ackMsg.nakCode))
				writeSafeString(nakCodeString);
			else
				uInt32ToString(iMsg.ackMsg.nakCode);
		}
		if (rsslAckMsgCheckHasText(&iMsg.ackMsg))
		{
			writeBufVar(&JSON_TEXT, true);
			writeSafeString(iMsg.ackMsg.text.data, iMsg.ackMsg.text.length);
		}
		if (rsslAckMsgCheckPrivateStream(&iMsg.ackMsg))
		{
			writeBufVar(&JSON_PRIVATE, true);
			writeJsonBoolString(true);
		}
		if (rsslAckMsgCheckHasSeqNum(&iMsg.ackMsg))
		{
			writeBufVar(&JSON_SEQNUM, true);
			uInt32ToString(iMsg.ackMsg.seqNum);
		}
		if (rsslAckMsgCheckHasMsgKey(&iMsg.ackMsg))
		{
			writeBufVar(&JSON_KEY, true);
			if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr, iMsg.msgBase.domainType))
				return 0;
		}
		if (rsslAckMsgCheckHasExtendedHdr(&iMsg.ackMsg))
		{
			writeBufVar(&JSON_EXTHDR, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.ackMsg.extendedHeader))
				return 0;
		}
		if (rsslAckMsgCheckQualifiedStream(&iMsg.ackMsg))
		{
			writeBufVar(&JSON_QUALIFIED, true);
			writeJsonBoolString(true);
	}
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeComma();
		return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, 0, true);
	}
	return 1;
}
///////////////////////////////////
//
// Generic Msg Processor
//
///////////////////////////////////
int rwfToJsonSimple::processGenericMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{
	if (iMsg.genericMsg.flags != RSSL_GNMF_NONE)
	{
		if (rsslGenericMsgCheckHasMsgKey(&iMsg.genericMsg))
		{
			writeBufVar(&JSON_KEY, true);
			//Key including service id of generic message should not be touch/modified by TREP
			if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr, iMsg.msgBase.domainType, false))
				return 0;
		}
		if (iMsg.genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY)
		{
			writeBufVar(&JSON_REQKEY, true);
			if (!processMsgKey(&iMsg.genericMsg.reqMsgKey, iterPtr, iMsg.msgBase.domainType))
				return 0;
		}
		if (rsslGenericMsgCheckHasSeqNum(&iMsg.genericMsg))
		{
			writeBufVar(&JSON_SEQNUM, true);
			uInt32ToString(iMsg.genericMsg.seqNum);
		}
		if (rsslGenericMsgCheckHasSecondarySeqNum(&iMsg.genericMsg))
		{
			writeBufVar(&JSON_SECSEQNUM, true);
			uInt32ToString(iMsg.genericMsg.secondarySeqNum);
		}
		if (rsslGenericMsgCheckHasPartNum(&iMsg.genericMsg))
		{
			writeBufVar(&JSON_PARTNUMBER, true);
			uInt32ToString(iMsg.genericMsg.partNum);
		}
		if (rsslGenericMsgCheckHasPermData(&iMsg.genericMsg))
		{
			writeBufVar(&JSON_PERMDATA, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.genericMsg.permData))
				return 0;
		}
		if (rsslGenericMsgCheckHasExtendedHdr(&iMsg.genericMsg))
		{
			writeBufVar(&JSON_EXTHDR, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.genericMsg.extendedHeader))
				return 0;
		}
	}
	if (!(rsslGenericMsgCheckMessageComplete(&iMsg.genericMsg)))
	{
		// Default is complete only send if incomplete
		writeBufVar(&JSON_COMPLETE, true);
		writeJsonBoolString(false);
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeComma();
		return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, 0, true);
	}
	return 1;
}
///////////////////////////////////
//
// Post Msg Processor
//
///////////////////////////////////
int rwfToJsonSimple::processPostMsg(RsslDecodeIterator *iterPtr, RsslMsg &iMsg)
{

	writeBufVar(&JSON_POSTUSERINFO, true);
	if (!processPostUserInfo(&iMsg.postMsg.postUserInfo))
		return 0;

	if (iMsg.postMsg.flags != RSSL_PSMF_POST_COMPLETE)
	{
		if (rsslPostMsgCheckHasPostId(&iMsg.postMsg))
		{
			writeBufVar(&JSON_POSTID, true);
			uInt32ToString(iMsg.postMsg.postId);
		}
		if (rsslPostMsgCheckHasMsgKey(&iMsg.postMsg))
		{
			writeBufVar(&JSON_KEY, true);
			if (!processMsgKey(rsslGetMsgKey(&iMsg), iterPtr, iMsg.msgBase.domainType))
				return 0;
		}
		if (rsslPostMsgCheckHasSeqNum(&iMsg.postMsg))
		{
			writeBufVar(&JSON_SEQNUM, true);
			uInt32ToString(iMsg.postMsg.seqNum);
		}
		if (!rsslPostMsgCheckPostComplete(&iMsg.postMsg))
		{
			// Default is complete only send if incomplete
			writeBufVar(&JSON_COMPLETE, true);
			writeJsonBoolString(false);
		}
		if (rsslPostMsgCheckAck(&iMsg.postMsg))
		{
			writeBufVar(&JSON_ACK, true);
			writeJsonBoolString(true);
		}
		if (rsslPostMsgCheckHasPermData(&iMsg.postMsg))
		{
			writeBufVar(&JSON_PERMDATA, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.postMsg.permData))
				return 0;
		}
		if (rsslPostMsgCheckHasPartNum(&iMsg.postMsg))
		{
			writeBufVar(&JSON_PARTNUMBER, true);
			uInt32ToString(iMsg.postMsg.partNum);
		}
		if (rsslPostMsgCheckHasPostUserRights(&iMsg.postMsg))
		{
			writeBufVar(&JSON_POSTUSERRIGHTS, true);
			uInt32ToString(iMsg.postMsg.postUserRights);
		}
		if (rsslPostMsgCheckHasExtendedHdr(&iMsg.postMsg))
		{
			writeBufVar(&JSON_EXTHDR, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &iMsg.postMsg.extendedHeader))
				return 0;
		}
	}
	if (iMsg.msgBase.containerType != RSSL_DT_NO_DATA)
	{
		writeComma();
		return processContainer(iMsg.msgBase.containerType, iterPtr, &iMsg.msgBase.encDataBody, 0, true);
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
int rwfToJsonSimple::processMsgKey(const RsslMsgKey *keyPtr, RsslDecodeIterator *iterPtr, RsslUInt8 domain, bool wantServiceName)
{
	bool comma = false;
	const char* nameTypeString = 0;

	if (keyPtr->flags == RSSL_MKF_NONE)
	{
		writeOb();
		writeOe();
		return 1;
	}

	writeOb();
	if (rsslMsgKeyCheckHasServiceId((RsslMsgKey*)keyPtr))
	{
		writeBufVar(&JSON_KEY_SERVICE, comma);
		if (wantServiceName && _rsslServiceIdToNameCallback)
		{
			RsslBuffer serviceName;
			if (_rsslServiceIdToNameCallback(&serviceName, _closure, keyPtr->serviceId) == RSSL_RET_SUCCESS)
				writeString(serviceName.data, serviceName.length);
			else
				uInt32ToString(keyPtr->serviceId);
		}
		else
			uInt32ToString(keyPtr->serviceId);
		comma = true;
	}

	if (rsslMsgKeyCheckHasNameType((RsslMsgKey*)keyPtr) && keyPtr->nameType != 1)
	{
		writeBufVar(&JSON_KEY_NAME_TYPE, comma);
		if (domain ==  RSSL_DMT_LOGIN)
		{
			nameTypeString = rsslRDMLoginUserIdTypeToOmmString(keyPtr->nameType);
		}
		else
			nameTypeString = rsslRDMInstrumentNameTypeToOmmString(keyPtr->nameType);
		if (nameTypeString)
			writeString(nameTypeString);
		else
			uInt32ToString(keyPtr->nameType);

		if (!comma)
			comma = true;
	}
	if (rsslMsgKeyCheckHasName((RsslMsgKey*)keyPtr))
	{
		// workaround for RSSL requiring name if nametype is provided
		// Skip the name if nameType is COOKIE and name is NULL,
		if (!(keyPtr->nameType == RDM_LOGIN_USER_COOKIE || keyPtr->nameType == 5))
		{
			writeBufVar(&JSON_NAME, comma);
			if (keyPtr->name.length == 0 ||	(keyPtr->name.length == 1 && *keyPtr->name.data == 0))
				writeNull();
			else
				writeString(keyPtr->name.data, keyPtr->name.length);
			if (!comma)
				comma = true;
		}
	}
	if (rsslMsgKeyCheckHasFilter((RsslMsgKey*)keyPtr))
	{
		writeBufVar(&JSON_KEY_FILTER, comma);
		uInt32ToString(keyPtr->filter);
		if (!comma)
			comma = true;
	}
	if (rsslMsgKeyCheckHasIdentifier((RsslMsgKey*)keyPtr))
	{
		writeBufVar(&JSON_KEY_IDENTIFIER, comma);
		uInt32ToString(keyPtr->identifier);
		if (!comma)
			comma = true;
	}

	if (rsslMsgKeyCheckHasAttrib((RsslMsgKey*)keyPtr))
	{
		RsslDecodeIterator dIter;
		RsslBuffer buf;
		buf.data = keyPtr->encAttrib.data;
		buf.length = keyPtr->encAttrib.length;
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorBuffer(&dIter, &buf);
		if (comma)
			writeComma();
		if (!processContainer(keyPtr->attribContainerType, &dIter, &keyPtr->encAttrib, 0, true))
			return 0;
		//		writeOe(); // End Key Attrib
	}
	writeOe();
	return 1;
}
///////////////////////////////////
//
// QOS Processor
//
///////////////////////////////////
int rwfToJsonSimple::processQOS(const RsslQos* qosPtr)
{
	writeOb();

	writeBufVar(&JSON_TIMELINESS, false);
	writeString(rsslQosTimelinessToOmmString(qosPtr->timeliness));
	writeBufVar(&JSON_RATE, true);
	writeString(rsslQosRateToOmmString(qosPtr->rate));

	if (qosPtr->dynamic)
	{
		writeBufVar(&JSON_DYNAMIC, true);
		writeJsonBoolString(true);
	}
	if (qosPtr->timeliness == RSSL_QOS_TIME_DELAYED)
	{
		writeBufVar(&JSON_TIMEINFO, true);
		uInt32ToString(qosPtr->timeInfo);
	}
	if (qosPtr->rate  == RSSL_QOS_RATE_TIME_CONFLATED)
	{
		writeBufVar(&JSON_RATEINFO, true);
		uInt32ToString(qosPtr->rateInfo);
	}

	writeOe();
	return 1;
}
///////////////////////////////////
//
// Post User Info  Processors
//
///////////////////////////////////
int rwfToJsonSimple::processPostUserInfo(RsslPostUserInfo* poiPtr)
{
	writeOb();
	writeBufVar(&JSON_ADDRESS, false);
	char postUserAddress[16];
	rsslIPAddrUIntToString(poiPtr->postUserAddr, postUserAddress);
	writeString(postUserAddress);
	writeBufVar(&JSON_USERID, true);
	uInt32ToString(poiPtr->postUserId);
	writeOe();
	return 1;
}
///////////////////////////////////
//
// Post User Info  Processors
//
///////////////////////////////////
int rwfToJsonSimple::processState(const RsslState* statePtr)
{
	writeOb();

	writeBufVar(&JSON_STREAM, false);
	writeString(rsslStreamStateToOmmString(statePtr->streamState));
	writeBufVar(&JSON_DATA, true);
	writeString(rsslDataStateToOmmString(statePtr->dataState));
	if (statePtr->code > RSSL_SC_NONE)
	{
		writeBufVar(&JSON_CODE, true);
		writeString(rsslStateCodeToOmmString(statePtr->code));
	}


	if (statePtr->text.length > 0)
	{
		writeBufVar(&JSON_TEXT, true);
		writeJsonString('"');
		for(int i = 0; i < (int)statePtr->text.length; i++) 
		{
			switch(statePtr->text.data[i])
			{
			case '\"':
				writeJsonString("\\\"", 2);
				break;
			case '\\':
				writeJsonString("\\\\", 2);
				break;
			default:
				if (statePtr->text.data[i] < ' ' || statePtr->text.data[i] == 0x7F )
				{
					if (verifyJsonMessageSize(6) == 0) return 0;
					_pstr += sprintf(_pstr, "\\u%04x", statePtr->text.data[i]);
				}
				else
					writeJsonString(statePtr->text.data[i]);
				break;
			}
		}
		writeJsonString('"');
	}
	writeOe();
	return 1;
}
//////////////////////////////////////////////////////////////////////
//
// Container Processors
//
//////////////////////////////////////////////////////////////////////
int rwfToJsonSimple::processContainer(RsslUInt8 containerType, RsslDecodeIterator *iterPtr,const RsslBuffer *encDataBufPtr, void * setDb, bool writeTag)
{
	if (containerType > RSSL_DT_NO_DATA &&
		containerType <= RSSL_DT_CONTAINER_TYPE_MAX &&
		_containerHandlers[containerType - RSSL_DT_CONTAINER_TYPE_MIN])
	{
		return (this->*_containerHandlers[containerType - RSSL_DT_CONTAINER_TYPE_MIN])(iterPtr, encDataBufPtr, setDb, writeTag);
	}
	return 0;
}
///////////////////////////////////
//
// Opaque Container Processor
//
///////////////////////////////////
int rwfToJsonSimple::processOpaque(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	if (writeTag)
		writeBufVar(&JSON_OPAQUE, false);

	return rwfToJsonBase::processOpaque(iterPtr, encDataBufPtr, setDb, writeTag);
}
///////////////////////////////////
//
// XML Container Processor
//
///////////////////////////////////
int rwfToJsonSimple::processXml(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	if (writeTag)
		writeBufVar(&JSON_XML, false);

	return rwfToJsonBase::processXml(iterPtr, encDataBufPtr, setDb, writeTag);
}
///////////////////////////////////
//
// Field List Format Processor
//
///////////////////////////////////
int rwfToJsonSimple::processFieldList(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslLocalFieldSetDefDb *fieldSetDefDb = (RsslLocalFieldSetDefDb*)setDb ;
	const RsslDictionaryEntry *def;
	RsslFieldList fieldList;
	RsslFieldEntry field;

 	RsslRet retVal = 0;
	bool comma = false;

	if (writeTag)
		writeBufVar(&JSON_FIELDS, false);

	if (encDataBufPtr->length == 0)
	{
		writeEmptyObject();
		return 1;
	}

	rsslClearFieldList(&fieldList);

	if ((retVal = rsslDecodeFieldList(iterPtr, &fieldList, fieldSetDefDb)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}

	writeOb();	// Begin of Field List

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
				writeBufVar(&def->acronym, inner);
				if (!inner)
					inner = true;
				if (def->rwfType < RSSL_DT_SET_PRIMITIVE_MAX)
				{
					if (def->rwfType != RSSL_DT_ENUM || ((_convFlags & EnumExpansionFlag) == 0))
					{
						if (!processPrimitive(iterPtr, def->rwfType))
							return 0;
					}
					else
					{
						if (!processEnumerationExpansion(iterPtr, def))
							return 0;
					}
				}
				else
				{
					if (!processContainer(def->rwfType, iterPtr, &field.encData, 0, false))
						return 0;
				}
			}
		}
		if (!inner && def)
			inner = true;
	}

	writeOe();	// End of Field List

	return 1;
}
///////////////////////////////////
//
// Element List Container Processor
//
///////////////////////////////////
int rwfToJsonSimple::processElementList(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslElementList				elementList;
	RsslElementEntry			elementEntry;
	RsslLocalElementSetDefDb	*elementSetDefDb = (RsslLocalElementSetDefDb*)setDb;
	bool comma = false;
 	RsslRet retVal = 0;
	int inner = false;

	if (writeTag)
		writeBufVar(&JSON_ELEMENTS, false);

	if (encDataBufPtr->length == 0)
	{
		writeEmptyObject();
		return 1;
	}

	rsslClearElementList(&elementList);

	if ((retVal = rsslDecodeElementList(iterPtr, &elementList, elementSetDefDb)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}
	inner = false;
	writeOb();

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
				writeJsonString(_COMMA_CHAR);
			else
				inner = true;

				if (elementEntry.dataType == RSSL_DT_ASCII_STRING &&
				elementEntry.encData.length > 0)
			{
				writeString(elementEntry.name.data, elementEntry.name.length);
				writeJsonString(_COLON_CHAR);
				if (!processPrimitive(iterPtr, elementEntry.dataType))
					return 0;
			}
			else if (elementEntry.dataType == RSSL_DT_UINT &&
					 elementEntry.encData.length > 0)
			{
				writeString(elementEntry.name.data, elementEntry.name.length);
				writeJsonString(_COLON_CHAR);
				if (!processPrimitive(iterPtr, elementEntry.dataType))
					return 0;
			}
			else
			{
				writeString(elementEntry.name.data, elementEntry.name.length);
				writeJsonString(_COLON_CHAR);
				writeOb();	// Begin Element
				writeBufVar(&JSON_TYPE, false);
				writeString(rsslDataTypeToOmmString(elementEntry.dataType));
				writeBufVar(&JSON_DATA, true);

				if (elementEntry.dataType < RSSL_DT_SET_PRIMITIVE_MAX)
				{
					if (!processPrimitive(iterPtr, elementEntry.dataType))
						return 0;
				}
				else if (elementEntry.dataType > RSSL_DT_NO_DATA)
				{
					if (!processContainer(elementEntry.dataType, iterPtr, &elementEntry.encData, 0, false))
						return 0;
				}
				writeOe();	// End of Element
			}
		}
	}

	writeOe();  // End of ElementList

	return 1;
}
///////////////////////////////////
//
// Filter List Container Processor
//
///////////////////////////////////
int rwfToJsonSimple::processFilterList(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslFilterList filterList;
	RsslFilterEntry filterEntry;
	RsslUInt8 entryContainerType;
 	RsslRet retVal = 0;
	bool comma = false;

	if (writeTag)
		writeBufVar(&JSON_FILTERLIST, false);

	if (encDataBufPtr->length == 0)
	{
		writeEmptyObject();
		return 1;
	}

	rsslClearFilterList(&filterList);
	if ((retVal = rsslDecodeFilterList(iterPtr, &filterList)) < RSSL_RET_SUCCESS)
	{
		return 0;
	}

	writeOb();	// Begin of FilterList

	if (rsslFilterListCheckHasTotalCountHint(&filterList))
	{
		writeBufVar(&JSON_COUNTHINT, false);
		int32ToString(filterList.totalCountHint);
		comma = true;
	}

	// Data
	if (filterList.encEntries.length == 0)
	{
		// Valid FilterList without any payload
		writeOe();  // End of FilterList (No data)
		return 1;
	}

	writeBufVar(&JSON_ENTRIES, comma);
	comma = false;

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

		if (!filterEntry.action)
			return 0;

		writeBufVar(&JSON_ID, false);
		int32ToString(filterEntry.id);
		writeBufVar(&JSON_ACTION, true);
		writeString(rsslFilterEntryActionToOmmString(filterEntry.action));

		if (rsslFilterEntryCheckHasContainerType(&filterEntry))
			entryContainerType = filterEntry.containerType;

		if (rsslFilterEntryCheckHasPermData(&filterEntry))
		{
			writeBufVar(&JSON_PERMDATA, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &filterEntry.permData))
				return 0;
		}

		if (entryContainerType > RSSL_DT_NO_DATA)
		{
			writeComma();
			if (!processContainer(entryContainerType, iterPtr, &filterEntry.encData, 0, true))
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
int rwfToJsonSimple::processVector(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
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
	if (writeTag)
		writeBufVar(&RSSL_OMMSTR_DT_VECTOR, false);

	writeOb();	// Begin of Vector
	if (!(vector.flags & RSSL_VTF_NONE))
	{
		if (rsslVectorCheckHasSetDefs(&vector))
		{
			if (vector.containerType == RSSL_DT_FIELD_LIST)
			{
				rsslClearLocalFieldSetDefDb(&fieldSetDb);
				if (_setDefDbMem.inUse < 15)
				{
					fieldSetDb.entries.data = _setDefDbMem.setDefDbMem[_setDefDbMem.inUse];
					_setDefDbMem.inUse += 1;
				}
				else
					return 0;
				fieldSetDb.entries.length = 4096;

				if (rsslDecodeLocalFieldSetDefDb(iterPtr, &fieldSetDb) < RSSL_RET_SUCCESS)
					return 0;
				
				localSetDb = &fieldSetDb;
			}
			else if (vector.containerType == RSSL_DT_ELEMENT_LIST)
			{
				rsslClearLocalElementSetDefDb(&elementSetDb);
				if (_setDefDbMem.inUse < 15)
				{
					elementSetDb.entries.data = _setDefDbMem.setDefDbMem[_setDefDbMem.inUse];
					_setDefDbMem.inUse += 1;
				}
				else
					return 0;
				elementSetDb.entries.length = 4096;

				if (rsslDecodeLocalElementSetDefDb(iterPtr, &elementSetDb) < RSSL_RET_SUCCESS)
					return 0;

				localSetDb = &elementSetDb;
			}
		}
		if (rsslVectorCheckHasSummaryData(&vector) && vector.containerType > RSSL_DT_NO_DATA)
		{
			writeBufVar(&JSON_SUMMARY, false);
			writeOb();  // Begin of Summary data
			if (!processContainer(vector.containerType, iterPtr, &vector.encSummaryData, localSetDb, true))
				return 0;
			writeOe();  // End of Summary data
			comma = true;
		}
		if (rsslVectorCheckHasTotalCountHint(&vector))
		{
			writeBufVar(&JSON_COUNTHINT, comma);
			int32ToString(vector.totalCountHint);
			comma = true;
		}
		if (rsslVectorCheckSupportsSorting(&vector))
		{
			writeBufVar(&JSON_SUPPORTSORTING, comma);
			writeJsonBoolString(true);
			comma = true;
		}
	}
	if (vector.encEntries.length == 0)
	{
		writeOe();  // End of Vector (No data)
		if (rsslVectorCheckHasSetDefs(&vector))
		{
			_setDefDbMem.inUse -= 1;
		}
		return 1;
	}

	writeBufVar(&JSON_ENTRIES, comma);

	comma = false;
	writeAb();	// Begin of Data
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

		writeOb();  // Begin of Vector Entry

		writeBufVar(&JSON_INDEX, false);
		uInt32ToString(vectorEntry.index);

		writeBufVar(&JSON_ACTION, true);
		writeString(rsslVectorEntryActionToOmmString(vectorEntry.action));

		if (rsslVectorEntryCheckHasPermData(&vectorEntry))
		{
			writeBufVar(&JSON_PERMDATA, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &vectorEntry.permData))
				return 0;
		}
		if (vector.containerType > RSSL_DT_NO_DATA && vectorEntry.encData.length > 0)
		{
			writeComma();
			if (!processContainer(vector.containerType, iterPtr, &vectorEntry.encData, localSetDb, true))
				return 0;
		}

		writeOe();  // End of Vector Entry
	}
	writeAe();	// End of Data
	writeOe();  // End of Vector

	if (rsslVectorCheckHasSetDefs(&vector))
	{
		_setDefDbMem.inUse -= 1;
	}

	return 1;
}
///////////////////////////////////
//
// Map Container Processor
//
///////////////////////////////////
int rwfToJsonSimple::processMap(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
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
	if (writeTag)
		writeBufVar(&JSON_MAP, false);

	writeOb();	// Begin of Map
	writeBufVar(&JSON_KEYTYPE, false);
	writeString(rsslDataTypeToOmmString(map.keyPrimitiveType));

	if (!(map.flags & RSSL_MPF_NONE))
	{
		if (rsslMapCheckHasSetDefs(&map))
		{
			if (map.containerType == RSSL_DT_FIELD_LIST)
			{
				rsslClearLocalFieldSetDefDb(&fieldSetDb);
				if (_setDefDbMem.inUse < 15)
				{
					fieldSetDb.entries.data = _setDefDbMem.setDefDbMem[_setDefDbMem.inUse];
					_setDefDbMem.inUse += 1;
				}
				else
					return 0;
				fieldSetDb.entries.length = 4096;

				if (rsslDecodeLocalFieldSetDefDb(iterPtr, &fieldSetDb) < RSSL_RET_SUCCESS)
					return 0;

				localSetDb = &fieldSetDb;
			}
			else if (map.containerType == RSSL_DT_ELEMENT_LIST)
			{
				rsslClearLocalElementSetDefDb(&elementSetDb);
				if (_setDefDbMem.inUse < 15)
				{
					elementSetDb.entries.data = _setDefDbMem.setDefDbMem[_setDefDbMem.inUse];
					_setDefDbMem.inUse += 1;
				}
				else
					return 0;
				elementSetDb.entries.length = 4096;

				if (rsslDecodeLocalElementSetDefDb(iterPtr, &elementSetDb) < RSSL_RET_SUCCESS)
					return 0;

				localSetDb = &elementSetDb;
			}
		}
		if (rsslMapCheckHasSummaryData(&map) && map.containerType > RSSL_DT_NO_DATA)
		{
			writeBufVar(&JSON_SUMMARY, true);
			writeOb();
			if (!processContainer(map.containerType, iterPtr, &map.encSummaryData, localSetDb, true))
				return 0;
			writeOe();
		}
		if (rsslMapCheckHasTotalCountHint(&map))
		{
			writeBufVar(&JSON_COUNTHINT, true);
			int32ToString(map.totalCountHint);
		}
		if (rsslMapCheckHasKeyFieldId(&map))
		{
			writeBufVar(&JSON_KEYFIELDID, true);
			int32ToString(map.keyFieldId);
		}
	}
	if (map.encEntries.length == 0)
	{
		writeOe();  // End of Map (No data)
		if (rsslMapCheckHasSetDefs(&map))
		{
			_setDefDbMem.inUse -= 1;
		}
		return 1;
	}

	writeBufVar(&JSON_ENTRIES, true);

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

		if (!mapEntry.action)
			return 0;

		writeBufVar(&JSON_ACTION, false);
		writeString(rsslMapEntryActionToOmmString(mapEntry.action));

		writeBufVar(&JSON_KEY, true);
		if (map.keyPrimitiveType < RSSL_DT_SET_PRIMITIVE_MAX)
			if (!processPrimitive(map.keyPrimitiveType, &mapEntry.encKey))
				return 0;

		if (rsslMapEntryCheckHasPermData(&mapEntry))
		{
			writeBufVar(&JSON_PERMDATA, true);
			if (!processPrimitive(RSSL_DT_BUFFER, &mapEntry.permData))
				return 0;
		}
		if (mapEntry.encData.length > 0)
		{
			writeComma();
			if (!processContainer(map.containerType, iterPtr, &mapEntry.encData, localSetDb, true))
				return 0;
		}
		writeOe();  // End of Map Entry
	}
	writeAe();	// End of Data
	writeOe();	// End of Map

	if (rsslMapCheckHasSetDefs(&map))
	{
		_setDefDbMem.inUse -= 1;
	}

	return 1;
}
///////////////////////////////////
//
// Series Container Processor
//
///////////////////////////////////
int rwfToJsonSimple::processSeries(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	RsslSeries series;
	RsslSeriesEntry seriesEntry;
	void *localSetDb = 0;
	RsslLocalFieldSetDefDb	 fieldSetDb;
	RsslLocalElementSetDefDb elementSetDb;
 	RsslRet retVal = 0;
	rsslClearSeries(&series);
	bool comma = false;

	// Check for empty series
	if ((retVal = rsslDecodeSeries(iterPtr, &series)) < RSSL_RET_SUCCESS)
	{
	return 0;
}

	if (writeTag)
		writeBufVar(&JSON_SERIES, false);

	writeOb();	// Begin of Series

	if (!(series.flags & RSSL_SRF_NONE))
	{
		if (rsslSeriesCheckHasSetDefs(&series))
		{
			if (series.containerType == RSSL_DT_FIELD_LIST)
			{
				rsslClearLocalFieldSetDefDb(&fieldSetDb);
				if (_setDefDbMem.inUse < 15)
				{
					fieldSetDb.entries.data = _setDefDbMem.setDefDbMem[_setDefDbMem.inUse];
					_setDefDbMem.inUse += 1;
				}
				else
					return 0;
				fieldSetDb.entries.length = 4096;

				if (rsslDecodeLocalFieldSetDefDb(iterPtr, &fieldSetDb) < RSSL_RET_SUCCESS)
					return 0;

				localSetDb = &fieldSetDb;
			}
			else if (series.containerType == RSSL_DT_ELEMENT_LIST)
			{
				rsslClearLocalElementSetDefDb(&elementSetDb);
				if (_setDefDbMem.inUse < 15)
				{
					elementSetDb.entries.data = _setDefDbMem.setDefDbMem[_setDefDbMem.inUse];
					_setDefDbMem.inUse += 1;
				}
				else
					return 0;
				elementSetDb.entries.length = 4096;

				if (rsslDecodeLocalElementSetDefDb(iterPtr, &elementSetDb) < RSSL_RET_SUCCESS)
					return 0;

				localSetDb = &elementSetDb;
			}
		}
		if (rsslSeriesCheckHasSummaryData(&series))
		{
			writeBufVar(&JSON_SUMMARY, false);
			writeOb();
			if (!processContainer(series.containerType, iterPtr, &series.encSummaryData, localSetDb, true))
				return 0;
			writeOe();
			comma = true;
		}
		if (rsslSeriesCheckHasTotalCountHint(&series))
		{
			writeBufVar(&JSON_COUNTHINT, comma);
			int32ToString(series.totalCountHint);
			comma = true;
		}
	}
	if (series.encEntries.length == 0)
	{
		writeOe();  // End of Series (No data)

		if (rsslSeriesCheckHasSetDefs(&series))
		{
			_setDefDbMem.inUse -= 1;
		}
		return 1;
	}

	writeBufVar(&JSON_ENTRIES, comma);
	writeAb();	// Begin of Data
	comma = false;
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

		writeOb();
		if (!processContainer(series.containerType, iterPtr, &seriesEntry.encData, localSetDb, true))
			return 0;
		writeOe();
	}
	writeAe();	// End of Data
	writeOe();	// End of Series

	if (rsslSeriesCheckHasSetDefs(&series))
	{
		_setDefDbMem.inUse -= 1;
	}
	return 1;
}

///////////////////////////////////
//
// OMM-JSON Container Processor
//
///////////////////////////////////
int rwfToJsonSimple::processJson(RsslDecodeIterator *iterPtr, const RsslBuffer *encDataBufPtr, void *setDb, bool writeTag)
{
	if (writeTag)
		writeBufVar(&RSSL_OMMSTR_DT_JSON, false);

	if (encDataBufPtr->length == 0)
		writeNull();
	else
	{
		jsmn_parser jsmnParser;
		while (true)
		{
			jsmn_init(&jsmnParser);
			jsmnerr_t ret = jsmn_parse(&jsmnParser, encDataBufPtr->data,  encDataBufPtr->length, _tokens, _numTokens);
			if ( ret < JSMN_SUCCESS)
			{
				if ( ret == JSMN_ERROR_NOMEM )
				{
				  	free(_tokens);
					_numTokens += _incSize;
					if ((_tokens = (jsmntok_t*)malloc(_numTokens * sizeof(jsmntok_t))) == NULL)
						return 0;
					continue;
				}
				else
					return 0;
			}
			else
				break;
		}

		writeJsonString(encDataBufPtr->data, encDataBufPtr->length);
	}

	return 1;
}

///////////////////////////////////
//
// Message Container Processor
//
///////////////////////////////////
int rwfToJsonSimple::processReal(RsslDecodeIterator *iterPtr)
{
	RsslReal realVal;
	RsslRet retVal;
	RsslBuffer rsslBuf;
	char buf[50];

	rsslClearReal(&realVal);

	if ((retVal = rsslDecodeReal(iterPtr, &realVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA || realVal.isBlank)
	{
		writeNull();
		return 1;
	}

	//Fraction and Inf/-Inf/Nan
	if (realVal.hint > RSSL_RH_MIN_DIVISOR)
	{
		if (realVal.hint <= RSSL_RH_MAX_DIVISOR)
		{
			RsslDouble doubleVal = (realVal.value * powHints[realVal.hint]);

			doubleToStr(doubleVal);
		}
		else
		{
			switch(realVal.hint)
			{
				case RSSL_RH_INFINITY:
					writeString("Inf");
			        break;
				case RSSL_RH_NEG_INFINITY:
					writeString("-Inf");
					break;
				case RSSL_RH_NOT_A_NUMBER:
					writeString("NaN");
					break;
				default:
					return 0;
			}
		}
	}
	else if ((_convFlags & EncodeRealAsPrimitive) == 0)
	{
		rsslBuf.data = buf;
		rsslBuf.length = 50;
		rsslRealToString(&rsslBuf, &realVal);
		if (verifyJsonMessageSize(rsslBuf.length) == 0) return 0;
		memcpy(_pstr, buf, rsslBuf.length);
		_pstr += rsslBuf.length;
	}
	else
	{
		int64ToString(realVal.value);
		if( realVal.hint != RSSL_RH_EXPONENT0)
		{
			writeJsonString('e');
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

int rwfToJsonSimple::processDate(RsslDecodeIterator *iterPtr)
{
	RsslDateTime dateTimeVal;
	RsslBuffer rsslBuf;
	RsslRet retVal;
	char buf[50];

	rsslClearDateTime(&dateTimeVal);

	if ((retVal = rsslDecodeDate(iterPtr, &dateTimeVal.date)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
	}
	else
	{
		writeJsonString(_DOUBLE_QUOTE_CHAR);
		rsslBuf.data = buf;
		rsslBuf.length = 50;
		retVal = rsslDateTimeToStringFormat(&rsslBuf, RSSL_DT_DATE, &dateTimeVal, RSSL_STR_DATETIME_ISO8601);

		if ( retVal == RSSL_RET_SUCCESS)
		{
			if (verifyJsonMessageSize(rsslBuf.length) == 0) return 0;
			memcpy(_pstr, buf, rsslBuf.length);
			_pstr += rsslBuf.length;
		}
		else
		{
			return 0;
		}
		writeJsonString(_DOUBLE_QUOTE_CHAR);
	}
	return 1;
}
///////////////////////////////////
//
// Time
//
///////////////////////////////////

int rwfToJsonSimple::processTime(RsslDecodeIterator *iterPtr)
{
	RsslDateTime dateTimeVal;
	RsslBuffer rsslBuf;
	RsslRet retVal;
	char buf[50];

	rsslClearDateTime(&dateTimeVal);

	if ((retVal = rsslDecodeTime(iterPtr, &dateTimeVal.time)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
	}
	else
	{
		writeJsonString(_DOUBLE_QUOTE_CHAR);
		rsslBuf.data = buf;
		rsslBuf.length = 50;
		retVal = rsslDateTimeToStringFormat(&rsslBuf, RSSL_DT_TIME, &dateTimeVal, RSSL_STR_DATETIME_ISO8601);

		if ( retVal == RSSL_RET_SUCCESS)
		{
			if (verifyJsonMessageSize(rsslBuf.length) == 0) return 0;
			memcpy(_pstr, buf, rsslBuf.length);
			_pstr += rsslBuf.length;
		}
		else
		{
			return 0;
		}
		writeJsonString(_DOUBLE_QUOTE_CHAR);
	}
	return 1;
}
///////////////////////////////////
//
// DateTime
//
///////////////////////////////////
int rwfToJsonSimple::processDateTime(RsslDecodeIterator *iterPtr)
{
	RsslDateTime dateTimeVal;
	RsslBuffer rsslBuf;
	RsslRet retVal;
	char buf[50];

	rsslClearDateTime(&dateTimeVal);

	if ((retVal = rsslDecodeDateTime(iterPtr, &dateTimeVal)) < RSSL_RET_SUCCESS)
		return 0;

	if (retVal == RSSL_RET_BLANK_DATA)
	{
		writeNull();
	}
	else
	{
		writeJsonString(_DOUBLE_QUOTE_CHAR);
		rsslBuf.data = buf;
		rsslBuf.length = 50;
		retVal = rsslDateTimeToStringFormat(&rsslBuf, RSSL_DT_DATETIME, &dateTimeVal, RSSL_STR_DATETIME_ISO8601);

		if ( retVal == RSSL_RET_SUCCESS)
		{
			if (verifyJsonMessageSize(rsslBuf.length) == 0) return 0;
			memcpy(_pstr, buf, rsslBuf.length);
			_pstr += rsslBuf.length;
		}
		else
		{
			return 0;
		}
		writeJsonString(_DOUBLE_QUOTE_CHAR);
	}
	return 1;
}
///////////////////////////////////
//
// QOS
//
///////////////////////////////////
int rwfToJsonSimple::processQOS(RsslDecodeIterator *iterPtr)
{
	RsslQos qosVal;
	RsslRet retVal;


	rsslClearQos(&qosVal);

	if ((retVal = rsslDecodeQos(iterPtr, &qosVal)) < RSSL_RET_SUCCESS)
		return 0;
	return processQOS(&qosVal);

}
///////////////////////////////////
//
// State
//
///////////////////////////////////
int rwfToJsonSimple::processState(RsslDecodeIterator *iterPtr)
{
	RsslState stateVal;
	RsslRet retVal;

	rsslClearState(&stateVal);

	if ((retVal = rsslDecodeState(iterPtr, &stateVal)) < RSSL_RET_SUCCESS)
		return 0;
	return processState(&stateVal);
}

///////////////////////////////////
//
// Enumeration
//
///////////////////////////////////
int rwfToJsonSimple::processEnumeration(RsslDecodeIterator *iterPtr)
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
// Expanded Enumeration
//
///////////////////////////////////
int rwfToJsonSimple::processEnumerationExpansion(RsslDecodeIterator *iterPtr, const RsslDictionaryEntry *def)
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

	const RsslEnumType* pEnumType = (def->pEnumTypeTable && enumVal <= def->pEnumTypeTable->maxValue) ? def->pEnumTypeTable->enumTypes[enumVal] : NULL;

	if (pEnumType != NULL && pEnumType->display.data != NULL)
	{
		rmtesToUtf8(pEnumType->display);
	}
	else
		uInt32ToString(enumVal);

	return 1;
}
///////////////////////////////////
//
// Array
//
///////////////////////////////////
int rwfToJsonSimple::processArray(RsslDecodeIterator *iterPtr)
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

	writeBufVar(&JSON_TYPE, false);
	writeString(rsslDataTypeToOmmString(arrayVal.primitiveType));

	comma = false;
	writeBufVar(&JSON_DATA, true);
	writeAb();
	while ((retVal = rsslDecodeArrayEntry(iterPtr, &buf)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (comma == false)
		{
			comma = true;
		}
		else
		{
			writeComma();
		}
		if (arrayVal.primitiveType < RSSL_DT_SET_PRIMITIVE_MAX)
			if (!processPrimitive(iterPtr, arrayVal.primitiveType))
				return 0;
	}

	writeAe();
	writeOe();
	return 1;
}

int rwfToJsonSimple::estimateJsonLength(RsslUInt32 rwfLength)
{
	return ((rwfLength * 6) + 300);
}

const RsslBuffer *rwfToJsonSimple::generateErrorMessage(char *_errorText, const char *_errorFile, RsslInt32 *_errorLine, RsslInt32 *_errorOffset, RsslBuffer *_errorOriginalMessage, RsslInt32 streamId)
{
	int charsToEscape = 0;

	// Count up all escaped characters
	if (_errorOriginalMessage)
	{
		for(int i = 0; i < (int)_errorOriginalMessage->length; i++) {
			switch(_errorOriginalMessage->data[i])
			{
			case '\"':
			case '\\':
			case '\n':
			case '\t':
				// Account for 1 char \, ", n, or t to escape, example "\n"
				charsToEscape++;
				break;
			default:
				if (_errorOriginalMessage->data[i] < ' ' || _errorOriginalMessage->data[i] == 0x7F )
				{
					// Account for 5 chars "u0000" to escape, example "\u0000"
					charsToEscape+= 5;
				}
				break;
			}
		}
	}

	int estimatedErrorMaxLength = (_errorOriginalMessage ? _errorOriginalMessage->length : 0) + (_errorFile ? (int)strlen(_errorFile) : 0) + charsToEscape + 207;

	if(estimatedErrorMaxLength  > _maxLen)
	{
	  //If resize fails return failure.
	  if( resize(estimatedErrorMaxLength) == 0 )
	    return 0;

	  reset();
	}
	_reset = false;

	writeBufVar(&JSON_TYPE, true);
	writeString("Error");

	writeBufVar(&JSON_TEXT, true);
	writeString(_errorText);

	writeBufVar(&JSON_DEBUG, true);
	writeOb();

	if(_errorFile)
	{
		writeBufVar(&JSON_FILE, false);
		writeSafeString(_errorFile);
	}
	if(_errorLine)
	{
		writeBufVar(&JSON_LINE, _errorFile);
		uInt32ToString(*_errorLine);
	}
	if(_errorOffset)
	{
		writeBufVar(&JSON_OFFSET, _errorFile || _errorLine);
		uInt32ToString(*_errorOffset);
	}
	if(_errorOriginalMessage)
	{
		writeBufVar(&JSON_MESSAGE, _errorFile || _errorLine || _errorOffset);
		writeJsonErrorMessage(_errorOriginalMessage);
	}

	writeOe();
	writeOe();

	return getJsonMsg(streamId, false);
}
