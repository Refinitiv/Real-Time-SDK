/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslMsg.h"
#include "rtr/rsslMsgDecoders.h"
#include "rtr/rsslMessageInt.h"
#include "rtr/rwfNetwork.h"
#include "rtr/decoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"

/* Can this container be decoded using the RSSL decoders? */
RTR_C_INLINE RsslBool _rsslCanDecodeContainerType(RsslUInt8 containerType)
{
	switch(containerType)
	{
	case RSSL_DT_FIELD_LIST:
	case RSSL_DT_FILTER_LIST:
	case RSSL_DT_ELEMENT_LIST:
	case RSSL_DT_MAP:
	case RSSL_DT_VECTOR:
	case RSSL_DT_SERIES:
	case RSSL_DT_MSG:
		return RSSL_TRUE;
	default:
		return RSSL_FALSE;
	}
}

RTR_C_INLINE char* _rsslDecodeQos(RsslQos * qos, char * position)
{
	RsslUInt8 qosValue;

	position += rwfGet8(qosValue, position);

	qos->timeliness = qosValue >> 5;
	qos->rate = (qosValue >> 1) & 0xF;
	qos->dynamic = qosValue & 0x1;

	if (qos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN)
		position += rwfGet16(qos->timeInfo, position);
	else
		qos->timeInfo = 0;

	if (qos->rate > RSSL_QOS_RATE_JIT_CONFLATED)
		position += rwfGet16(qos->rateInfo, position);
	else
		qos->rateInfo = 0;

	return position;
}

RTR_C_INLINE char* _rsslDecodeState(RsslState * state, char * position)
{
	RsslUInt8 stateVal = 0;
	
	position += rwfGet8(stateVal, position);
	
	/* only take lowest three bits */
	state->dataState = (stateVal & 0x7);

	/* now shift and take top five bits */
	state->streamState = (stateVal >> 3);
	
	position += rwfGet8(state->code, position);
	position = _rsslDecodeBuffer15(&state->text, position);
	return position;
}

RsslRet RTR_FASTCALL rsslDecodeBaseKey(RsslMsgKey * key, const char * data)
{
	char * position = (char *)data;

	position += rwfGetResBitU15(&key->flags, position);

	if (key->flags & RSSL_MKF_HAS_SERVICE_ID)
		position += rwfGetOptByteU16(&key->serviceId, position);
	if (key->flags & RSSL_MKF_HAS_NAME)
	{
		/* take name off wire */
		position += rwfGetBuffer8(&key->name, position);

		/* name type is only present if name is there */
		if (key->flags & RSSL_MKF_HAS_NAME_TYPE)
			position += rwfGet8(key->nameType, position);	
	}
	
	if (key->flags & RSSL_MKF_HAS_FILTER)
		position += rwfGet32(key->filter, position);

	if (key->flags & RSSL_MKF_HAS_IDENTIFIER)
		position += rwfGet32(key->identifier, position);

	if (key->flags & RSSL_MKF_HAS_ATTRIB)
	{
		position += rwfGet8(key->attribContainerType, position);
		/* container type needs to be scaled back up */
		key->attribContainerType += RSSL_DT_CONTAINER_TYPE_MIN;
		/* size is now an RB15 */
		if (key->attribContainerType != RSSL_DT_NO_DATA)
			position = _rsslDecodeBuffer15(&key->encAttrib, position);
		else
		{
			key->encAttrib.data = 0;
			key->encAttrib.length = 0;
		}
	}

	return (RsslRet)(position - data);
}

RSSL_API RsslRet rsslDecodeMsg(RsslDecodeIterator * dIter, RsslMsg * msg)
{
	RsslRet ret = 0;
	char * position;
	RsslDecodingLevel *_levelInfo;

	RSSL_ASSERT(dIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(msg, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(dIter->_pBuffer, Invalid parameters or parameters passed in as NULL);

	if (++dIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
 	_levelInfo = &dIter->_levelInfo[dIter->_decodingLevel];
	_levelInfo->_containerType = RSSL_DT_MSG;

	if (_levelInfo->_endBufPtr - dIter->_curBufPtr == 0)
	{
		return RSSL_RET_INCOMPLETE_DATA;
	}

	if ((ret = rsslDecodeMsgHeader(dIter, msg)) < 0)
		return(RSSL_RET_FAILURE);

	/* Maintain this use of the position variable for now, in case there is public use of this function. */
	dIter->_curBufPtr += ret;
	position = dIter->_curBufPtr;

	if(position > _levelInfo->_endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	/* the data section is not length specified */
	msg->msgBase.encDataBody.length = (rtrUInt32)(_levelInfo->_endBufPtr - position);

	if (msg->msgBase.encDataBody.length > 0)
	{
		msg->msgBase.encDataBody.data = (char*) position;
		
		if(!_rsslCanDecodeContainerType(msg->msgBase.containerType))
		{
			/* RSSL has no decoders for this format(e.g. Opaque). Move past it. */
			dIter->_curBufPtr += msg->msgBase.encDataBody.length;
			_endOfList(dIter);
			return RSSL_RET_SUCCESS;
		}

    	/* For now, _endBufPtr for msg and data levels is the same. */
    	dIter->_levelInfo[dIter->_decodingLevel+1]._endBufPtr = _levelInfo->_endBufPtr;
    	
    	return RSSL_RET_SUCCESS;
	}
	else
	{
		/* No payload. Reset iterator and return. */
		msg->msgBase.encDataBody.data = 0;
		_endOfList(dIter);
		return RSSL_RET_SUCCESS;
	}
}

RSSL_API RsslRet rsslDecodeDataSection(RsslMsg * msg, const RsslBuffer * buffer)
{
	msg->msgBase.encDataBody.length = buffer->length;
	msg->msgBase.encDataBody.data = buffer->data;

	return RSSL_RET_SUCCESS;
}


RSSL_API RsslRet rsslDecodeMsgHeader(const RsslDecodeIterator * dIter, RsslMsg * msg)
{
	char *position = dIter->_curBufPtr, *hdrStartPos;
	RsslUInt16 headerSize = 0;
	RsslUInt16 keySize = 0;

	/* header size */
	position += rwfGet16(headerSize, position);
	hdrStartPos = position;

	/* ensure there is enough data to decode the header */
	if ((position + headerSize) > dIter->_levelInfo->_endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;
	
	position += rwfGet8(msg->msgBase.msgClass, position);
	
	/* mask msgClass top three bits for reserved use later */
	/* Top three bits reserved for later use */
	msg->msgBase.msgClass &= 0x1F;

	position += rwfGet8(msg->msgBase.domainType, position);
	position += rwfGet32(msg->msgBase.streamId, position);

	msg->msgBase.encMsgBuffer.data = dIter->_curBufPtr;
	msg->msgBase.encMsgBuffer.length = (rtrUInt32)(dIter->_levelInfo[dIter->_decodingLevel]._endBufPtr - dIter->_curBufPtr);

    /* IMPORTANT: When new message classes are added, rsslCopyMsg and rsslValidateMsg have to modified as well */

	switch (msg->msgBase.msgClass)
	{
	case RSSL_MC_UPDATE:
		position += rwfGetResBitU15(&msg->updateMsg.flags, position);

		position += rwfGet8(msg->msgBase.containerType, position);
		/* need to scale containerType */
		msg->msgBase.containerType += RSSL_DT_CONTAINER_TYPE_MIN;

		position += rwfGet8(msg->updateMsg.updateType, position);
		

		if (msg->updateMsg.flags & RSSL_UPMF_HAS_SEQ_NUM)
			position += rwfGet32(msg->updateMsg.seqNum , position);
		else
			msg->updateMsg.seqNum = 0;

		if (msg->updateMsg.flags & RSSL_UPMF_HAS_CONF_INFO)
		{
			position += rwfGetResBitU15(&msg->updateMsg.conflationCount , position);
			position += rwfGet16(msg->updateMsg.conflationTime , position);
		}

		if (msg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA)
		{
			position = _rsslDecodeBuffer15(&msg->updateMsg.permData, position);
		}
		else
		{
			rsslClearBuffer(&msg->updateMsg.permData);
		}

		if (msg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY)
		{
			position += rwfGetResBitU15(&keySize, position);
			/* dont iterate position by this value anymore.  We want to add the keySize to position */
			rsslDecodeBaseKey(&msg->msgBase.msgKey, position);
			/* add keySize to position */
			position += keySize;
		}
		else
		{
			rsslClearMsgKey(&msg->msgBase.msgKey);
		}
	
		if (msg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER)
			position += rwfGetBuffer8(&msg->updateMsg.extendedHeader, position);
		else
			rsslClearBuffer(&msg->updateMsg.extendedHeader);

		if (msg->updateMsg.flags & RSSL_UPMF_HAS_POST_USER_INFO)
		{
			/* decode only if actually in the header */
			if ((headerSize - (position - hdrStartPos)) >= (int)sizeof(msg->updateMsg.postUserInfo))
			{
				/* get user address */
				position += rwfGet32(msg->updateMsg.postUserInfo.postUserAddr , position);
				/* get user ID */
				position += rwfGet32(msg->updateMsg.postUserInfo.postUserId , position);
			}
			else /* not really there, unset flag */
			{
				msg->updateMsg.flags &= ~RSSL_UPMF_HAS_POST_USER_INFO;
			}
		}
		else
		{
			msg->updateMsg.postUserInfo.postUserAddr = 0;
			msg->updateMsg.postUserInfo.postUserId = 0;
		}
		break;

	case RSSL_MC_GENERIC:
		position += rwfGetResBitU15(&msg->genericMsg.flags, position);

		position += rwfGet8(msg->msgBase.containerType, position);
		/* need to scale containerType */
		msg->msgBase.containerType += RSSL_DT_CONTAINER_TYPE_MIN;

		if (msg->genericMsg.flags & RSSL_GNMF_HAS_SEQ_NUM)
			position += rwfGet32(msg->genericMsg.seqNum , position);
		else
			msg->genericMsg.seqNum = 0;

		if (msg->genericMsg.flags & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM)
			position += rwfGet32(msg->genericMsg.secondarySeqNum, position);
		else
			msg->genericMsg.secondarySeqNum = 0;

		if (msg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA)
		{
			position = _rsslDecodeBuffer15(&msg->genericMsg.permData, position);
		}
		else
		{
			rsslClearBuffer(&msg->genericMsg.permData);
		}

		if (msg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY)
		{
			position += rwfGetResBitU15(&keySize, position);
			/* dont iterate position by this value anymore.  We want to add the keySize to position */
			rsslDecodeBaseKey(&msg->msgBase.msgKey, position);
			/* add keySize to position */
			position += keySize;
		}
		else
		{
			rsslClearMsgKey(&msg->msgBase.msgKey);
		}
	
		if (msg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER)
			position += rwfGetBuffer8(&msg->genericMsg.extendedHeader, position);
		else
			rsslClearBuffer(&msg->genericMsg.extendedHeader);

		if (msg->genericMsg.flags & RSSL_GNMF_HAS_PART_NUM)
		{
			/* decode only if actually in the header */
			if ((headerSize - (position - hdrStartPos)) > 0)
			{
				position += rwfGetResBitU15(&msg->genericMsg.partNum, position);
			}
			else /* not really there, unset flag */
			{
				msg->genericMsg.flags &= ~RSSL_GNMF_HAS_PART_NUM;
				msg->genericMsg.partNum = 0;
			}
		}
		else
		{
			msg->genericMsg.partNum = 0;
		}

		if (msg->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY)
		{
			position += rwfGetResBitU15(&keySize, position);
			/* dont iterate position by this value anymore.  We want to add the keySize to position */
			rsslDecodeBaseKey(&msg->genericMsg.reqMsgKey, position);
			/* add keySize to position */
			position += keySize;
		}
		else
		{
			rsslClearMsgKey(&msg->genericMsg.reqMsgKey);
		}
		break;

	case RSSL_MC_REFRESH:
		position += rwfGetResBitU15(&msg->refreshMsg.flags, position);
		position += rwfGet8(msg->msgBase.containerType, position);
		/* need to scale containerType */
		msg->msgBase.containerType += RSSL_DT_CONTAINER_TYPE_MIN;

		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM)
			position += rwfGet32(msg->refreshMsg.seqNum , position);
		else
			msg->refreshMsg.seqNum = 0;
	
		position = _rsslDecodeState(&msg->refreshMsg.state, position);

		position += rwfGetBuffer8(&msg->refreshMsg.groupId, position);
			
		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA)
		{
			position = _rsslDecodeBuffer15(&msg->refreshMsg.permData, position);
		}
		else
		{
			rsslClearBuffer(&msg->refreshMsg.permData);
		}	

		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_QOS)
		{
	        position = _rsslDecodeQos( &msg->refreshMsg.qos, position );
		}
		else
		{
			msg->refreshMsg.qos.dynamic = 0;
			msg->refreshMsg.qos.rate = 0;
			msg->refreshMsg.qos.rateInfo = 0;
			msg->refreshMsg.qos.timeInfo = 0;
			msg->refreshMsg.qos.timeliness = 0;
		}

		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY)
		{
			position += rwfGetResBitU15(&keySize, position);
			/* dont iterate position by this value anymore.  We want to add the keySize to position */
    		rsslDecodeBaseKey(&msg->msgBase.msgKey, position);
			/* add keySize to position */
			position += keySize;
		}
		else
		{
			rsslClearMsgKey(&msg->msgBase.msgKey);
		}

		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER)
			position += rwfGetBuffer8(&msg->refreshMsg.extendedHeader, position);
		else
			rsslClearBuffer(&msg->refreshMsg.extendedHeader);

		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_POST_USER_INFO)
		{
			/* decode only if actually in the header */
			if ((headerSize - (position - hdrStartPos)) >= (int) sizeof(msg->refreshMsg.postUserInfo))
			{
				/* get user address */
				position += rwfGet32(msg->refreshMsg.postUserInfo.postUserAddr, position);
				/* get user ID */
				position += rwfGet32(msg->refreshMsg.postUserInfo.postUserId, position);
			}
			else /* not really there, unset flag */
			{
				msg->refreshMsg.flags &= ~RSSL_RFMF_HAS_POST_USER_INFO;
			}
		}
		else
		{
			msg->refreshMsg.postUserInfo.postUserAddr = 0;
			msg->refreshMsg.postUserInfo.postUserId = 0;
		}

		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_PART_NUM)
		{
			/* decode only if actually in the header */
			if ((headerSize - (position - hdrStartPos)) > 0)
			{
				position += rwfGetResBitU15(&msg->refreshMsg.partNum, position);
			}
			else /* not really there, unset flag */
			{
				msg->refreshMsg.partNum = 0;
				msg->refreshMsg.flags &= ~RSSL_RFMF_HAS_PART_NUM;
			}
		}
		else
		{
			msg->refreshMsg.partNum = 0;
		}

		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY)
		{
			position += rwfGetResBitU15(&keySize, position);
			/* dont iterate position by this value anymore.  We want to add the keySize to position */
			rsslDecodeBaseKey(&msg->refreshMsg.reqMsgKey, position);
			/* add keySize to position */
			position += keySize;
		}
		else
		{
			rsslClearMsgKey(&msg->refreshMsg.reqMsgKey);
		}
		break;

	case RSSL_MC_POST:
		position += rwfGetResBitU15(&msg->postMsg.flags, position);

		position += rwfGet8(msg->msgBase.containerType, position);
		/* need to scale containerType */
		msg->msgBase.containerType += RSSL_DT_CONTAINER_TYPE_MIN;
		
		/* get user address */
		position += rwfGet32(msg->postMsg.postUserInfo.postUserAddr , position);
		/* get user ID */
		position += rwfGet32(msg->postMsg.postUserInfo.postUserId , position);

		if (msg->postMsg.flags & RSSL_PSMF_HAS_SEQ_NUM)
			position += rwfGet32(msg->postMsg.seqNum , position);
		else
			msg->postMsg.seqNum = 0;

		if (msg->postMsg.flags & RSSL_PSMF_HAS_POST_ID)
			position += rwfGet32(msg->postMsg.postId, position);
		else
			msg->postMsg.postId = 0;

		if(msg->postMsg.flags & RSSL_PSMF_HAS_PERM_DATA)
			position = _rsslDecodeBuffer15(&msg->postMsg.permData, position);
		else 
			rsslClearBuffer(&msg->postMsg.permData);	
			
		if (msg->postMsg.flags & RSSL_PSMF_HAS_MSG_KEY)
		{
			position += rwfGetResBitU15(&keySize, position);
			/* dont iterate position by this value anymore.  We want to add the keySize to position */
			rsslDecodeBaseKey(&msg->msgBase.msgKey, position);
			/* add keySize to position */
			position += keySize;
		}
		else
		{
			rsslClearMsgKey(&msg->msgBase.msgKey);
		}
	
		if (msg->postMsg.flags & RSSL_PSMF_HAS_EXTENDED_HEADER)
			position += rwfGetBuffer8(&msg->postMsg.extendedHeader, position);
		else
			rsslClearBuffer(&msg->postMsg.extendedHeader);

		if (msg->postMsg.flags & RSSL_PSMF_HAS_PART_NUM)
		{
			/* decode only if actually in the header */
			if ((headerSize - (position - hdrStartPos)) > 0)
			{
				position += rwfGetResBitU15(&msg->postMsg.partNum, position);
			}
			else /* not really there, unset flag and set partNum to 0*/
			{
				msg->postMsg.partNum = 0;
				msg->postMsg.flags &= ~RSSL_PSMF_HAS_PART_NUM;
			}
		}
		else
		{
			msg->postMsg.partNum = 0;
		}
		
		if (msg->postMsg.flags & RSSL_PSMF_HAS_POST_USER_RIGHTS)
		{
			/* decode only if actually in the header */
			if ((headerSize - (position - hdrStartPos)) > 0)
			{
				position += rwfGetResBitU15(&msg->postMsg.postUserRights, position);
			}
			else
			{ /* not really there, unset flag and set postUserRights to 0*/
				msg->postMsg.postUserRights = 0;
				msg->postMsg.flags &= ~RSSL_PSMF_HAS_POST_USER_RIGHTS;
			}
		}
		else
			msg->postMsg.postUserRights = 0;
		break;

	case RSSL_MC_REQUEST:
		position += rwfGetResBitU15(&msg->requestMsg.flags, position);
		position += rwfGet8(msg->msgBase.containerType, position);
		/* need to scale containerType */
		msg->msgBase.containerType += RSSL_DT_CONTAINER_TYPE_MIN;

		if (msg->requestMsg.flags & RSSL_RQMF_HAS_PRIORITY)
		{
			position += rwfGet8(msg->requestMsg.priorityClass, position);
			position += rwfGetOptByteU16(&msg->requestMsg.priorityCount, position);
		}
		else
		{
			msg->requestMsg.priorityClass = 1;
			msg->requestMsg.priorityCount = 1;
		}

		if (msg->requestMsg.flags & RSSL_RQMF_HAS_QOS)
		{
	        position = _rsslDecodeQos( &msg->requestMsg.qos, position );
		}
		else
		{
			msg->requestMsg.qos.dynamic = 0;
			msg->requestMsg.qos.rate = 0;
			msg->requestMsg.qos.rateInfo = 0;
			msg->requestMsg.qos.timeInfo = 0;
			msg->requestMsg.qos.timeliness = 0;
		}

		if (msg->requestMsg.flags & RSSL_RQMF_HAS_WORST_QOS)
		{
	        position = _rsslDecodeQos( &msg->requestMsg.worstQos, position);
		}
		else
		{
			msg->requestMsg.worstQos.dynamic = 0;
			msg->requestMsg.worstQos.rate = 0;
			msg->requestMsg.worstQos.rateInfo = 0;
			msg->requestMsg.worstQos.timeInfo = 0;
			msg->requestMsg.worstQos.timeliness = 0;
		}

		position += rwfGetResBitU15(&keySize, position);
		/* dont iterate position by this value anymore.  We want to add the keySize to position */
		rsslDecodeBaseKey(&msg->msgBase.msgKey, position);
		/* add keySize to position */
		position += keySize;

		if (msg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER)
			position += rwfGetBuffer8(&msg->requestMsg.extendedHeader, position);
		else
			rsslClearBuffer(&msg->requestMsg.extendedHeader);
		break;

	case RSSL_MC_STATUS:
		position += rwfGetResBitU15(&msg->statusMsg.flags, position);
		position += rwfGet8(msg->msgBase.containerType, position);
		/* need to scale containerType */
		msg->msgBase.containerType += RSSL_DT_CONTAINER_TYPE_MIN;

		if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
		{
			position = _rsslDecodeState(&msg->statusMsg.state, position);
		}
		
		if (msg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID)
		{
			position += rwfGetBuffer8(&msg->statusMsg.groupId, position);
		}
		else
		{
			rsslClearBuffer(&msg->statusMsg.groupId);
		}

		if (msg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA)
		{
			position = _rsslDecodeBuffer15(&msg->statusMsg.permData, position);
		}
		else
		{
			rsslClearBuffer(&msg->statusMsg.permData);
		}

		if (msg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY)
		{
			position += rwfGetResBitU15(&keySize, position);
    		rsslDecodeBaseKey(&msg->msgBase.msgKey, position);
			position += keySize;
		}
		else
		{
			rsslClearMsgKey(&msg->msgBase.msgKey);
		}

		if (msg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER)
			position += rwfGetBuffer8(&msg->statusMsg.extendedHeader, position);
		else
			rsslClearBuffer(&msg->statusMsg.extendedHeader);

		if (msg->statusMsg.flags & RSSL_STMF_HAS_POST_USER_INFO)
		{
			/* decode only if actually in the header */
			if ((headerSize - (position - hdrStartPos)) >= (int)sizeof(msg->statusMsg.postUserInfo))
			{
				/* get user address */
				position += rwfGet32(msg->statusMsg.postUserInfo.postUserAddr, position);
				/* get user ID */
				position += rwfGet32(msg->statusMsg.postUserInfo.postUserId, position);
			}
			else /* not really there, unset flag */
			{
				msg->statusMsg.flags &= ~RSSL_STMF_HAS_POST_USER_INFO;
			}
		}
		else
		{
			msg->statusMsg.postUserInfo.postUserAddr = 0;
			msg->statusMsg.postUserInfo.postUserId = 0;
		}
		
		if (msg->statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY)
		{
			position += rwfGetResBitU15(&keySize, position);
			/* dont iterate position by this value anymore.  We want to add the keySize to position */
			rsslDecodeBaseKey(&msg->statusMsg.reqMsgKey, position);
			/* add keySize to position */
			position += keySize;
		}
		else
		{
			rsslClearMsgKey(&msg->statusMsg.reqMsgKey);
		}
		break;

	case RSSL_MC_CLOSE:
		position += rwfGetResBitU15(&msg->closeMsg.flags, position);
		position += rwfGet8(msg->msgBase.containerType, position);
		/* need to scale containerType */
		msg->msgBase.containerType += RSSL_DT_CONTAINER_TYPE_MIN;
	

		if (msg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER)
			position += rwfGetBuffer8(&msg->closeMsg.extendedHeader, position);
		else
			rsslClearBuffer(&msg->closeMsg.extendedHeader);
		break;

	case RSSL_MC_ACK:
		position += rwfGetResBitU15(&msg->ackMsg.flags, position);
		position += rwfGet8(msg->msgBase.containerType, position);
		/* need to scale containerType */
		msg->msgBase.containerType += RSSL_DT_CONTAINER_TYPE_MIN;

		position += rwfGet32(msg->ackMsg.ackId, position);

		if (msg->ackMsg.flags & RSSL_AKMF_HAS_NAK_CODE)
		{
			position += rwfGet8(msg->ackMsg.nakCode, position);
		}
		else
			msg->ackMsg.nakCode = 0;

		if (msg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
		{
			/*position += rsslDecodeSmallBuffer(&msg->ackMsg.text, position); */
			position += rwfGetBuffer16(&msg->ackMsg.text, position);
		}
		else
		{
			rsslClearBuffer(&msg->ackMsg.text);
		}

		if (msg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
            position += rwfGet32(msg->ackMsg.seqNum, position);
        else
            msg->ackMsg.seqNum = 0;

        if (msg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY)
        {
            position += rwfGetResBitU15(&keySize, position);
            rsslDecodeBaseKey(&msg->msgBase.msgKey, position);
            position += keySize;
        }
        else
        {
            rsslClearMsgKey(&msg->msgBase.msgKey);
        }

		if (msg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER)
			position += rwfGetBuffer8(&msg->ackMsg.extendedHeader, position);
		else
			rsslClearBuffer(&msg->ackMsg.extendedHeader);
		break;

	default:
		return RSSL_RET_FAILURE;
	}

	/* move to end of header */
	position = dIter->_curBufPtr + headerSize + 2;

	return (RsslRet)(position - dIter->_curBufPtr);
}


/* TO DO Figure out duplicate definitions in messagesDecoders.c */
#define _RSSL_MSG_CLASS_POS                 2
#define _RSSL_MSG_TYPE_POS                  3
#define _RSSL_MSG_STREAMID_POS		        4


RSSL_API RsslUInt8 rsslExtractMsgClass( const RsslDecodeIterator *dIter )
{
    RsslUInt8 msgClass;
	const RsslBuffer *pEncodedMessageBuffer = dIter->_pBuffer;
	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_CHECKSIZE(_RSSL_MSG_CLASS_POS + sizeof(msgClass), pEncodedMessageBuffer->length);

	/* Get msgClass as UInt8 */
	rwfGet8( msgClass, (pEncodedMessageBuffer->data + _RSSL_MSG_CLASS_POS) );

    return msgClass;
}

RSSL_API RsslUInt8 rsslExtractDomainType( const RsslDecodeIterator *dIter )
{
    RsslUInt8 domainType;
	const RsslBuffer *pEncodedMessageBuffer = dIter->_pBuffer;

	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_CHECKSIZE(_RSSL_MSG_TYPE_POS + sizeof(domainType), pEncodedMessageBuffer->length);

    /* Get domainType as UInt8 */
	rwfGet8( domainType, (pEncodedMessageBuffer->data + _RSSL_MSG_TYPE_POS)  );

    return domainType;
}

RSSL_API RsslInt32 rsslExtractStreamId( const RsslDecodeIterator *dIter )
{
    RsslInt32  streamId;
	const RsslBuffer *pEncodedMessageBuffer = dIter->_pBuffer;

	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_CHECKSIZE(_RSSL_MSG_STREAMID_POS + sizeof(streamId), pEncodedMessageBuffer->length);

	/* Get streamId as Int32 */
	rwfGet32( streamId, (pEncodedMessageBuffer->data + _RSSL_MSG_STREAMID_POS)  );

    return streamId;
}

RSSL_API RsslRet rsslExtractGroupId( 
				const RsslDecodeIterator *dIter,
				RsslBuffer *			groupId )  
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	RsslUInt16 stateTextLen = 0;
	const RsslBuffer *pEncodedMessageBuffer = dIter->_pBuffer;

	char * position;

	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pEncodedMessageBuffer->data, Invalid parameters or parameters passed in as NULL);

	if (pEncodedMessageBuffer->length < 10)
		return RSSL_RET_INVALID_ARGUMENT;

	position = pEncodedMessageBuffer->data;
	position += _RSSL_MSG_CLASS_POS;  /* for header size */
	position += rwfGet8(msgClass, position);

	if ((msgClass != RSSL_MC_REFRESH) && ( msgClass != RSSL_MC_STATUS))
		return RSSL_RET_FAILURE;

	position++; // domainType
	position += 4; // streamId
	position += rwfGetResBitU15(&mFlags, position);

	if ((msgClass == RSSL_MC_STATUS) && (!(mFlags & RSSL_STMF_HAS_GROUP_ID)))
		return RSSL_RET_FAILURE;

	position++; // containerType
			
	if ((msgClass == RSSL_MC_REFRESH) && (mFlags & RSSL_RFMF_HAS_SEQ_NUM))
		position += 4;  // seqNo

	if (((msgClass == RSSL_MC_STATUS) && (mFlags & RSSL_STMF_HAS_STATE)) || (msgClass == RSSL_MC_REFRESH))
	{
		/* move past state info */
		position += 2;
		position += rwfGetResBitU15(&stateTextLen, position);
		position += stateTextLen;
	}
	
	rwfGetBuffer8(groupId, position);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslExtractSeqNum( 
				const RsslDecodeIterator *dIter,
				RsslUInt32 *			seqNum )
                     
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	RsslBuffer tempBuf;
	const RsslBuffer *pEncodedMessageBuffer = dIter->_pBuffer;

	char * position;

	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pEncodedMessageBuffer->data, Invalid parameters or parameters passed in as NULL);

	if (pEncodedMessageBuffer->length < 10)
		return RSSL_RET_INVALID_ARGUMENT;

	position = pEncodedMessageBuffer->data;
	position += _RSSL_MSG_CLASS_POS;  /* for header size */
	position += rwfGet8(msgClass, position);

	switch (msgClass)
	{
		case RSSL_MC_UPDATE:
		{
			position += 5; //domainType and streamId
			position += rwfGetResBitU15(&mFlags, position);

			if (!(mFlags & RSSL_UPMF_HAS_SEQ_NUM))
				return RSSL_RET_FAILURE;

			position += 2;
		}
		break;
		case RSSL_MC_REFRESH:
		{
			position += 5; //domainType and streamId
			position += rwfGetResBitU15(&mFlags, position);

			if (!(mFlags & RSSL_RFMF_HAS_SEQ_NUM))
				return RSSL_RET_FAILURE;

			++position; //containerType
		}
		break;
		case RSSL_MC_GENERIC:
		{
			position += 5; // domainType and streamId 
			position += rwfGetResBitU15(&mFlags, position);

			if (!(mFlags & RSSL_GNMF_HAS_SEQ_NUM))
				return RSSL_RET_FAILURE;
			
			++position; // containerType
		}
		break;
		case RSSL_MC_POST:
		{
			position += 5; //domainType and streamId
			position += rwfGetResBitU15(&mFlags, position);

			if (!(mFlags & RSSL_PSMF_HAS_SEQ_NUM))
				return RSSL_RET_FAILURE;

			position += 9; //postUserInfo and containerType
		}
		break;
		case RSSL_MC_ACK:
		{
			position += 5; //domainType and streamId
			position += rwfGetResBitU15(&mFlags, position);

			if (!(mFlags & RSSL_AKMF_HAS_SEQ_NUM))
				return RSSL_RET_FAILURE;
			
			position += 5; // ackId and containerType

			if (mFlags & RSSL_AKMF_HAS_NAK_CODE)
				++position; // nakCode

			if (mFlags & RSSL_AKMF_HAS_TEXT)
				position += rwfGetBuffer16(&tempBuf, position);
		}
		break;
		default:
			return RSSL_RET_FAILURE;
	}

	rwfGet32(*seqNum, position);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslExtractPostId( 
				const RsslDecodeIterator *dIter,
				RsslUInt32 *			postId )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	const RsslBuffer *pEncodedMessageBuffer = dIter->_pBuffer;

	char * position;

	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pEncodedMessageBuffer->data, Invalid parameters or parameters passed in as NULL);

	if (pEncodedMessageBuffer->length < 10)
		return RSSL_RET_INVALID_ARGUMENT;

	position = pEncodedMessageBuffer->data;
	position += _RSSL_MSG_CLASS_POS;  /* for header size */
	position += rwfGet8(msgClass, position);

	switch (msgClass)
	{
		case RSSL_MC_POST:
		{
			position += 5; //domainType and streamId
			position += rwfGetResBitU15(&mFlags, position);

			if (!(mFlags & RSSL_PSMF_HAS_POST_ID))
				return RSSL_RET_FAILURE;

			position += 9; //postUserInfo and containerType

			if (mFlags & RSSL_PSMF_HAS_SEQ_NUM)
				position += 4;
		}
		break;
		default:
			return RSSL_RET_FAILURE;
	}

	rwfGet32(*postId, position);

	return RSSL_RET_SUCCESS;
}

RSSL_API const RsslUInt32* rsslGetSeqNum( const RsslMsg * pMsg )
{
	switch (pMsg->msgBase.msgClass)
	{
		case RSSL_MC_UPDATE:
			if (pMsg->updateMsg.flags & RSSL_UPMF_HAS_SEQ_NUM)
				return &pMsg->updateMsg.seqNum;
		break;
		case RSSL_MC_REFRESH:
			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_SEQ_NUM)
				return &pMsg->refreshMsg.seqNum;
		break;
		case RSSL_MC_GENERIC:
			if (pMsg->genericMsg.flags & RSSL_GNMF_HAS_SEQ_NUM)
				return &pMsg->genericMsg.seqNum;
		break;
		case RSSL_MC_POST:
			if (pMsg->postMsg.flags & RSSL_PSMF_HAS_SEQ_NUM)
				return &pMsg->postMsg.seqNum;
		break;
		case RSSL_MC_ACK:
			if (pMsg->ackMsg.flags & RSSL_AKMF_HAS_SEQ_NUM)
				return &pMsg->ackMsg.seqNum;
        break;
		/* these messages do not have seqNum so they will fall through to return 0 */
		//case RSSL_MC_REQUEST:
		//case RSSL_MC_STATUS:
		//case RSSL_MC_CLOSE:
	}
	return 0;
}

RSSL_API const RsslMsgKey* rsslGetMsgKey( const RsslMsg * pMsg )
{
	switch (pMsg->msgBase.msgClass)
	{
		/* update first for performance */
		case RSSL_MC_UPDATE:
			if (pMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY)
				return &pMsg->msgBase.msgKey;
		break;
		case RSSL_MC_REFRESH:
			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY)
				return &pMsg->msgBase.msgKey;
		break;
		case RSSL_MC_REQUEST:
			return &pMsg->msgBase.msgKey;
		break;
		case RSSL_MC_STATUS:
			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY)
				return &pMsg->msgBase.msgKey;
		break;
		case RSSL_MC_GENERIC:
			if (pMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY)
				return &pMsg->msgBase.msgKey;
		break;
		case RSSL_MC_POST:
			if (pMsg->postMsg.flags & RSSL_PSMF_HAS_MSG_KEY)
				return &pMsg->msgBase.msgKey;
		break;
		case RSSL_MC_ACK:
			if (pMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY)
				return &pMsg->msgBase.msgKey;
		break;
	}
	/* anything that hasnt returned by now (no key or close msg) should just return 0 */
	return 0;
}

RSSL_API const RsslMsgKey* rsslGetReqMsgKey( const RsslMsg * pMsg )
{
	switch (pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY)
				return &pMsg->refreshMsg.reqMsgKey;
		break;
		case RSSL_MC_STATUS:
			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY)
				return &pMsg->statusMsg.reqMsgKey;
		break;
		case RSSL_MC_GENERIC:
			if (pMsg->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY)
				return &pMsg->genericMsg.reqMsgKey;
		break;
	}
	/* anything that hasnt returned by now doesnt have a request key and should just return 0 */
	return 0;
}

RSSL_API const RsslUInt16* rsslGetFlags( const RsslMsg * pMsg )
{
	switch (pMsg->msgBase.msgClass)
	{
        case RSSL_MC_UPDATE:
			return &pMsg->updateMsg.flags;
		case RSSL_MC_GENERIC:
			return &pMsg->genericMsg.flags;
		case RSSL_MC_REFRESH:
			return &pMsg->refreshMsg.flags;
		case RSSL_MC_POST:
			return &pMsg->postMsg.flags;
		case RSSL_MC_REQUEST:
			return &pMsg->requestMsg.flags;
		case RSSL_MC_STATUS:
			return &pMsg->statusMsg.flags;
		case RSSL_MC_CLOSE:
			return &pMsg->closeMsg.flags;
		case RSSL_MC_ACK:
			return &pMsg->ackMsg.flags;
		default:
			return 0;
	}
}

RSSL_API const RsslBuffer* rsslGetExtendedHeader( const RsslMsg * pMsg )
{
	switch (pMsg->msgBase.msgClass)
	{
        case RSSL_MC_UPDATE:
			if (pMsg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER)
				return &pMsg->updateMsg.extendedHeader;
		break;
		case RSSL_MC_GENERIC:
			if (pMsg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER)
				return &pMsg->genericMsg.extendedHeader;
		break;
		case RSSL_MC_REFRESH:
			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER)
				return &pMsg->refreshMsg.extendedHeader;
		break;
		case RSSL_MC_POST:
			if (pMsg->postMsg.flags & RSSL_PSMF_HAS_EXTENDED_HEADER)
				return &pMsg->postMsg.extendedHeader;
		break;
		case RSSL_MC_REQUEST:
			if (pMsg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER)
				return &pMsg->requestMsg.extendedHeader;
		break;
		case RSSL_MC_STATUS:
			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER)
				return &pMsg->statusMsg.extendedHeader;
		break;
		case RSSL_MC_CLOSE:
			if (pMsg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER)
				return &pMsg->statusMsg.extendedHeader;
		break;
		case RSSL_MC_ACK:
			if (pMsg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER)
				return &pMsg->ackMsg.extendedHeader;
		break;
	}
	/* anything that did not have extended header will return 0 */
	return 0;
}

RSSL_API const RsslState* rsslGetState( const RsslMsg * pMsg )
{
	switch (pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			return &pMsg->refreshMsg.state;
		break;
		case RSSL_MC_STATUS:
			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
				return &pMsg->statusMsg.state;
		break;
		/* these messages do not have state, so they will fall through and return 0 */
		/*
			case RSSL_MC_UPDATE: 
			case RSSL_MC_POST:
			case RSSL_MC_REQUEST:
			case RSSL_MC_CLOSE:
			case RSSL_MC_ACK:
			case RSSL_MC_GENERIC:
		*/
	}
	return 0;
}

RSSL_API const RsslBuffer* rsslGetPermData( const RsslMsg * pMsg )
{
	switch (pMsg->msgBase.msgClass)
	{
        case RSSL_MC_UPDATE:
			if (pMsg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA)
				return &pMsg->updateMsg.permData;
		break;
		case RSSL_MC_GENERIC:
			if (pMsg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA)
				return &pMsg->genericMsg.permData;
		break;
		case RSSL_MC_REFRESH:
			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA)
				return &pMsg->refreshMsg.permData;
		break;
		case RSSL_MC_STATUS:
			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA)
				return &pMsg->statusMsg.permData;
		break;
		case RSSL_MC_POST: 
			if (pMsg->postMsg.flags & RSSL_PSMF_HAS_PERM_DATA)
				return &pMsg->postMsg.permData;
		break;
		/* these messages do not have perm data, so they will fall through and return 0 */
		/*
			case RSSL_MC_REQUEST:
			case RSSL_MC_CLOSE:
			case RSSL_MC_ACK:
		*/
	}
	return 0;
}

RSSL_API const RsslBuffer* rsslGetGroupId( const RsslMsg * pMsg )
{
	switch (pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			return &pMsg->refreshMsg.groupId;
		break;
		case RSSL_MC_STATUS:
			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID)
				return &pMsg->statusMsg.groupId;
		break;
		/* these messages dont have perm data so they will fall through and return 0 */
		/*
			case RSSL_MC_UPDATE:
			case RSSL_MC_POST:
			case RSSL_MC_REQUEST:
			case RSSL_MC_CLOSE:
			case RSSL_MC_ACK:
			case RSSL_MC_GENERIC:
		*/
	}
	return 0;
}

RsslRet rsslExtractHeader(RsslBuffer * header, const RsslMsg * msg)
{
	char * end;

	RSSL_ASSERT(header, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(msg, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(msg->msgBase.encMsgBuffer.data, Invalid parameters or parameters passed in as NULL);

	if ((msg->msgBase.encDataBody.data != 0) && (msg->msgBase.encDataBody.length > 0))	
		end = msg->msgBase.encDataBody.data;
	else
		end = msg->msgBase.encMsgBuffer.data + msg->msgBase.encMsgBuffer.length;

	RSSL_ASSERT(end, Empty contents);

	header->data = msg->msgBase.encMsgBuffer.data;
	header->length = (rtrUInt32)(end - header->data);
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeMsgKeyAttrib(RsslDecodeIterator *iIter, const RsslMsgKey *pKey)
{
	/* Create a level to save the current iterator position.
	 * Once the decode of the opaque is finished, the iterator will be returned to this position. */
    RsslDecodingLevel *_levelInfo;
    
    RSSL_ASSERT(iIter, Invalid parameters or parameters passed in as NULL);
    RSSL_ASSERT(pKey, Invalid parameters or parameters passed in as NULL);

	if ( !(pKey->flags & RSSL_MKF_HAS_ATTRIB) )
		return RSSL_RET_INVALID_ARGUMENT;
	if(!_rsslCanDecodeContainerType(pKey->attribContainerType))
		return RSSL_RET_INVALID_ARGUMENT;

	++iIter->_decodingLevel;
	if (iIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS - 1)
		return RSSL_RET_ITERATOR_OVERRUN;
    
    _levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];

	/* Set type to NO_DATA. When an entry decoder finishes and sees this,
	 * they will know that this was only a 'temporary' container and reset curBufPtr
	 * (actual rwf container decoders set this to their appropriate type). */
    _levelInfo->_containerType = RSSL_DT_NO_DATA;

	/* Save iterator position */
    _levelInfo->_nextEntryPtr = iIter->_curBufPtr; 

	/* Setup iterator to decode opaque. */
	iIter->_curBufPtr = pKey->encAttrib.data;
	iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = pKey->encAttrib.data + pKey->encAttrib.length;

	/* Leave the current _levelInfo's _endBufPtr where it is. It's not used in the opaque decoding,
	 * and it needs to be pointing to the correct position after the reset. */
	
	return RSSL_RET_SUCCESS;
}

RsslRet rsslExtractDataSection(RsslBuffer * data, const RsslMsg * msg)
{
	char * start;

	RSSL_ASSERT(data, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(msg, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(msg->msgBase.encMsgBuffer.data, Invalid parameters or parameters passed in as NULL);

	if ((msg->msgBase.encDataBody.data != 0) && (msg->msgBase.encDataBody.length > 0))
		start = msg->msgBase.encDataBody.data;
	else
		start = 0;
	data->data = start;
	if (start)
		data->length = (rtrUInt32)(msg->msgBase.encDataBody.data + msg->msgBase.encDataBody.length - start);
	else
		data->length = 0;
	return RSSL_RET_SUCCESS;
}
