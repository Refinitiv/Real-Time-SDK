/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslMessageInt.h"
#include "rtr/encoderTools.h"
#include "rtr/rsslMsg.h"
#include "rtr/rsslMsgEncoders.h"
#include "rtr/rwfNetwork.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslIteratorUtilsInt.h"


RSSL_API RsslRet rsslAddGroupId(RsslBuffer *pGroupId,
								RsslUInt16  groupIdToAdd)
{
	RSSL_ASSERT(pGroupId, Invalid parameters or parameters passed in as NULL);

	pGroupId->length += rwfPut16((pGroupId->data + pGroupId->length), groupIdToAdd);
	
	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet _rsslEncodeQosInt(RsslEncodeIterator *pIter,
									const RsslQos *pQos )
{					
	RsslUInt8 qos;
	RsslUInt8 dataLength = 1;

	dataLength += (pQos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN) ? 2: 0;
	dataLength += (pQos->rate > RSSL_QOS_RATE_JIT_CONFLATED) ? 2 : 0;

	if (_rsslIteratorOverrun(pIter, dataLength))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	if (pQos->timeliness == RSSL_QOS_TIME_UNSPECIFIED ||
		pQos->rate == RSSL_QOS_RATE_UNSPECIFIED)
		return RSSL_RET_INVALID_DATA;

	qos = (pQos->timeliness << 5);
	qos |= (pQos->rate << 1);
	qos |= pQos->dynamic;

	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, qos);
	if (pQos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN)
		pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, pQos->timeInfo);

	if (pQos->rate > RSSL_QOS_RATE_JIT_CONFLATED)
		pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, pQos->rateInfo);

	return RSSL_RET_SUCCESS;			
}

RTR_C_INLINE RsslRet _rsslEncodeStateInt(RsslEncodeIterator *pIter, 
										const RsslState *pState )
{
	RsslUInt8	state;
	RsslUInt16  dataLength = 3 + pState->text.length;

	if (pState->text.length > 0x80)
		dataLength += 1;

	if (_rsslIteratorOverrun(pIter, dataLength))
		return(RSSL_RET_BUFFER_TOO_SMALL);
	if (pState->text.length > RWF_MAX_U15)
		return RSSL_RET_INVALID_DATA;

	if (pState->streamState == RSSL_STREAM_UNSPECIFIED)
		return RSSL_RET_INVALID_DATA;

	state = (pState->streamState << 3)	;
	state |= pState->dataState;

	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, state);
	pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pState->code);
	pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pState->text);

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet _rsslEncodeMsgPartNumPost(RsslEncodeIterator *pIter, const RsslMsg *pMsg)
{
	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_UPDATE:
			if(pMsg->updateMsg.flags & RSSL_UPMF_HAS_POST_USER_INFO)
			{
				pIter->_curBufPtr += rwfPut32(pIter->_curBufPtr, pMsg->updateMsg.postUserInfo.postUserAddr);
				pIter->_curBufPtr += rwfPut32(pIter->_curBufPtr, pMsg->updateMsg.postUserInfo.postUserId);
			}
			break;

		case RSSL_MC_REFRESH:
			if(pMsg->refreshMsg.flags & RSSL_RFMF_HAS_POST_USER_INFO)
			{
				pIter->_curBufPtr += rwfPut32(pIter->_curBufPtr, pMsg->refreshMsg.postUserInfo.postUserAddr);
				pIter->_curBufPtr += rwfPut32(pIter->_curBufPtr, pMsg->refreshMsg.postUserInfo.postUserId);
			}
			if(pMsg->refreshMsg.flags & RSSL_RFMF_HAS_PART_NUM)
				pIter->_curBufPtr += rwfPutResBitU15(pIter->_curBufPtr, pMsg->refreshMsg.partNum);
			break;

		case RSSL_MC_STATUS:
			if(pMsg->statusMsg.flags & RSSL_STMF_HAS_POST_USER_INFO)
			{
				pIter->_curBufPtr += rwfPut32(pIter->_curBufPtr, pMsg->statusMsg.postUserInfo.postUserAddr);
				pIter->_curBufPtr += rwfPut32(pIter->_curBufPtr, pMsg->statusMsg.postUserInfo.postUserId);
			}
			break;

		case RSSL_MC_GENERIC:
			if(pMsg->genericMsg.flags & RSSL_GNMF_HAS_PART_NUM)
				pIter->_curBufPtr += rwfPutResBitU15(pIter->_curBufPtr, pMsg->genericMsg.partNum);
			break;

		case RSSL_MC_POST:
			if(pMsg->postMsg.flags & RSSL_PSMF_HAS_PART_NUM)
				pIter->_curBufPtr += rwfPutResBitU15(pIter->_curBufPtr, pMsg->postMsg.partNum);
			if(pMsg->postMsg.flags & RSSL_PSMF_HAS_POST_USER_RIGHTS)
				pIter->_curBufPtr += rwfPutResBitU15(pIter->_curBufPtr, pMsg->postMsg.postUserRights);
			break;

		case RSSL_MC_REQUEST:
		case RSSL_MC_CLOSE:
		case RSSL_MC_ACK:
			break;
	}

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet rsslEncodeKeyInternal(RsslEncodeIterator		*pIter,
											const RsslMsgKey		*pKey )
{
    RsslUInt16 flags;
 
	if (_rsslIteratorOverrun(pIter, 2))
		return(RSSL_RET_BUFFER_TOO_SMALL);

    /* Store flags as UInt16 */
    flags = pKey->flags;

    if( ( flags & RSSL_MKF_HAS_NAME )  && (pKey->name.length == 0 || pKey->name.data == 0) )
        flags &= ~RSSL_MKF_HAS_NAME;

	if ((!( flags & RSSL_MKF_HAS_NAME)) && (flags & RSSL_MKF_HAS_NAME_TYPE))
		flags &= ~RSSL_MKF_HAS_NAME_TYPE;

	if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((flags < 0x80) ? 1 : 2) , pIter->_endBufPtr))
			return(RSSL_RET_BUFFER_TOO_SMALL);
 
	pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, flags );

    /* Store SourceId as UINt16_ob */
    if( flags & RSSL_MKF_HAS_SERVICE_ID )
	{
		if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((pKey->serviceId < 0xFE) ? 1 : 3) , pIter->_endBufPtr))
				return(RSSL_RET_BUFFER_TOO_SMALL);
    	pIter->_curBufPtr += rwfPutOptByteU16( pIter->_curBufPtr, pKey->serviceId );
	}

    /* Store name as CharPtr == SmallBuffer */
    if( flags & RSSL_MKF_HAS_NAME )
    {
		/* verify name length is only 1 byte */
		if (_rsslIteratorOverrun(pIter, (pKey->name.length + 1)))
			return (RSSL_RET_BUFFER_TOO_SMALL);
		/* ensure key name does not exceed length */
		if (pKey->name.length > RWF_MAX_8)
			return RSSL_RET_INVALID_DATA;
	
		pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pKey->name);

		/* need to allow name types with no name for COOKIE.  Also users could have their own that dont have name  payload */
		/* should only do things with NameType if we have name */
		if ( flags & RSSL_MKF_HAS_NAME_TYPE)
		{
			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 1 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, pKey->nameType);
		}
    }

    /* Store Filter as UInt32 */
    if( flags & RSSL_MKF_HAS_FILTER )
	{
		if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
				return(RSSL_RET_BUFFER_TOO_SMALL);
        pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pKey->filter );
	}

	/* Store Identifier as UInt32 */
    if( flags & RSSL_MKF_HAS_IDENTIFIER )
	{
		if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
				return(RSSL_RET_BUFFER_TOO_SMALL);
        pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pKey->identifier );
	}

    return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet _rsslEncodeMsgReqKey(RsslEncodeIterator *pIter, RsslMsg *pMsg, RsslBool *pReqKeyAttrib)
{
	RsslRet			retVal_Check = RSSL_RET_SUCCESS;
	RsslUInt16		keySize;
	char*			lenPos;
	RsslMsgKey		*key = NULL;

	*pReqKeyAttrib = RSSL_FALSE;

	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			if( pMsg->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY )
				key = &pMsg->refreshMsg.reqMsgKey;
			break;

		case RSSL_MC_STATUS:
			if( pMsg->statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY )
				key = &pMsg->statusMsg.reqMsgKey;
			break;

		case RSSL_MC_GENERIC:
			if( pMsg->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY )
				key = &pMsg->genericMsg.reqMsgKey;
			break;

		case RSSL_MC_UPDATE:
		case RSSL_MC_POST:
		case RSSL_MC_REQUEST:
		case RSSL_MC_CLOSE:
		case RSSL_MC_ACK:
		default:
			break;
	}
	if (key)
	{
		/* save position for storing key size */
		lenPos = pIter->_curBufPtr;
		pIter->_curBufPtr += 2;

		if ((retVal_Check = rsslEncodeKeyInternal( pIter, key)) < 0)
			return retVal_Check;

		/* Store attrib as SmallBuffer */
		if( key->flags & RSSL_MKF_HAS_ATTRIB ) 
		{
			/* write attrib data format and save length position */
			if (_rsslIteratorOverrun(pIter, 3))
				return RSSL_RET_BUFFER_TOO_SMALL;
							
			RSSL_ASSERT(_rsslValidAggregateDataType(key->attribContainerType), Invalid container type);
			/* opaque container type needs to be scaled before encoding */
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (key->attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN));
					
			/* if we have attribs here, put them on the wire */
			if (key->encAttrib.length && key->encAttrib.data)
			{
				/* encode length as RB15 and put data afterwards */
				if (key->attribContainerType != RSSL_DT_NO_DATA)
				{
					if (_rsslIteratorOverrun(pIter, (key->encAttrib.length + ((key->encAttrib.length < 0x80) ? 1 : 2))))
						return RSSL_RET_BUFFER_TOO_SMALL;
					if (key->encAttrib.length > RWF_MAX_U15)
						return RSSL_RET_INVALID_DATA;
					pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &key->encAttrib);
				}
			}
			else
			{
				/* attribInfo needs to be encoded */
				/* save U15 mark */
				if (key->attribContainerType != RSSL_DT_NO_DATA)
					pIter->_curBufPtr = _rsslSetupU15Mark(&pIter->_levelInfo[pIter->_encodingLevel]._internalMark2, 0, pIter->_curBufPtr);
				else
				{
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizeBytes = 0;
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizePtr = pIter->_curBufPtr;
				}
				*pReqKeyAttrib = RSSL_TRUE;
			}
		}
				
		/* user is done with key and there are no attribs they still need to write */
		if (!(*pReqKeyAttrib))
		{
			/* write key length */
			/* now store the size - have to trick it into being an RB15 */
			/* only want the encoded size of the key */
			keySize = (RsslUInt16)(pIter->_curBufPtr - lenPos - 2);
			/* now set the RB bit */
			keySize |= 0x8000;
			/* store it - dont need to increment iterator because its already at end of key */
			rwfPut16(lenPos, keySize);
		}
		else
		{
			/* user still has to encode attribs for request key */
			/* store this in the internalMark to fill in later */
			pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 2;
			pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = lenPos;
		}
	}
	return retVal_Check;
}

static RsslRet RTR_FASTCALL rsslEncodeMsgInternal(	RsslEncodeIterator	*pIter,
											const RsslMsg				*pMsg,
											RsslBool					*pKeyOpaque,
                                            RsslBool                    *pExtendedHeader,
											RsslBool					*pReqKey )
{
	RsslRet	retVal_Check	= RSSL_RET_SUCCESS;
	RsslUInt16		flags;
	RsslUInt16		groupId;
	RsslUInt16		keySize;
	char*			lenPos;

	RSSL_ASSERT( pKeyOpaque, Invalid parameters or parameters passed in as NULL );
    RSSL_ASSERT( pExtendedHeader, Invalid parameters or parameters passed in as NULL );
	/* ensure container type is valid */
	RSSL_ASSERT(_rsslValidAggregateDataType(pMsg->msgBase.containerType), Invalid container type);

	*pKeyOpaque     = RSSL_FALSE;
    *pExtendedHeader = RSSL_FALSE;
    *pReqKey = RSSL_FALSE;

	/* make sure required elements can be encoded */
	/* msgClass (1) domainType(1) stream id (4) */
	if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 6, pIter->_endBufPtr))
		return(RSSL_RET_BUFFER_TOO_SMALL);

	/* Store msgClass as UInt8 */
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pMsg->msgBase.msgClass );

	/* Store domainType as UInt8 */
	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pMsg->msgBase.domainType );

	/* Store streamId as Int32 */
	pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->msgBase.streamId );

    /* IMPORTANT: When new message classes are added, rsslCopyMsg and rsslValidateMsg have to modified as well */

    switch( pMsg->msgBase.msgClass )
	{
		case RSSL_MC_UPDATE: 
			/* Store update flags  as UInt15 */
			flags = pMsg->updateMsg.flags;

			if( flags & RSSL_UPMF_HAS_PERM_DATA && pMsg->updateMsg.permData.length == 0 )
				flags &= ~RSSL_UPMF_HAS_PERM_DATA;
			if( flags & RSSL_UPMF_HAS_CONF_INFO && (pMsg->updateMsg.conflationCount == 0 && pMsg->updateMsg.conflationTime == 0))
				flags &= ~RSSL_UPMF_HAS_CONF_INFO;
			
			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((flags < 0x80) ? 3 : 4) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

    		/* Store update flags as UInt15 */ 
			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, flags );

			/* Store containerType as UInt8 */ 
			/* container type needs to be scaled before encoding */
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN));			

			/* Store update type as UInt8 */
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pMsg->updateMsg.updateType );

			if( flags & RSSL_UPMF_HAS_SEQ_NUM )
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

	    		pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->updateMsg.seqNum );
			}

			if( flags & RSSL_UPMF_HAS_CONF_INFO )
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((pMsg->updateMsg.conflationCount < 0x80) ? 3 : 4) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

    			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, pMsg->updateMsg.conflationCount );
				pIter->_curBufPtr += rwfPut16( pIter->_curBufPtr, pMsg->updateMsg.conflationTime );
			}

			/* Store Perm info */
			if( flags & RSSL_UPMF_HAS_PERM_DATA )
			{

				if (_rsslIteratorOverrun(pIter, pMsg->updateMsg.permData.length + ((pMsg->updateMsg.permData.length < 0x80) ? 1 : 2)))
					return RSSL_RET_BUFFER_TOO_SMALL;
				if (pMsg->updateMsg.permData.length > RWF_MAX_U15)
					return RSSL_RET_INVALID_DATA;
			
				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->updateMsg.permData);
			}

			if( flags & RSSL_UPMF_HAS_MSG_KEY )
			{
				/* save position for storing key size */
				lenPos = pIter->_curBufPtr;
				pIter->_curBufPtr += 2;

				if ((retVal_Check = rsslEncodeKeyInternal( pIter, &pMsg->msgBase.msgKey)) < 0)
					break;
				
				 /* Store opaque as SmallBuffer */
				if( pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) 
				{
					/* write opaque data format and save length position */
					if (_rsslIteratorOverrun(pIter, 3))
						return RSSL_RET_BUFFER_TOO_SMALL;
						
					RSSL_ASSERT(_rsslValidAggregateDataType(pMsg->msgBase.msgKey.attribContainerType), Invalid container type);
					/* opaque container type needs to be scaled before encoding */
					pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.msgKey.attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN));
					
					/* if we have a key opaque here, put it on the wire */
					if (pMsg->msgBase.msgKey.encAttrib.length && pMsg->msgBase.msgKey.encAttrib.data)
					{
						/* encode length as RB15 and put data afterwards */
						if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
						{
							if (_rsslIteratorOverrun(pIter, (pMsg->msgBase.msgKey.encAttrib.length + ((pMsg->msgBase.msgKey.encAttrib.length < 0x80) ? 1 : 2))))
								return RSSL_RET_BUFFER_TOO_SMALL;
							if (pMsg->msgBase.msgKey.encAttrib.length > RWF_MAX_U15)
								return RSSL_RET_INVALID_DATA;
							pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->msgBase.msgKey.encAttrib);
						}
					}
					else
					{
						/* opaque needs to be encoded */
						/* save U15 mark */
						 if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
							 pIter->_curBufPtr = _rsslSetupU15Mark(&pIter->_levelInfo[pIter->_encodingLevel]._internalMark2, 0, pIter->_curBufPtr);
						 else
						 {
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizeBytes = 0;
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizePtr = pIter->_curBufPtr;
						 }
						 *pKeyOpaque = RSSL_TRUE;
					}
				}
				
				/* user is done with key and there is no opaque they still need to write */
				if (!(*pKeyOpaque))
				{
					/* write key length */
					/* now store the size - have to trick it into being an RB15 */
					/* only want the encoded size of the key */
					keySize = (RsslUInt16)((pIter->_curBufPtr - lenPos - 2));
					/* now set the RB bit */
					keySize |= 0x8000;
					/* store it - dont need to increment iterator because its already at end of key */
					rwfPut16(lenPos, keySize);
				}
				else
				{
					/* user still has to encode key opaque */
					/* store this in the internalMark to fill in later */
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 2;
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = lenPos;
				}
			}

            if( flags & RSSL_UPMF_HAS_EXTENDED_HEADER )
            {
				/* user passed it in, lets encode it now */
				if (*pKeyOpaque)
				{
					/* User set flag to indicate there is an extended header but they didnt encode their opaque yet */
					/* set us up to expect opaque */
					*pExtendedHeader = RSSL_TRUE;
				}
				else
				{
					/* no key opaque - if the extended header is here, put it on the wire, otherwise
					   set it up so we expect extended header */
					if( pMsg->updateMsg.extendedHeader.length && pMsg->updateMsg.extendedHeader.data  )
					{
						if (_rsslIteratorOverrun(pIter, pMsg->updateMsg.extendedHeader.length + 1))
							return RSSL_RET_BUFFER_TOO_SMALL;

						if (pMsg->updateMsg.extendedHeader.length > RWF_MAX_8)
							return RSSL_RET_INVALID_DATA;
								
						pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->updateMsg.extendedHeader);
					}
					else
					{
						if (_rsslIteratorOverrun(pIter, 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
						
						*pExtendedHeader = RSSL_TRUE;
						/* must reserve space now */
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = pIter->_curBufPtr;
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 1;
						pIter->_curBufPtr++; /* move pointer */
					}
				}
            }
			break; 

		case RSSL_MC_GENERIC: 
			/* Store update flags  as UInt15 */
			flags = pMsg->genericMsg.flags;

			if( flags & RSSL_GNMF_HAS_PERM_DATA && pMsg->genericMsg.permData.length == 0 )
				flags &= ~RSSL_GNMF_HAS_PERM_DATA;
						
			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((flags < 0x80) ? 2 : 3) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

    		/* Store update flags as UInt15 */ 
			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, flags );

			/* Store containerType as UInt8 */ 
			/* container type needs to be scaled before encoding */
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN) );			

			if( flags & RSSL_GNMF_HAS_SEQ_NUM )
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
	    		pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->genericMsg.seqNum );
			}
			
			if ( flags & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM )
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
				pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->genericMsg.secondarySeqNum );
			}
			
			/* Store Perm info */
			if( flags & RSSL_GNMF_HAS_PERM_DATA )
			{
				if (_rsslIteratorOverrun(pIter, pMsg->genericMsg.permData.length + ((pMsg->genericMsg.permData.length < 0x80) ? 1 : 2)))
					return RSSL_RET_BUFFER_TOO_SMALL;
				if (pMsg->genericMsg.permData.length > RWF_MAX_U15)
					return RSSL_RET_INVALID_DATA;
					
				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->genericMsg.permData);
			}

			if( flags & RSSL_GNMF_HAS_MSG_KEY )
			{
				/* save position for storing key size */
				lenPos = pIter->_curBufPtr;
				pIter->_curBufPtr += 2;

				if ((retVal_Check = rsslEncodeKeyInternal( pIter, &pMsg->msgBase.msgKey)) < 0)
					break;
				
				 /* Store opaque as SmallBuffer */
				if( pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) 
				{

					/* write opaque data format and save length position */
					if (_rsslIteratorOverrun(pIter, 3))
						return RSSL_RET_BUFFER_TOO_SMALL;
						
					RSSL_ASSERT(_rsslValidAggregateDataType(pMsg->msgBase.msgKey.attribContainerType), Invalid container type);
					/* opaque container type needs to be scaled before encoding */
					pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.msgKey.attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN));
					
					/* if we have a key opaque here, put it on the wire */
					if (pMsg->msgBase.msgKey.encAttrib.length && pMsg->msgBase.msgKey.encAttrib.data)
					{
						/* encode length as RB15 and put data afterwards */
						if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
						{
							if (_rsslIteratorOverrun(pIter, (pMsg->msgBase.msgKey.encAttrib.length + ((pMsg->msgBase.msgKey.encAttrib.length < 0x80) ? 1 : 2))))
								return RSSL_RET_BUFFER_TOO_SMALL;
							if (pMsg->msgBase.msgKey.encAttrib.length > RWF_MAX_U15)
								return RSSL_RET_INVALID_DATA;
							pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->msgBase.msgKey.encAttrib);
						}
					}
					else
					{
						/* opaque needs to be encoded */
						/* save U15 mark */
						 if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
							 pIter->_curBufPtr = _rsslSetupU15Mark(&pIter->_levelInfo[pIter->_encodingLevel]._internalMark2, 0, pIter->_curBufPtr);
						 else
						 {
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizeBytes = 0;
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizePtr = pIter->_curBufPtr;
						 }
						 *pKeyOpaque = RSSL_TRUE;
					}
				}
				
				/* user is done with key and there is no opaque they still need to write */
				if (!(*pKeyOpaque))
				{
					/* write key length */
					/* now store the size - have to trick it into being an RB15 */
					/* only want the encoded size of the key */
					keySize = (RsslUInt16)((pIter->_curBufPtr - lenPos - 2));
					/* now set the RB bit */
					keySize |= 0x8000;
					/* store it - dont need to increment iterator because its already at end of key */
					rwfPut16(lenPos, keySize);
				}
				else
				{
					/* user still has to encode key opaque */
					/* store this in the internalMark to fill in later */
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 2;
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = lenPos;
				}
			}

            if( flags & RSSL_GNMF_HAS_EXTENDED_HEADER )
            {
				/* user passed it in, lets encode it now */
				if (*pKeyOpaque)
				{
					/* User set flag to indicate there is an extended header but they didnt encode their opaque yet */
					/* set us up to expect opaque */
					*pExtendedHeader = RSSL_TRUE;
				}
				else
				{
					/* no key opaque - if the extended header is here, put it on the wire, otherwise
					   set it up so we expect extended header */
					if( pMsg->genericMsg.extendedHeader.length && pMsg->genericMsg.extendedHeader.data  )
					{
						if (_rsslIteratorOverrun(pIter, pMsg->genericMsg.extendedHeader.length + 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
						if (pMsg->genericMsg.extendedHeader.length > RWF_MAX_8)
							return RSSL_RET_INVALID_DATA;
										
						pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->genericMsg.extendedHeader);
					}
					else
					{
						if (_rsslIteratorOverrun(pIter, 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
							
						*pExtendedHeader = RSSL_TRUE;
						/* must reserve space now */
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = pIter->_curBufPtr;
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 1;
						pIter->_curBufPtr++; /* move pointer */
					}
				}
            }
			if( flags & RSSL_GNMF_HAS_REQ_MSG_KEY )
			{
				*pReqKey = RSSL_TRUE;
			}
			break; 

		case RSSL_MC_REFRESH:			
			/* Store refresh flags  as UInt16, cleaning perm flag if necessary */
			flags = pMsg->refreshMsg.flags;
			if( flags & RSSL_RFMF_HAS_PERM_DATA && pMsg->refreshMsg.permData.length == 0 )
				flags &= ~RSSL_RFMF_HAS_PERM_DATA;

			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((flags < 0x80) ? 2 : 3) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, flags );
			
    		/* Store containerType as UInt8 */ 
			/* container type needs to be scaled */
	    	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN) );

			if( flags & RSSL_RFMF_HAS_SEQ_NUM )
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
	    		pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->refreshMsg.seqNum );
			}
	
			if ((retVal_Check = _rsslEncodeStateInt(pIter, &pMsg->refreshMsg.state)) < 0)
				break;

			/* Store groupId as small buffer */
			if ((pMsg->refreshMsg.groupId.data) && (pMsg->refreshMsg.groupId.length > 0))
			{
				if (_rsslIteratorOverrun(pIter, pMsg->refreshMsg.groupId.length +  1))
					return RSSL_RET_BUFFER_TOO_SMALL;
				if (pMsg->refreshMsg.groupId.length > RWF_MAX_8)
					return RSSL_RET_INVALID_DATA;
					
				pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->refreshMsg.groupId);
			}
			else
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 3 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
				/* No group Id - store 0 in the format used by group mechanism */
				/* length is 2 */
				groupId = 2;
				/* write length */
				pIter->_curBufPtr += rwfPutOptByteU16(pIter->_curBufPtr, groupId);
				/* now write value */
				groupId = 0;
				pIter->_curBufPtr += rwfPut16(pIter->_curBufPtr, groupId);
			}
				
			if( flags & RSSL_RFMF_HAS_PERM_DATA )
			{
				if (_rsslIteratorOverrun(pIter, pMsg->refreshMsg.permData.length + ((pMsg->refreshMsg.permData.length < 0x80) ? 1 : 2)))
					return RSSL_RET_BUFFER_TOO_SMALL;
				if (pMsg->refreshMsg.permData.length > RWF_MAX_U15)
					return RSSL_RET_INVALID_DATA;
				
				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->refreshMsg.permData);
			}
	
			/* Store QoS */
			if( flags & RSSL_RFMF_HAS_QOS )
			{
				if ((retVal_Check = _rsslEncodeQosInt(pIter, &pMsg->refreshMsg.qos)) < 0)
					break;
			}
			
			if( flags & RSSL_RFMF_HAS_MSG_KEY )
			{
				/* save position for storing key size */
				lenPos = pIter->_curBufPtr;
				pIter->_curBufPtr += 2;

				if ((retVal_Check = rsslEncodeKeyInternal( pIter, &pMsg->msgBase.msgKey)) < 0)
					break;
				
				 /* Store opaque as SmallBuffer */
				if( pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) 
				{

					/* write opaque data format and save length position */
					if (_rsslIteratorOverrun(pIter, 3))
						return RSSL_RET_BUFFER_TOO_SMALL;
						
					RSSL_ASSERT(_rsslValidAggregateDataType(pMsg->msgBase.msgKey.attribContainerType), Invalid container type);
					/* opaque container type needs to be scaled before encoding */
					pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.msgKey.attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN));
					
					/* if we have a key opaque here, put it on the wire */
					if (pMsg->msgBase.msgKey.encAttrib.length && pMsg->msgBase.msgKey.encAttrib.data)
					{
						/* encode length as RB15 and put data afterwards */
						if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
						{
							if (_rsslIteratorOverrun(pIter, (pMsg->msgBase.msgKey.encAttrib.length + ((pMsg->msgBase.msgKey.encAttrib.length < 0x80) ? 1 : 2))))
								return RSSL_RET_BUFFER_TOO_SMALL;
							if (pMsg->msgBase.msgKey.encAttrib.length > RWF_MAX_U15)
								return RSSL_RET_INVALID_DATA;
							pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->msgBase.msgKey.encAttrib);
						}
					}
					else
					{
						/* opaque needs to be encoded */
						/* save U15 mark */
						 if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
							 pIter->_curBufPtr = _rsslSetupU15Mark(&pIter->_levelInfo[pIter->_encodingLevel]._internalMark2, 0, pIter->_curBufPtr);
						 else
						 {
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizeBytes = 0;
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizePtr = pIter->_curBufPtr;
						 }
						 *pKeyOpaque = RSSL_TRUE;
					}
				}
				
				/* user is done with key and there is no opaque they still need to write */
				if (!(*pKeyOpaque))
				{
					/* write key length */
					/* now store the size - have to trick it into being an RB15 */
					/* only want the encoded size of the key */
					keySize = (RsslUInt16)((pIter->_curBufPtr - lenPos - 2));
					/* now set the RB bit */
					keySize |= 0x8000;
					/* store it - dont need to increment iterator because its already at end of key */
					rwfPut16(lenPos, keySize);
				}
				else
				{
					/* user still has to encode key opaque */
					/* store this in the internalMark to fill in later */
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 2;
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = lenPos;
				}
			}

            if( flags & RSSL_RFMF_HAS_EXTENDED_HEADER )
            {
				/* user passed it in, lets encode it now */
				if (*pKeyOpaque)
				{
					/* User set flag to indicate there is an extended header but they didnt encode their opaque yet */
					/* set us up to expect opaque */
					*pExtendedHeader = RSSL_TRUE;
				}
				else
				{
					/* no key opaque - if the extended header is here, put it on the wire, otherwise
					   set it up so we expect extended header */
					if( pMsg->refreshMsg.extendedHeader.length && pMsg->refreshMsg.extendedHeader.data  )
					{
					
						if (_rsslIteratorOverrun(pIter, pMsg->refreshMsg.extendedHeader.length + 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
						if (pMsg->refreshMsg.extendedHeader.length > RWF_MAX_8)
							return RSSL_RET_INVALID_DATA;
										
						pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->refreshMsg.extendedHeader);
					}
					else
					{
						if (_rsslIteratorOverrun(pIter, 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
							
						*pExtendedHeader = RSSL_TRUE;
						/* must reserve space now */
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = pIter->_curBufPtr;
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 1;
						pIter->_curBufPtr++; /* move pointer */
					}
				}
            }

			if( flags & RSSL_RFMF_HAS_REQ_MSG_KEY )
			{
				*pReqKey = RSSL_TRUE;
			}
		   	break; 

		case RSSL_MC_POST:
			flags = pMsg->postMsg.flags;

			if(flags & RSSL_PSMF_HAS_PERM_DATA && (pMsg->postMsg.permData.length == 0 || pMsg->postMsg.permData.data == 0) )
				flags &= ~RSSL_PSMF_HAS_PERM_DATA;

			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((flags < 0x80) ? 10 : 11) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

			/* Store flags as UInt15 */
			pIter->_curBufPtr += rwfPutResBitU15(pIter->_curBufPtr, flags);
			
			/* Store containerType as UInt8 */
			/* container type needs to be scaled */
			pIter->_curBufPtr += rwfPut8(pIter->_curBufPtr, (pMsg->msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN));

			/* Put User Address */
			pIter->_curBufPtr += rwfPut32(pIter->_curBufPtr, pMsg->postMsg.postUserInfo.postUserAddr);

			/* Put User ID */
			pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->postMsg.postUserInfo.postUserId );

			if( flags & RSSL_PSMF_HAS_SEQ_NUM )
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
	    		pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->postMsg.seqNum );
			}
			
			if( flags & RSSL_PSMF_HAS_POST_ID )
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
	    		pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->postMsg.postId );
			}

			if( flags & RSSL_PSMF_HAS_PERM_DATA )
			{
				if (_rsslIteratorOverrun(pIter, pMsg->postMsg.permData.length + ((pMsg->postMsg.permData.length < 0x80) ? 1 : 2)))
					return RSSL_RET_BUFFER_TOO_SMALL;
				if (pMsg->postMsg.permData.length > RWF_MAX_U15)
					return RSSL_RET_INVALID_DATA;
				
				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->postMsg.permData);
			}
		
			if( flags & RSSL_PSMF_HAS_MSG_KEY )
			{
				/* save position for storing key size */
				lenPos = pIter->_curBufPtr;
				pIter->_curBufPtr += 2;

				if ((retVal_Check = rsslEncodeKeyInternal( pIter, &pMsg->msgBase.msgKey)) < 0)
					break;
				
				 /* Store opaque as SmallBuffer */
				if( pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) 
				{
					/* write opaque data format and save length position */
					if (_rsslIteratorOverrun(pIter, 3))
						return RSSL_RET_BUFFER_TOO_SMALL;
						
					RSSL_ASSERT(_rsslValidAggregateDataType(pMsg->msgBase.msgKey.attribContainerType), Invalid container type);
					/* opaque container type needs to be scaled before encoding */
					pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.msgKey.attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN));
					
					/* if we have a key opaque here, put it on the wire */
					if (pMsg->msgBase.msgKey.encAttrib.length && pMsg->msgBase.msgKey.encAttrib.data)
					{
						/* encode length as RB15 and put data afterwards */
						if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
						{
							if (_rsslIteratorOverrun(pIter, (pMsg->msgBase.msgKey.encAttrib.length + ((pMsg->msgBase.msgKey.encAttrib.length < 0x80) ? 1 : 2))))
								return RSSL_RET_BUFFER_TOO_SMALL;
							if (pMsg->msgBase.msgKey.encAttrib.length > RWF_MAX_U15)
								return RSSL_RET_INVALID_DATA;
							pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->msgBase.msgKey.encAttrib);
						}
					}
					else
					{
						/* opaque needs to be encoded */
						/* save U15 mark */
						 if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
							 pIter->_curBufPtr = _rsslSetupU15Mark(&pIter->_levelInfo[pIter->_encodingLevel]._internalMark2, 0, pIter->_curBufPtr);
						 else
						 {
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizeBytes = 0;
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizePtr = pIter->_curBufPtr;
						 }
						 *pKeyOpaque = RSSL_TRUE;
					}
				}
				
				/* user is done with key and there is no opaque they still need to write */
				if (!(*pKeyOpaque))
				{
					/* write key length */
					/* now store the size - have to trick it into being an RB15 */
					/* only want the encoded size of the key */
					keySize = (RsslUInt16)((pIter->_curBufPtr - lenPos - 2));
					/* now set the RB bit */
					keySize |= 0x8000;
					/* store it - dont need to increment iterator because its already at end of key */
					rwfPut16(lenPos, keySize);
				}
				else
				{
					/* user still has to encode key opaque */
					/* store this in the internalMark to fill in later */
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 2;
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = lenPos;
				}
			}

            if( flags & RSSL_PSMF_HAS_EXTENDED_HEADER )
            {
				/* user passed it in, lets encode it now */
				if (*pKeyOpaque)
				{
					/* User set flag to indicate there is an extended header but they didnt encode their opaque yet */
					/* set us up to expect opaque */
					*pExtendedHeader = RSSL_TRUE;
				}
				else
				{
					/* no key opaque - if the extended header is here, put it on the wire, otherwise
					   set it up so we expect extended header */
					if( pMsg->postMsg.extendedHeader.length && pMsg->postMsg.extendedHeader.data  )
					{
					
						if (_rsslIteratorOverrun(pIter, pMsg->postMsg.extendedHeader.length + 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
						if (pMsg->postMsg.extendedHeader.length > RWF_MAX_8)
							return RSSL_RET_INVALID_DATA;
										
						pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->postMsg.extendedHeader);
					}
					else
					{
						if (_rsslIteratorOverrun(pIter, 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
							
						*pExtendedHeader = RSSL_TRUE;
						/* must reserve space now */
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = pIter->_curBufPtr;
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 1;
						pIter->_curBufPtr++; /* move pointer */
					}
				}
            }
		   break;

		case RSSL_MC_REQUEST: 
			/* Make sure elements can be encoded */
			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 3, pIter->_endBufPtr))
				return(RSSL_RET_BUFFER_TOO_SMALL);

			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((pMsg->requestMsg.flags < 0x80) ? 2 : 3) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

			/* Store flags as UInt8 */
			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, pMsg->requestMsg.flags );

    		/* Store containerType as UInt8 */ 
			/* container type needs to be scaled */
	    	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN) );

            if( pMsg->requestMsg.flags & RSSL_RQMF_HAS_PRIORITY )
            {
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((pMsg->requestMsg.priorityCount < 0xFE) ? 2 : 4) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
    		    /* Store PriorityClass as UInt8 */ 
	    	    pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pMsg->requestMsg.priorityClass );
    		    /* Store PriorityCount as UInt16_ob */ 
	    	    pIter->_curBufPtr += rwfPutOptByteU16( pIter->_curBufPtr, pMsg->requestMsg.priorityCount );
            }

			/* Store QoS */
			if( pMsg->requestMsg.flags & RSSL_RQMF_HAS_QOS )
			{
				if ((retVal_Check = _rsslEncodeQosInt(pIter, &pMsg->requestMsg.qos)) < 0)
					break;
			}

			/* Store WorstQoS */
			if( pMsg->requestMsg.flags & RSSL_RQMF_HAS_WORST_QOS )
			{
				if ((retVal_Check = _rsslEncodeQosInt(pIter,  &pMsg->requestMsg.worstQos )) < 0)
					break;
			}
			
			/* save position for storing key size */
			lenPos = pIter->_curBufPtr;
			pIter->_curBufPtr += 2;

			if ((retVal_Check = rsslEncodeKeyInternal( pIter, &pMsg->msgBase.msgKey)) < 0)
				break;
			
			 /* Store opaque as SmallBuffer */
			if( pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) 
			{
				/* write opaque data format and save length position */
				if (_rsslIteratorOverrun(pIter, 3))
					return RSSL_RET_BUFFER_TOO_SMALL;
					
				RSSL_ASSERT(_rsslValidAggregateDataType(pMsg->msgBase.msgKey.attribContainerType), Invalid container type);
				/* opaque container type needs to be scaled before encoding */
				pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.msgKey.attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN));
				
				/* if we have a key opaque here, put it on the wire */
				if (pMsg->msgBase.msgKey.encAttrib.length && pMsg->msgBase.msgKey.encAttrib.data)
				{
					/* encode length as RB15 and put data afterwards */
					if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
					{
						if (_rsslIteratorOverrun(pIter, (pMsg->msgBase.msgKey.encAttrib.length + ((pMsg->msgBase.msgKey.encAttrib.length < 0x80) ? 1 : 2))))
							return RSSL_RET_BUFFER_TOO_SMALL;
						if (pMsg->msgBase.msgKey.encAttrib.length > RWF_MAX_U15)
							return RSSL_RET_INVALID_DATA;
						pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->msgBase.msgKey.encAttrib);
					}
				}
				else
				{
					/* opaque needs to be encoded */
					/* save U15 mark */
					 if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
 						 pIter->_curBufPtr = _rsslSetupU15Mark(&pIter->_levelInfo[pIter->_encodingLevel]._internalMark2, 0, pIter->_curBufPtr);
					 else
					 {
						 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizeBytes = 0;
						 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizePtr = pIter->_curBufPtr;
					 }
					 *pKeyOpaque = RSSL_TRUE;
				}
			}
			
			/* user is done with key and there is no opaque they still need to write */
			if (!(*pKeyOpaque))
			{
				/* write key length */
				/* now store the size - have to trick it into being an RB15 */
				/* only want the encoded size of the key */
				keySize = (RsslUInt16)((pIter->_curBufPtr - lenPos - 2));
				/* now set the RB bit */
				keySize |= 0x8000;
				/* store it - dont need to increment iterator because its already at end of key */
				rwfPut16(lenPos, keySize);
			}
			else
			{
				/* user still has to encode key opaque */
				/* store this in the internalMark to fill in later */
				pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 2;
				pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = lenPos;
			}
		
            if( pMsg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER )
            {
				/* user passed it in, lets encode it now */
				if (*pKeyOpaque)
				{
					/* User set flag to indicate there is an extended header but they didnt encode their opaque yet */
					/* set us up to expect opaque */
					*pExtendedHeader = RSSL_TRUE;
				}
				else
				{
					/* no key opaque - if the extended header is here, put it on the wire, otherwise
					   set it up so we expect extended header */
					if( pMsg->requestMsg.extendedHeader.length && pMsg->requestMsg.extendedHeader.data  )
					{
					
						if (_rsslIteratorOverrun(pIter, pMsg->requestMsg.extendedHeader.length + 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
						if (pMsg->requestMsg.extendedHeader.length > RWF_MAX_8)
							return RSSL_RET_INVALID_DATA;
										
						pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->requestMsg.extendedHeader);
					}
					else
					{
						if (_rsslIteratorOverrun(pIter, 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
							
						*pExtendedHeader = RSSL_TRUE;
						/* must reserve space now */
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = pIter->_curBufPtr;
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 1;
						pIter->_curBufPtr++; /* move pointer */
					}
				}
            }
			break; 

		case RSSL_MC_STATUS: 
			/* Store status flags  as UInt15, cleaning perm flag if necessary */
			flags = pMsg->statusMsg.flags;
			if( flags & RSSL_STMF_HAS_PERM_DATA && pMsg->statusMsg.permData.length == 0 )
				flags &= ~RSSL_STMF_HAS_PERM_DATA;
			if( flags & RSSL_STMF_HAS_GROUP_ID && pMsg->statusMsg.groupId.length == 0 )
				flags &= ~RSSL_STMF_HAS_GROUP_ID;
		
			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((flags < 0x80) ? 2 : 3) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, flags );
			
    		/* Store containerType as UInt8 */ 
			/* container type needs to be scaled */
	    	pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN) );

            /* Store  state */
            if( flags & RSSL_STMF_HAS_STATE )
            {
				if ((retVal_Check = _rsslEncodeStateInt(pIter, &pMsg->statusMsg.state)) < 0)
					break;
            }
            
			/* Store groupId as small buffer */
			if( flags & RSSL_STMF_HAS_GROUP_ID )
			{
				if (_rsslIteratorOverrun(pIter, pMsg->statusMsg.groupId.length + 1))
					return RSSL_RET_BUFFER_TOO_SMALL;
				if (pMsg->statusMsg.groupId.length > RWF_MAX_8)
					return RSSL_RET_INVALID_DATA;
				
				pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->statusMsg.groupId);
			}
		
			/* Store Perm info */
			if( flags & RSSL_STMF_HAS_PERM_DATA )
			{
				if (_rsslIteratorOverrun(pIter, pMsg->statusMsg.permData.length + ((pMsg->statusMsg.permData.length < 0x80) ? 1 : 2)))
					return RSSL_RET_BUFFER_TOO_SMALL;
				if (pMsg->statusMsg.permData.length > RWF_MAX_U15)
					return RSSL_RET_INVALID_DATA;
				pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->statusMsg.permData);
			}

			if( flags & RSSL_STMF_HAS_MSG_KEY )
			{
				/* save position for storing key size */
				lenPos = pIter->_curBufPtr;
				pIter->_curBufPtr += 2;

				if ((retVal_Check = rsslEncodeKeyInternal( pIter, &pMsg->msgBase.msgKey)) < 0)
					break;
				
				 /* Store opaque as SmallBuffer */
				if( pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) 
				{
					/* write opaque data format and save length position */
					if (_rsslIteratorOverrun(pIter, 3))
						return RSSL_RET_BUFFER_TOO_SMALL;
						
					RSSL_ASSERT(_rsslValidAggregateDataType(pMsg->msgBase.msgKey.attribContainerType), Invalid container type);
					/* opaque container type needs to be scaled before encoding */
					pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.msgKey.attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN));
					
					/* if we have a key opaque here, put it on the wire */
					if (pMsg->msgBase.msgKey.encAttrib.length && pMsg->msgBase.msgKey.encAttrib.data)
					{
						/* encode length as RB15 and put data afterwards */
						if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
						{
							if (_rsslIteratorOverrun(pIter, (pMsg->msgBase.msgKey.encAttrib.length + ((pMsg->msgBase.msgKey.encAttrib.length < 0x80) ? 1 : 2))))
								return RSSL_RET_BUFFER_TOO_SMALL;
							if (pMsg->msgBase.msgKey.encAttrib.length > RWF_MAX_U15)
								return RSSL_RET_INVALID_DATA;
							pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->msgBase.msgKey.encAttrib);
						}
					}
					else
					{
						/* opaque needs to be encoded */
						/* save U15 mark */
						 if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
							 pIter->_curBufPtr = _rsslSetupU15Mark(&pIter->_levelInfo[pIter->_encodingLevel]._internalMark2, 0, pIter->_curBufPtr);
						 else
						 {
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizeBytes = 0;
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizePtr = pIter->_curBufPtr;
						 }
						 *pKeyOpaque = RSSL_TRUE;
					}
				}
				
				/* user is done with key and there is no opaque they still need to write */
				if (!(*pKeyOpaque))
				{
					/* write key length */
					/* now store the size - have to trick it into being an RB15 */
					/* only want the encoded size of the key */
					keySize = (RsslUInt16)((pIter->_curBufPtr - lenPos - 2));
					/* now set the RB bit */
					keySize |= 0x8000;
					/* store it - dont need to increment iterator because its already at end of key */
					rwfPut16(lenPos, keySize);
				}
				else
				{
					/* user still has to encode key opaque */
					/* store this in the internalMark to fill in later */
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 2;
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = lenPos;
				}
			}

           if( flags & RSSL_STMF_HAS_EXTENDED_HEADER )
           {
				/* user passed it in, lets encode it now */
				if (*pKeyOpaque)
				{
					/* User set flag to indicate there is an extended header but they didnt encode their opaque yet */
					/* set us up to expect opaque */
					*pExtendedHeader = RSSL_TRUE;
				}
				else
				{
					/* no key opaque - if the extended header is here, put it on the wire, otherwise
					   set it up so we expect extended header */
					if( pMsg->statusMsg.extendedHeader.length && pMsg->statusMsg.extendedHeader.data  )
					{
						if (_rsslIteratorOverrun(pIter, pMsg->statusMsg.extendedHeader.length + 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
						if (pMsg->statusMsg.extendedHeader.length > RWF_MAX_8)
							return RSSL_RET_INVALID_DATA;
										
						pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->statusMsg.extendedHeader);
					}
					else
					{
						if (_rsslIteratorOverrun(pIter, 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
							
						*pExtendedHeader = RSSL_TRUE;
						/* must reserve space now */
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = pIter->_curBufPtr;
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 1;
						pIter->_curBufPtr++; // move pointer */
					}
				}
            }

			if( flags & RSSL_STMF_HAS_REQ_MSG_KEY )
			{
				*pReqKey = RSSL_TRUE;
			}
			break; 

		case RSSL_MC_CLOSE: 
			/* msgKey.flags must be 0 as close message does not allow a key */
			RSSL_ASSERT(!pMsg->msgBase.msgKey.flags, No message key allowed on RsslCloseMsg);

			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((pMsg->closeMsg.flags < 0x80) ? 2 : 3) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

			/* Store close flags  as UInt16*/
			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, pMsg->closeMsg.flags );

			/* container type needs to be scaled */
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN) );

			if( pMsg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER )
            {
			    if (( pMsg->closeMsg.extendedHeader.length ) && (pMsg->closeMsg.extendedHeader.data))
			    {
					/* now put data header there */
					if (_rsslIteratorOverrun(pIter, pMsg->closeMsg.extendedHeader.length + 1))
						return RSSL_RET_BUFFER_TOO_SMALL;
					if (pMsg->closeMsg.extendedHeader.length > RWF_MAX_8)
						return RSSL_RET_INVALID_DATA;
					
					pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->closeMsg.extendedHeader);
			    }
				else
				{
					if (_rsslIteratorOverrun(pIter, 1))
						return RSSL_RET_BUFFER_TOO_SMALL;
						
					*pExtendedHeader = RSSL_TRUE;
					/* must reserve space now */
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = pIter->_curBufPtr;
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 1;
					pIter->_curBufPtr++; /* move pointer */
				}
            }
			break; 

		case RSSL_MC_ACK: 
			flags = pMsg->ackMsg.flags;

			if ( flags & RSSL_AKMF_HAS_NAK_CODE && pMsg->ackMsg.nakCode == RSSL_NAKC_NONE)
				flags &= ~RSSL_AKMF_HAS_NAK_CODE;

			if ( flags & RSSL_AKMF_HAS_TEXT && pMsg->ackMsg.text.length == 0)
				flags &= ~RSSL_AKMF_HAS_TEXT;

			if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, ((flags < 0x80) ? 6 : 7) , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);

			/* Store flags flags  as UInt16*/
			pIter->_curBufPtr += rwfPutResBitU15( pIter->_curBufPtr, flags );
			
			/* container type needs to be scaled */
			pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.containerType - RSSL_DT_CONTAINER_TYPE_MIN) );
            /* Store ackId as UInt32 */
            pIter->_curBufPtr += rwfPut32( pIter->_curBufPtr, pMsg->ackMsg.ackId );
            if( flags & RSSL_AKMF_HAS_NAK_CODE )
            {
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 1 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
                /* Store nakCode as UInt8 */
                pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, pMsg->ackMsg.nakCode );
			}

			if (flags & RSSL_AKMF_HAS_TEXT)
			{
				if (_rsslIteratorOverrun(pIter, pMsg->ackMsg.text.length + ((pMsg->ackMsg.text.length < 0xFE) ? 1 : 3)))
					return RSSL_RET_BUFFER_TOO_SMALL;
				/* ensure does not overrun buf16 len */
				if (pMsg->ackMsg.text.length > RWF_MAX_16)
					return RSSL_RET_INVALID_DATA;
					
                /* Store text as a string */
                pIter->_curBufPtr = _rsslEncodeBuffer16(pIter->_curBufPtr, &pMsg->ackMsg.text );
			}
				
			if (flags & RSSL_AKMF_HAS_SEQ_NUM)
			{
				if (_rsslBufferOverrunEndPtr(pIter->_curBufPtr, 4 , pIter->_endBufPtr))
					return(RSSL_RET_BUFFER_TOO_SMALL);
				pIter->_curBufPtr += rwfPut32(pIter->_curBufPtr, pMsg->ackMsg.seqNum);
			}

			if( flags & RSSL_AKMF_HAS_MSG_KEY )
			{	
				/* save position for storing key size */
				lenPos = pIter->_curBufPtr;
				pIter->_curBufPtr += 2;

				if ((retVal_Check = rsslEncodeKeyInternal( pIter, &pMsg->msgBase.msgKey)) < 0)
					break;
				
				 /* Store opaque as SmallBuffer */
				if( pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) 
				{
					/* write opaque data format and save length position */
					if (_rsslIteratorOverrun(pIter, 3))
						return RSSL_RET_BUFFER_TOO_SMALL;
						
					RSSL_ASSERT(_rsslValidAggregateDataType(pMsg->msgBase.msgKey.attribContainerType), Invalid container type);
					/* opaque container type needs to be scaled before encoding */
					pIter->_curBufPtr += rwfPut8( pIter->_curBufPtr, (pMsg->msgBase.msgKey.attribContainerType - RSSL_DT_CONTAINER_TYPE_MIN));
					
					/* if we have a key opaque here, put it on the wire */
					if (pMsg->msgBase.msgKey.encAttrib.length && pMsg->msgBase.msgKey.encAttrib.data)
					{
						/* encode length as RB15 and put data afterwards */
						if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
						{
							if (_rsslIteratorOverrun(pIter, (pMsg->msgBase.msgKey.encAttrib.length + ((pMsg->msgBase.msgKey.encAttrib.length < 0x80) ? 1 : 2))))
								return RSSL_RET_BUFFER_TOO_SMALL;
							if (pMsg->msgBase.msgKey.encAttrib.length > RWF_MAX_U15)
								return RSSL_RET_INVALID_DATA;
							pIter->_curBufPtr = _rsslEncodeBuffer15(pIter->_curBufPtr, &pMsg->msgBase.msgKey.encAttrib);
						}
					}
					else
					{
						/* opaque needs to be encoded */
						/* save U15 mark */
						 if (pMsg->msgBase.msgKey.attribContainerType != RSSL_DT_NO_DATA)
							 pIter->_curBufPtr = _rsslSetupU15Mark(&pIter->_levelInfo[pIter->_encodingLevel]._internalMark2, 0, pIter->_curBufPtr);
						 else
						 {
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizeBytes = 0;
							 pIter->_levelInfo[pIter->_encodingLevel]._internalMark2._sizePtr = pIter->_curBufPtr;
						 }
						 *pKeyOpaque = RSSL_TRUE;
					}
				}
				
				/* user is done with key and there is no opaque they still need to write */
				if (!(*pKeyOpaque))
				{
					/* write key length */
					/* now store the size - have to trick it into being an RB15 */
					/* only want the encoded size of the key */
					keySize = (RsslUInt16)((pIter->_curBufPtr - lenPos - 2));
					/* now set the RB bit */
					keySize |= 0x8000;
					/* store it - dont need to increment iterator because its already at end of key */
					rwfPut16(lenPos, keySize);
				}
				else
				{
					/* user still has to encode key opaque */
					/* store this in the internalMark to fill in later */
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 2;
					pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = lenPos;
				}
			}

            if( flags & RSSL_AKMF_HAS_EXTENDED_HEADER )
            {
				/* user passed it in, lets encode it now */
				if (*pKeyOpaque)
				{
					/* User set flag to indicate there is an extended header but they didnt encode their opaque yet */
					/* set us up to expect opaque */
					*pExtendedHeader = RSSL_TRUE;
					/* space will be reserved by the OpaqueComplete function */
				}
				else
				{
					/* no key opaque - if the extended header is here, put it on the wire, otherwise
					   set it up so we expect extended header */
					if( pMsg->ackMsg.extendedHeader.length && pMsg->ackMsg.extendedHeader.data  )
					{
					
						if (_rsslIteratorOverrun(pIter, pMsg->ackMsg.extendedHeader.length + 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
						if (pMsg->ackMsg.extendedHeader.length > RWF_MAX_8)
							return RSSL_RET_INVALID_DATA;
										
						pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, &pMsg->ackMsg.extendedHeader);
					}
					else
					{
						if (_rsslIteratorOverrun(pIter, 1))
							return RSSL_RET_BUFFER_TOO_SMALL;
								
						*pExtendedHeader = RSSL_TRUE;
						/* must reserve space now */
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizePtr = pIter->_curBufPtr;
						pIter->_levelInfo[pIter->_encodingLevel]._internalMark._sizeBytes = 1;
						pIter->_curBufPtr++; /* move pointer */
					}
				}
            }
			break; 

		default:
			retVal_Check = RSSL_RET_INVALID_ARGUMENT;
	}

	return retVal_Check;
}

RSSL_API RsslRet rsslEncodeMsg(RsslEncodeIterator		*pIter,
								RsslMsg					*pMsg )
{
	RsslRet				ret;
	RsslBool			keyOpaque;
	RsslBool			reqKey;
	RsslBool			reqKeyAttrib = RSSL_FALSE;
    RsslBool            extendedHeader;
	RsslUInt16			headerSize;
	RsslEncodingLevel *_levelInfo;

	RSSL_ASSERT(pMsg, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pIter->_pBuffer, Invalid parameters or parameters passed in as NULL);

	if (!((pMsg->msgBase.encDataBody.length == 0) || pMsg->msgBase.encDataBody.data))
		return(RSSL_RET_INVALID_ARGUMENT);

	_levelInfo = &pIter->_levelInfo[++pIter->_encodingLevel]; if (pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	
	/* _initElemStartPos and encoding state should be the only two members used at a msg encoding level. */
	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	_levelInfo->_listType = (void*)pMsg;

	/* header length */
	_levelInfo->_countWritePtr = pIter->_curBufPtr;
	pIter->_curBufPtr += __RSZUI16;

	if ((ret = rsslEncodeMsgInternal(pIter, pMsg, &keyOpaque, &extendedHeader, &reqKey)) < 0)
	{
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
		return(ret);
	}

	/* Keys and or extendedHeaders have to be encoded */
	if( keyOpaque || extendedHeader ) 
	{
		pIter->_curBufPtr = _levelInfo->_initElemStartPos; 
		return RSSL_RET_INCOMPLETE_DATA;
	}

	/* Encode part numbers and posting data */
	_rsslEncodeMsgPartNumPost(pIter, pMsg);

	/* check if the client wants to encode a request msgKey */
	if (reqKey)
	{
		if ((ret = _rsslEncodeMsgReqKey(pIter, pMsg, &reqKeyAttrib )) < 0)
		{
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return(ret);
		}
		/* Keys and or extendedHeaders have to be encoded */
		if( reqKeyAttrib ) 
		{
			pIter->_curBufPtr = _levelInfo->_initElemStartPos; 
			return RSSL_RET_INCOMPLETE_DATA;
		}
	}

	/* Make sure NoData is handled properly */
	if ((pMsg->msgBase.containerType == RSSL_DT_NO_DATA) && (pMsg->msgBase.encDataBody.length > 0))
	{
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return RSSL_RET_UNEXPECTED_ENCODER_CALL;
	}
	
	/* write header size */
	headerSize = (RsslUInt16)(pIter->_curBufPtr - _levelInfo->_countWritePtr - 2);
	rwfPut16(_levelInfo->_countWritePtr, headerSize);

	if (_rsslIteratorOverrun(pIter, pMsg->msgBase.encDataBody.length))
	{
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return RSSL_RET_BUFFER_TOO_SMALL;
	}

	pIter->_curBufPtr =_rsslEncodeCopyData(pIter->_curBufPtr, &pMsg->msgBase.encDataBody);

	--pIter->_encodingLevel;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeMsgInit(RsslEncodeIterator			*pIter,
									RsslMsg						*pMsg,
									RsslUInt32					dataMaxSize )
{
	RsslBool		keyCount;
    RsslBool		extendedHeader;
	RsslBool		reqKey;
	RsslBool		reqKeyAttrib;
	RsslRet			retVal_Check;
	RsslUInt16		headerSize;
	RsslEncodingLevel *_levelInfo;
	
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pMsg, Invalid parameters or parameters passed in as NULL);

	_levelInfo = &pIter->_levelInfo[++pIter->_encodingLevel]; if (pIter->_encodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	
	_levelInfo->_internalMark._sizePtr = 0;
	_levelInfo->_internalMark._sizeBytes = 0;
	_levelInfo->_internalMark2._sizePtr = 0;
	_levelInfo->_internalMark2._sizeBytes = 0;
	/* _initElemStartPos and encoding state should be the only two members used at a msg encoding level. */
	_levelInfo->_initElemStartPos = pIter->_curBufPtr;

	/* Set the message onto the current _levelInfo */
	_levelInfo->_listType = (void*)pMsg;
	_levelInfo->_containerType = RSSL_DT_MSG;

	/* header length */
	_levelInfo->_countWritePtr = pIter->_curBufPtr;
	pIter->_curBufPtr += __RSZUI16;

	if ((retVal_Check = rsslEncodeMsgInternal(pIter, pMsg, &keyCount, &extendedHeader, &reqKey )) < 0)
	{
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		return retVal_Check;
	}

	if (keyCount)
	{
		if (extendedHeader && reqKey)
		{
			_levelInfo->_encodingState = RSSL_EIS_OPAQUE_EXTENDED_HEADER_REQATTRIB;
		}
		else if (extendedHeader)
		{
			_levelInfo->_encodingState = RSSL_EIS_OPAQUE_AND_EXTENDED_HEADER;
		}
		else if (reqKey)
		{
			_levelInfo->_encodingState = RSSL_EIS_OPAQUE_REQATTRIB;
		}
		else
		{
			_levelInfo->_encodingState = RSSL_EIS_OPAQUE;
		}
		return RSSL_RET_ENCODE_MSG_KEY_OPAQUE;
	}
	else if (extendedHeader)
	{
		if (reqKey)
		{
			_levelInfo->_encodingState = RSSL_EIS_EXTENDED_HEADER_REQATTRIB;
		}
		else
		{
			_levelInfo->_encodingState = RSSL_EIS_EXTENDED_HEADER;
		}
		return RSSL_RET_ENCODE_EXTENDED_HEADER;
	}

	/* if we got this far, then any required msgKey attribs and/or extended header were encoded */

	/* Encode the part number */
	_rsslEncodeMsgPartNumPost(pIter, (RsslMsg*)(_levelInfo->_listType));

	/* check if the client wants to encode a request msgKey */
	if (reqKey)
	{
		if ((retVal_Check = _rsslEncodeMsgReqKey(pIter, pMsg, &reqKeyAttrib )) < 0)
		{
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			return retVal_Check;
		}
		/* if the client didnt provide the request key attribs yet, then tell them to encode it now */
		if (reqKeyAttrib)
		{
			_levelInfo->_encodingState = RSSL_EIS_REQATTRIB;
			return RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB;
		}
	}
	/* write header size */
	headerSize = (RsslUInt16)(pIter->_curBufPtr - _levelInfo->_countWritePtr - 2);
	rwfPut16(_levelInfo->_countWritePtr, headerSize);

	/* now store current location so we can check it to ensure user did not put data 
		* when they shouldnt */
	/* count has been filled in already */
	pIter->_levelInfo[pIter->_encodingLevel]._countWritePtr = pIter->_curBufPtr;

	_levelInfo->_encodingState = RSSL_EIS_ENTRIES;

	if (pMsg->msgBase.containerType != RSSL_DT_NO_DATA)
	{
		return RSSL_RET_ENCODE_CONTAINER;
	}
	return RSSL_RET_SUCCESS;
}
															
RSSL_API RsslRet  rsslEncodeMsgComplete(RsslEncodeIterator	*pIter,
										RsslBool			 success )
{
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	RsslMsg* pMsg;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_ENTRIES) ||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);
	
	if (!success)
	{
		/* _initElemStartPos is at the start of the msg at this level. Typically this should be the same as pIter->pBuffer->data. */
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
	}
	else
	{
		pMsg = (RsslMsg*)_levelInfo->_listType;
		if ((pMsg->msgBase.containerType == RSSL_DT_NO_DATA) && 
			(pIter->_levelInfo[pIter->_encodingLevel]._countWritePtr != pIter->_curBufPtr))
		{
			/* user encoded payload when they should not have */
			/* roll back */
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			_levelInfo->_initElemStartPos = 0;
			_levelInfo->_countWritePtr = 0;
			return RSSL_RET_INVALID_DATA;
		}
	}

	--pIter->_encodingLevel;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeMsgKeyAttribComplete(RsslEncodeIterator		*pIter,
												RsslBool				success )
{
	RsslRet ret;
	RsslUInt16 headerSize;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	RsslMsg* pMsg = (RsslMsg*)(_levelInfo->_listType);

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_OPAQUE)								||
				(_levelInfo->_encodingState == RSSL_EIS_OPAQUE_AND_EXTENDED_HEADER)			||
				(_levelInfo->_encodingState == RSSL_EIS_OPAQUE_EXTENDED_HEADER_REQATTRIB)	||
				(_levelInfo->_encodingState == RSSL_EIS_OPAQUE_REQATTRIB)					||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if( success )
	{	
		if (_levelInfo->_internalMark2._sizeBytes > 0)
		{
			if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark2, pIter->_curBufPtr)) < 0)
			{
				/* rollback */
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				pIter->_curBufPtr = _levelInfo->_internalMark2._sizePtr;
				return(ret);
			}
		}
		else
		{
			/* no opaque was encoded - failure */
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return RSSL_RET_FAILURE;
		}

		/* write key length into buffer */
		if (_levelInfo->_internalMark._sizeBytes > 0)
		{
			if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark, pIter->_curBufPtr)) < 0)
			{
				/* roll back */
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				pIter->_curBufPtr = _levelInfo->_internalMark._sizePtr;
				return ret;
			}
		}
		else
		{
			/* no key was attempted to be encoded - failure */
			/* go to start of buffer */
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return RSSL_RET_FAILURE;
		}
		
		/* if they still need to encode extended header do that */
		if (_levelInfo->_encodingState == RSSL_EIS_OPAQUE_AND_EXTENDED_HEADER			||
			_levelInfo->_encodingState == RSSL_EIS_OPAQUE_EXTENDED_HEADER_REQATTRIB)
		{
			RsslBuffer* pExtHdr;
			RSSL_ASSERT(pMsg, Invalid parameters or parameters passed in as NULL);
			/* see if the extended header was pre-encoded */
			/* if we can, write it, if not reserve space */
			
			pExtHdr = (RsslBuffer *)rsslGetExtendedHeader(pMsg);

			if (pExtHdr && pExtHdr->length && pExtHdr->data)
			{
				/* we can encode this here and now */
				if (_rsslIteratorOverrun(pIter, pExtHdr->length + 1))
				{
					/* rollback */
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					pIter->_curBufPtr = _levelInfo->_initElemStartPos;
					return RSSL_RET_BUFFER_TOO_SMALL;
					
				}	
				if (pExtHdr->length > RWF_MAX_8)
				{
					/* roll back */
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					pIter->_curBufPtr = _levelInfo->_initElemStartPos;
					return RSSL_RET_INVALID_DATA;
				}
				pIter->_curBufPtr = _rsslEncodeBuffer8(pIter->_curBufPtr, pExtHdr);
			
				/*add in the part numbers and posting data to the header */
				_rsslEncodeMsgPartNumPost(pIter, (RsslMsg*)(_levelInfo->_listType));

				if (_levelInfo->_encodingState == RSSL_EIS_OPAQUE_EXTENDED_HEADER_REQATTRIB)
				{
					RsslBool reqKeyAttrib;

					if ((ret = _rsslEncodeMsgReqKey(pIter, pMsg, &reqKeyAttrib )) < 0)
					{
						pIter->_curBufPtr = _levelInfo->_initElemStartPos;
						_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
						return ret;
					}
					/* if the client didnt provide the request key attribs yet, then tell them to encode it now */
					if (reqKeyAttrib)
					{
						_levelInfo->_encodingState = RSSL_EIS_REQATTRIB;
						return RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB;
					}
				}
				/* write header size */
				headerSize = (RsslUInt16)(pIter->_curBufPtr - _levelInfo->_countWritePtr - 2);
				rwfPut16(_levelInfo->_countWritePtr, headerSize);
			
				/* now store current location so we can check it to ensure user did not put data 
				 * when they shouldnt */
				/* count has been filled in already */
				pIter->_levelInfo[pIter->_encodingLevel]._countWritePtr = pIter->_curBufPtr;

				_levelInfo->_encodingState = RSSL_EIS_ENTRIES;      
			
				if (pMsg->msgBase.containerType != RSSL_DT_NO_DATA)
				{
					ret = RSSL_RET_ENCODE_CONTAINER;
				}
				else
				{
					ret = RSSL_RET_SUCCESS;
				}
			}
			else
			{
				/* it will be encoded after this, reserve space for size */
				
				if (_rsslIteratorOverrun(pIter, 1))
				{
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					pIter->_curBufPtr = _levelInfo->_initElemStartPos;
					return RSSL_RET_BUFFER_TOO_SMALL;
				}
					
				/* must reserve space now */
				_levelInfo->_internalMark._sizePtr = pIter->_curBufPtr;
				_levelInfo->_internalMark._sizeBytes = 1;
				pIter->_curBufPtr++; // move pointer */

				if (_levelInfo->_encodingState == RSSL_EIS_OPAQUE_AND_EXTENDED_HEADER)
					_levelInfo->_encodingState = RSSL_EIS_EXTENDED_HEADER;
				else
					_levelInfo->_encodingState = RSSL_EIS_EXTENDED_HEADER_REQATTRIB;

				ret = RSSL_RET_ENCODE_EXTENDED_HEADER;
			}
		}
		else
		{
			/* msgKey is done and no extended header is needed */
			_rsslEncodeMsgPartNumPost(pIter, (RsslMsg*)(_levelInfo->_listType));

			/* check if the client wants to encode a request msgKey */
			if (_levelInfo->_encodingState == RSSL_EIS_OPAQUE_REQATTRIB)
			{
				RsslBool reqKeyAttrib;

				if ((ret = _rsslEncodeMsgReqKey(pIter, pMsg, &reqKeyAttrib )) < 0)
				{
					pIter->_curBufPtr = _levelInfo->_initElemStartPos;
					_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
					return ret;
				}

				/* if the client didnt provide the request key attribs yet, then tell them to encode it now */
				if (reqKeyAttrib)
				{
					_levelInfo->_encodingState = RSSL_EIS_REQATTRIB;
					return RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB;
				}
			}
			headerSize = (RsslUInt16)(pIter->_curBufPtr - _levelInfo->_countWritePtr - 2);
			rwfPut16(_levelInfo->_countWritePtr, headerSize);

			/* now store current location so we can check it to ensure user did not put data 
			 * when they shouldnt */
			/* count has been filled in already */
			pIter->_levelInfo[pIter->_encodingLevel]._countWritePtr = pIter->_curBufPtr;
			
			_levelInfo->_encodingState = RSSL_EIS_ENTRIES; 

			if (pMsg->msgBase.containerType != RSSL_DT_NO_DATA)
			{
				ret = RSSL_RET_ENCODE_CONTAINER;
			}
			else
			{
				ret = RSSL_RET_SUCCESS;
			}
		}
	}
	else
	{	
		/* roll back and change state */
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;

		ret = RSSL_RET_SUCCESS;
	}
	
	return ret;
}

RSSL_API RsslRet rsslEncodeExtendedHeaderComplete( RsslEncodeIterator	*pIter,
													RsslBool			success )
{
	RsslUInt8 size;
	RsslUInt16 headerSize;
	RsslRet ret;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	RsslMsg* pMsg = (RsslMsg*)(_levelInfo->_listType);

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_EXTENDED_HEADER)	||
				(_levelInfo->_encodingState == RSSL_EIS_EXTENDED_HEADER_REQATTRIB), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if( success )
	{	
		if (_levelInfo->_internalMark._sizeBytes > 0)
		{
			/* write one byte length onto wire */
			size = (RsslUInt8)((pIter->_curBufPtr - _levelInfo->_internalMark._sizePtr - _levelInfo->_internalMark._sizeBytes));
			rwfPut8(_levelInfo->_internalMark._sizePtr, size);
		}
		else
		{
			/* extended header shouldnt have been encoded */
			/* roll back and change state */
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
		}

		/* Extended Header is finished, encode the end of the header */
		_rsslEncodeMsgPartNumPost(pIter, (RsslMsg*)(_levelInfo->_listType));
		if (_levelInfo->_encodingState == RSSL_EIS_EXTENDED_HEADER_REQATTRIB)
		{
			RsslBool reqKeyAttrib;

			if ((ret = _rsslEncodeMsgReqKey(pIter, pMsg, &reqKeyAttrib )) < 0)
			{
				pIter->_curBufPtr = _levelInfo->_initElemStartPos;
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				return ret;
			}
			/* if the client didnt provide the request key attribs yet, then tell them to encode it now */
			if (reqKeyAttrib)
			{
				_levelInfo->_encodingState = RSSL_EIS_REQATTRIB;
				return RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB;
			}
		}

		/* write header size */
		headerSize = (RsslUInt16)(pIter->_curBufPtr - _levelInfo->_countWritePtr - 2);
		rwfPut16(_levelInfo->_countWritePtr, headerSize);

		/* now store current location so we can check it to ensure user did not put data 
		 * when they shouldnt */
		/* count has been filled in already */
		pIter->_levelInfo[pIter->_encodingLevel]._countWritePtr = pIter->_curBufPtr;

		/* should be ready to encode entries - dont save length for data */
		_levelInfo->_encodingState = RSSL_EIS_ENTRIES;  

		if (pMsg->msgBase.containerType != RSSL_DT_NO_DATA)
		{
			return RSSL_RET_ENCODE_CONTAINER;
		}
		else
		{
			return RSSL_RET_SUCCESS;
		}		
	}
	else
	{	
		/* roll back and change state */
		_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
		pIter->_curBufPtr = _levelInfo->_initElemStartPos;
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeMsgReqKeyAttribComplete(RsslEncodeIterator		*pIter,
													RsslBool				success )
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslUInt16 headerSize;
	RsslEncodingLevel *_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];
	RsslMsg* pMsg = (RsslMsg*)(_levelInfo->_listType);

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT((_levelInfo->_encodingState == RSSL_EIS_REQATTRIB)		||
				(_levelInfo->_encodingState == RSSL_EIS_WAIT_COMPLETE), Unexpected encoding attempted);
	RSSL_ASSERT(pIter->_pBuffer && pIter->_pBuffer->data, Invalid iterator use - check buffer);
	RSSL_ASSERT(pIter->_curBufPtr <= pIter->_endBufPtr, Data exceeds iterators buffer length);

	if( success )
	{	
		if (_levelInfo->_internalMark2._sizeBytes > 0)
		{
			if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark2, pIter->_curBufPtr)) < 0)
			{
				/* rollback */
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				pIter->_curBufPtr = _levelInfo->_internalMark2._sizePtr;
				return(ret);
			}
		}
		else
		{
			/* no attribs were encoded - failure */
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return RSSL_RET_FAILURE;
		}

		/* write key length into buffer */
		if (_levelInfo->_internalMark._sizeBytes > 0)
		{
			if ((ret = _rsslFinishU15Mark(&_levelInfo->_internalMark, pIter->_curBufPtr)) < 0)
			{
				/* roll back */
				_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
				pIter->_curBufPtr = _levelInfo->_internalMark._sizePtr;
				return ret;
			}
		}
		else
		{
			/* no key was attempted to be encoded - failure */
			/* go to start of buffer */
			_levelInfo->_encodingState = RSSL_EIS_WAIT_COMPLETE;
			pIter->_curBufPtr = _levelInfo->_initElemStartPos;
			return RSSL_RET_FAILURE;
		}

		/* header is done */
		headerSize = (RsslUInt16)(pIter->_curBufPtr - _levelInfo->_countWritePtr - 2);
		rwfPut16(_levelInfo->_countWritePtr, headerSize);

		/* now store current location so we can check it to ensure user did not put data when they shouldnt */
		/* count has been filled in already */
		pIter->_levelInfo[pIter->_encodingLevel]._countWritePtr = pIter->_curBufPtr;
			
		_levelInfo->_encodingState = RSSL_EIS_ENTRIES; 

		if (pMsg->msgBase.containerType != RSSL_DT_NO_DATA)
		{
			ret = RSSL_RET_ENCODE_CONTAINER;
		}
		else
		{
			ret = RSSL_RET_SUCCESS;
		}
	}
	return ret;
}

/* TO DO Figure out duplicate definitions in messagesDecoders.c */
#define _RSSL_MSG_CLASS_POS                 2
#define _RSSL_MSG_TYPE_POS                  3
#define _RSSL_MSG_STREAMID_POS		        4
#define _RSSL_MSG_BASE_END_POS				8 /* End of msgBase parameters */


RSSL_API RsslRet rsslReplaceDomainType(
                     RsslEncodeIterator		*pIter,
                     RsslUInt8               domainType )
{
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pEncodedMessageBuffer->data, Invalid parameters or parameters passed in as NULL);

	if (pEncodedMessageBuffer->length < _RSSL_MSG_TYPE_POS + __RSZUI8)
		return RSSL_RET_INVALID_ARGUMENT;

    /* Store domainType as UInt8 */
	rwfPut8( (pEncodedMessageBuffer->data + _RSSL_MSG_TYPE_POS), domainType );

    return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslReplaceStreamId(
                     RsslEncodeIterator		*pIter,
                     RsslInt32             streamId )
{
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;
	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pEncodedMessageBuffer->data, Invalid parameters or parameters passed in as NULL);

	if (pEncodedMessageBuffer->length < _RSSL_MSG_STREAMID_POS + __RSZUI32)
		return RSSL_RET_INVALID_ARGUMENT;

	/* Store streamId as Int32 */
	rwfPut32( (pEncodedMessageBuffer->data + _RSSL_MSG_STREAMID_POS), streamId );

    return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet		_rsslStatePos(
					char **					pos,	
					RsslUInt8 *				msgClass,                  
                    RsslUInt16 *			mFlags,
					RsslBuffer *			pEncodedMessageBuffer )
{
	char * position;

	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pEncodedMessageBuffer->data, Invalid parameters or parameters passed in as NULL);

	if (pEncodedMessageBuffer->length < 10)
		return RSSL_RET_INVALID_ARGUMENT;

	position = pEncodedMessageBuffer->data;
	position += _RSSL_MSG_CLASS_POS;  /* for header size */
	position += RWF_MOVE_8(msgClass, position);

	position++; // domainType
	position += 4; // streamId
	position += rwfGetResBitU15(mFlags, position);

	switch(*msgClass)
	{
		case RSSL_MC_REFRESH:
			position++; // containerType
			if (*mFlags & RSSL_RFMF_HAS_SEQ_NUM)
				position += 4;  // seqNo
			break;
        case RSSL_MC_STATUS:
			position++; // containerType
			if (!(*mFlags & RSSL_STMF_HAS_STATE))
				return RSSL_RET_INVALID_DATA;
			break;
		case RSSL_MC_UPDATE:
		case RSSL_MC_GENERIC:
		case RSSL_MC_POST:
		case RSSL_MC_REQUEST:
		case RSSL_MC_CLOSE:
		case RSSL_MC_ACK:
		default:
			return RSSL_RET_INVALID_ARGUMENT;
	}

	*pos = position;
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslReplaceStreamState(
                     RsslEncodeIterator		*pIter,
                     RsslUInt8 streamState )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslUInt8	state;
	RsslUInt8	curState = 0;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (streamState == RSSL_STREAM_UNSPECIFIED)	
		return RSSL_RET_INVALID_DATA;

	if (_rsslStatePos(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;
	
	switch(msgClass)
	{
		case RSSL_MC_REFRESH:
        case RSSL_MC_STATUS:
			rwfGet8(curState, position);

			state = (streamState << 3)	;
			state |= (curState & 0x7);

			rwfPut8(position, state);
			break;
		case RSSL_MC_UPDATE:
		case RSSL_MC_GENERIC:
		case RSSL_MC_POST:
		case RSSL_MC_REQUEST:
		case RSSL_MC_CLOSE:
		case RSSL_MC_ACK:
		default:
			return RSSL_RET_INVALID_ARGUMENT;
	}
	return RSSL_RET_SUCCESS;
}


RSSL_API RsslRet rsslReplaceDataState(
                     RsslEncodeIterator		*pIter,
                     RsslUInt8 dataState )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslUInt8	state;
	RsslUInt8	curState = 0;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslStatePos(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;
	
	switch(msgClass)
	{
		case RSSL_MC_REFRESH:
        case RSSL_MC_STATUS:
			rwfGet8(curState, position);

			state = dataState;
			state |= (curState & 0xF8);

			rwfPut8(position, state);
			break;
		case RSSL_MC_UPDATE:
		case RSSL_MC_GENERIC:
		case RSSL_MC_POST:
		case RSSL_MC_REQUEST:
		case RSSL_MC_CLOSE:
		case RSSL_MC_ACK:
		default:
			return RSSL_RET_INVALID_ARGUMENT;
	}
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslReplacePostId(
                     RsslEncodeIterator		*pIter,
                     RsslUInt32			postId )
{
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;
	char *position = pEncodedMessageBuffer->data;
	RsslUInt8 msgClass;
	RsslUInt16 flags;

	RSSL_CHECKSIZE(
			_RSSL_MSG_BASE_END_POS 
			+ 1 /* at least 1 for flags(U15) */ 
			+ 1 /* containerType */
			+ 8 /* address & userID */
			+ sizeof(postId)
			, pEncodedMessageBuffer->length);

	/* Check that message class is post msg */
	rwfGet8(msgClass, position + _RSSL_MSG_CLASS_POS);
	if (msgClass != RSSL_MC_POST) 
		return RSSL_RET_INVALID_ARGUMENT;

	/* flags */
	position += _RSSL_MSG_BASE_END_POS;
	position += rwfGetResBitU15(&flags, position);
	if (!(flags & RSSL_PSMF_HAS_POST_ID))
		return RSSL_RET_FAILURE;

	position += 1 /* containerType */ + 8 /* address & userID */;

	if (flags & RSSL_PSMF_HAS_SEQ_NUM)
		position += 4;

	RSSL_CHECKOFFBUFFER(position + 4, *pEncodedMessageBuffer);
	rwfPut32(position, postId);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslReplaceStateCode(
                     RsslEncodeIterator		*pIter,
                     RsslUInt8 stateCode )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslStatePos(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;
	
	switch(msgClass)
	{
		case RSSL_MC_REFRESH:
        case RSSL_MC_STATUS:
			position++; // state
			rwfPut8( position, stateCode );
			return RSSL_RET_SUCCESS;
		case RSSL_MC_UPDATE:
		case RSSL_MC_GENERIC:
		case RSSL_MC_POST:
		case RSSL_MC_REQUEST:
		case RSSL_MC_CLOSE:
		case RSSL_MC_ACK:
		default:
			return RSSL_RET_INVALID_ARGUMENT;
	}
}

RSSL_API RsslRet rsslReplaceGroupId(
                     RsslEncodeIterator		*pIter,
                     RsslBuffer 			groupId )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	RsslUInt8 groupLen = 0;
	RsslUInt16 stateTextLen = 0;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

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

	/* we are at the group id */
	rwfGet8(groupLen, position);

	/* this can only work for the same length group id being put in */
	if (groupLen != groupId.length)
		return RSSL_RET_INVALID_ARGUMENT;

	_rsslEncodeBuffer8(position, &groupId);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslReplaceSeqNum(
                     RsslEncodeIterator		*pIter,
                     RsslUInt32 			seqNum )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	RsslBuffer tempBuf;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

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
			position += 5; // domainType and streamId
			position += rwfGetResBitU15(&mFlags, position);

			if (!(mFlags & RSSL_UPMF_HAS_SEQ_NUM))
				return RSSL_RET_FAILURE;

			position += 2; //move past containerType and updateType
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
			{
				position += rwfGetBuffer16(&tempBuf, position);
			}
		}
		break;
		default:
			return RSSL_RET_FAILURE;
	}

	/* at correct position, replace seqNum */
	rwfPut32(position, seqNum);

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet	_rsslFlagsHelper(
					char **					pos,	
					RsslUInt8 *				msgClass,                  
                    RsslUInt16 *			mFlags,
					RsslBuffer *			pEncodedMessageBuffer )
{
	char * position;

	RSSL_ASSERT(pEncodedMessageBuffer, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pEncodedMessageBuffer->data, Invalid parameters or parameters passed in as NULL);

	if (pEncodedMessageBuffer->length < 9)
		return RSSL_RET_INVALID_ARGUMENT;

	position = pEncodedMessageBuffer->data;
	position += _RSSL_MSG_CLASS_POS;  /* for header size */
	position += RWF_MOVE_8(msgClass, position);

	position++; // domainType
	position += 4; // streamId
	rwfGetResBitU15(mFlags, position);

	*pos = position;
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslSetStreamingFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REQUEST)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RQMF_STREAMING)
		return RSSL_RET_SUCCESS;

	mFlags |= RSSL_RQMF_STREAMING;

	rwfPutResBitU15(position, mFlags);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslUnsetStreamingFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REQUEST)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RQMF_STREAMING)
	{
		mFlags &= ~RSSL_RQMF_STREAMING;
		rwfPutResBitU15(position, mFlags);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslSetNoRefreshFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REQUEST)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RQMF_NO_REFRESH)
		return RSSL_RET_SUCCESS;

	mFlags |= RSSL_RQMF_NO_REFRESH;

	rwfPutResBitU15(position, mFlags);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslUnsetNoRefreshFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REQUEST)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RQMF_NO_REFRESH)
	{
		mFlags &= ~RSSL_RQMF_NO_REFRESH;
		rwfPutResBitU15(position, mFlags);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslSetMsgKeyInUpdatesFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REQUEST)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RQMF_MSG_KEY_IN_UPDATES)
		return RSSL_RET_SUCCESS;

	mFlags |= RSSL_RQMF_MSG_KEY_IN_UPDATES;
	rwfPutResBitU15(position, mFlags);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslUnsetMsgKeyInUpdatesFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REQUEST)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RQMF_MSG_KEY_IN_UPDATES)
	{
		mFlags &= ~RSSL_RQMF_MSG_KEY_IN_UPDATES;
		rwfPutResBitU15(position, mFlags);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslSetConfInfoInUpdatesFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REQUEST)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RQMF_CONF_INFO_IN_UPDATES)
		return RSSL_RET_SUCCESS;

	mFlags |= RSSL_RQMF_CONF_INFO_IN_UPDATES;

	rwfPutResBitU15(position, mFlags);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslUnsetConfInfoInUpdatesFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REQUEST)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RQMF_CONF_INFO_IN_UPDATES)
	{
		mFlags &= ~RSSL_RQMF_CONF_INFO_IN_UPDATES;
		rwfPutResBitU15(position, mFlags);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslSetSolicitedFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REFRESH)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RFMF_SOLICITED)
		return RSSL_RET_SUCCESS;

	mFlags |= RSSL_RFMF_SOLICITED;

	rwfPutResBitU15(position, mFlags);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslUnsetSolicitedFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REFRESH)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RFMF_SOLICITED)
	{
		mFlags &= ~RSSL_RFMF_SOLICITED;
		rwfPutResBitU15(position, mFlags);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslSetRefreshCompleteFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REFRESH)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RFMF_REFRESH_COMPLETE)
		return RSSL_RET_SUCCESS;

	mFlags |= RSSL_RFMF_REFRESH_COMPLETE;

	rwfPutResBitU15(position, mFlags);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslUnsetRefreshCompleteFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_REFRESH)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_RFMF_REFRESH_COMPLETE)
	{
		mFlags &= ~RSSL_RFMF_REFRESH_COMPLETE;
		rwfPutResBitU15(position, mFlags);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslSetGenericCompleteFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_GENERIC)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_GNMF_MESSAGE_COMPLETE)
		return RSSL_RET_SUCCESS;

	mFlags |= RSSL_GNMF_MESSAGE_COMPLETE;

	rwfPutResBitU15(position, mFlags);

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslUnsetGenericCompleteFlag( RsslEncodeIterator		*pIter )
{
	RsslUInt8 msgClass;
	RsslUInt16 mFlags;
	char * position;
	RsslBuffer *pEncodedMessageBuffer = pIter->_pBuffer;

	if (_rsslFlagsHelper(&position, &msgClass, &mFlags, pEncodedMessageBuffer) < 0)
		return RSSL_RET_FAILURE;

	if (msgClass != RSSL_MC_GENERIC)
		return RSSL_RET_FAILURE;

	if (mFlags & RSSL_GNMF_MESSAGE_COMPLETE)
	{
		mFlags &= ~RSSL_GNMF_MESSAGE_COMPLETE;
		rwfPutResBitU15(position, mFlags);
	}

	return RSSL_RET_SUCCESS;
}


