/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


#include <limits.h>

#ifdef Linux
#include <stdlib.h>
#endif

#include "rtr/rtratoi.h"

#include "rtr/jsonToRwfConverter.h"

jsonToRwfConverter::jsonToRwfConverter(int bufSize, unsigned int flags, int numTokens, int incSize) :
	jsonToRwfBase(bufSize, flags, numTokens, incSize)
{
	memset(_fieldSetDefDbs, 0, RSSL_ITER_MAX_LEVELS * sizeof(RsslLocalFieldSetDefDb*));
	memset(_elementSetDefDbs, 0, RSSL_ITER_MAX_LEVELS * sizeof(RsslLocalElementSetDefDb*));
}

jsonToRwfConverter::~jsonToRwfConverter()
{
	// Cleanup any memory allocated for set def's.

	for (int i=0; i < RSSL_ITER_MAX_LEVELS ; i++)
	{
		if (_fieldSetDefDbs[i])
		{
			delete[] (RsslFieldSetDefEntry*)(_fieldSetDefDbs[i]->entries.data);
			delete _fieldSetDefDbs[i];
		}
		if (_elementSetDefDbs[i])
		{
			delete[] (RsslElementSetDefEntry*)(_elementSetDefDbs[i]->entries.data);
			delete _elementSetDefDbs[i];
		}
	}
}

bool jsonToRwfConverter::processMessage(jsmntok_t ** const msgTok, RsslJsonMsg *jsonMsgPtr)
{
	jsmntok_t *msgBaseTok = 0;
	jsmntok_t *tok = *msgTok;
	bool foundStart = false;

	jsmntok_t *dataTokPtr = 0;
	jsmntok_t *attribTokPtr = 0;
	jsmntok_t *reqKeyattrib = 0;

	/* In standard JSON, all messages should be RWF-converted messages. */
	RsslMsg *rsslMsgPtr = &jsonMsgPtr->jsonRsslMsg.rsslMsg;
	jsonMsgPtr->msgBase.msgClass = RSSL_JSON_MC_RSSL_MSG;

	tok++;
	while ( tok < _tokensEndPtr)
	{
		if (_jsonMsg[tok->start] == 'b')
		{
			foundStart = true;
			break;
		}
		tok++;
	}

	//Not able to find 'b' in the message
	if (!foundStart)
	{
		error(NO_MSG_BASE, __LINE__, __FILE__);
		return false;
	}

	if (tok->type == JSMN_STRING &&
		_jsonMsg[tok->start] == 'b')
	{
		tok++;
		msgBaseTok = tok;

		if (tok->type == JSMN_OBJECT)
		{
			tok++;
			while( tok < _tokensEndPtr && tok->end < msgBaseTok->end)
			{
				if (tok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, tok, __LINE__, __FILE__);
					return false;
				}

				switch (_jsonMsg[tok->start])
				{
				case 's':
				{
					tok++;
					if (tok->type == JSMN_PRIMITIVE)
					rsslMsgPtr->msgBase.streamId =
							rtr_atoi_size( &_jsonMsg[tok->start],
								       &_jsonMsg[tok->end]);
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
				case 't':
				{
					tok++;
					if (tok->type == JSMN_PRIMITIVE)
						rsslMsgPtr->msgBase.domainType =
							rtr_atoi_size( &_jsonMsg[tok->start],
								       &_jsonMsg[tok->end]);
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
				case 'c':
				{
					tok++;
					if (tok->type == JSMN_PRIMITIVE)
						rsslMsgPtr->msgBase.msgClass =
							rtr_atoi_size( &_jsonMsg[tok->start],
								       &_jsonMsg[tok->end]);
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
				case 'f':
				{
					tok++;
					if (tok->type == JSMN_PRIMITIVE)
						rsslMsgPtr->msgBase.containerType =
							rtr_atoi_size( &_jsonMsg[tok->start],
								       &_jsonMsg[tok->end]) + RSSL_DT_CONTAINER_TYPE_MIN;
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
				default:
					unexpectedParameter(tok, __LINE__, __FILE__);
					return false;
				}
				tok++;
			}
		}

		return encodeRsslMsg(rsslMsgPtr, msgTok, dataTokPtr,attribTokPtr, reqKeyattrib);
	}
	return false;
}

bool jsonToRwfConverter::processJson(jsmntok_t ** const tokPtr, void* setDb)
{
	error(UNSUPPORTED_DATA_TYPE, __LINE__, __FILE__);
	return false;
}

bool jsonToRwfConverter::processRequestMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	int ret = 0;
	int i = 0;
	int size = 0;
	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	// Assume streaming (default)
	rsslRequestMsgApplyStreaming(&rsslMsgPtr->requestMsg);

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	(*tokPtr)++;

	while ((*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < msgTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'b':
		{
		  	// Msg Base (just skip)
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_OBJECT)
				skipObject(tokPtr);
			else
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'g':
		{
			// Batch
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)	// Batch
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslRequestMsgApplyHasBatch(&rsslMsgPtr->requestMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'k':
		{
			// Msg Key
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, attribPtr) == false)
				return false;
			break;
		}
		case 's': // Streaming
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '0')
					rsslMsgPtr->requestMsg.flags &= ~RSSL_RQMF_STREAMING;
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			// Need to wait until the end to encode payload
			// Save the data start token index and move past
			*dataPtr = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'e':	// Extended Header
		{
			(*tokPtr)++;
			bufPtr = &rsslMsgPtr->requestMsg.extendedHeader;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;

			rsslRequestMsgApplyHasExtendedHdr(&rsslMsgPtr->requestMsg);
			break;
		}
		case 'i':	// Include Key
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')

					rsslRequestMsgApplyMsgKeyInUpdates(&rsslMsgPtr->requestMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'n':	// No Refresh
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslRequestMsgApplyNoRefresh(&rsslMsgPtr->requestMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'v':	// View
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslRequestMsgApplyHasView(&rsslMsgPtr->requestMsg);
					(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'h':	// Pause
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslRequestMsgApplyPause(&rsslMsgPtr->requestMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'u':	// Private Stream
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslRequestMsgApplyPrivateStream(&rsslMsgPtr->requestMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'c':	// Include Conflation Info
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslRequestMsgApplyConfInfoInUpdates(&rsslMsgPtr->requestMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'o':	// Qualified Stream
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslRequestMsgApplyQualifiedStream(&rsslMsgPtr->requestMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'q':	// Best QOS
		{
			(*tokPtr)++;
			if (populateQos(tokPtr, &rsslMsgPtr->requestMsg.qos) == false)
				return false;
			rsslRequestMsgApplyHasQos(&rsslMsgPtr->requestMsg);
			break;
		}
		case 'w':	// Worst QOS
		{
			(*tokPtr)++;
			if (populateQos(tokPtr, &rsslMsgPtr->requestMsg.worstQos) == false)
				return false;
			rsslRequestMsgApplyHasWorstQos(&rsslMsgPtr->requestMsg);
			break;
		}
		case 'p':	// Priority
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			size = (*tokPtr)->size;
			(*tokPtr)++;

			for (i = 0; i < size; i+=2)
			{
				if ((*tokPtr)->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				switch (_jsonMsg[(*tokPtr)->start])
				{
				case 'c':	// Class
				{
					(*tokPtr)++;
					rsslMsgPtr->requestMsg.priorityClass =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
							       &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
					break;
				}
				case 'n':	// Count
				{
					(*tokPtr)++;
					rsslMsgPtr->requestMsg.priorityCount =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
							       &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
					break;
				}
				default:
				{
					unexpectedParameter(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				}
			}
			rsslRequestMsgApplyHasPriority(&rsslMsgPtr->requestMsg);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	if (!rsslRequestMsgCheckHasQos(&rsslMsgPtr->requestMsg) && _flags & JSON_FLAG_DEFAULT_QOS)
	{
		// No QOS was provided, set to dynamic if configured
		rsslMsgPtr->requestMsg.qos.dynamic = 1;
		rsslMsgPtr->requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslMsgPtr->requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslRequestMsgApplyHasQos(&rsslMsgPtr->requestMsg);

		rsslMsgPtr->requestMsg.worstQos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		rsslMsgPtr->requestMsg.worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		rsslMsgPtr->requestMsg.worstQos.rateInfo = 65535;
		rsslRequestMsgApplyHasWorstQos(&rsslMsgPtr->requestMsg);

		// No QOS was provided, set to dynamic if configured
		rsslMsgPtr->requestMsg.qos.dynamic = 1;
		rsslMsgPtr->requestMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		rsslMsgPtr->requestMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		rsslRequestMsgApplyHasQos(&rsslMsgPtr->requestMsg);
	}
	return true;
}

bool jsonToRwfConverter::processRefreshMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	int ret = 0;

	// Default is solicited and compelte
	rsslMsgPtr->refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;


	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	(*tokPtr)++;

	while ((*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < msgTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'b':
		{
			// Msg Base (just skip)
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_OBJECT)
				skipObject(tokPtr);
			else
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 's':
		{
			(*tokPtr)++;
			if (populateState(tokPtr, &rsslMsgPtr->refreshMsg.state) == false)
				return false;
			break;
		}
		case 'g':	// Group Id
		{
			(*tokPtr)++;
			 bufPtr = &rsslMsgPtr->refreshMsg.groupId;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			// Need to wait until the end to encode payload
			// Save the data start token index and move past
			*dataPtr = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'q':	// QOS
		{
			(*tokPtr)++;
			rsslRefreshMsgApplyHasQos(&rsslMsgPtr->refreshMsg);
			if (populateQos(tokPtr, &rsslMsgPtr->refreshMsg.qos) == false)
				return false;
			break;
		}
		case 'u':	// Private Stream
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslRefreshMsgApplyPrivateStream(&rsslMsgPtr->refreshMsg);
			(*tokPtr)++;
			break;
		}
		case 'o':	// Qualified Stream
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslRefreshMsgApplyQualifiedStream(&rsslMsgPtr->refreshMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 't':	// Trash Cache
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslRefreshMsgApplyClearCache(&rsslMsgPtr->refreshMsg);
			(*tokPtr)++;
			break;
		}
		case 'x':	// Do Not Cache
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslRefreshMsgApplyDoNotCache(&rsslMsgPtr->refreshMsg);
			(*tokPtr)++;
			break;
		}
		case 'c':	// Refresh Complete
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '0')
				rsslMsgPtr->refreshMsg.flags &= ~RSSL_RFMF_REFRESH_COMPLETE;
			(*tokPtr)++;
			break;
		}
		case 'a':	// Solicited
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '0')
				rsslMsgPtr->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;
			(*tokPtr)++;
			break;
		}
		case 'k':	// Key
		{
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, attribPtr) == false)
				return false;
			rsslRefreshMsgApplyHasMsgKey(&rsslMsgPtr->refreshMsg);
			break;
		}
		case 'r':	// Request Key
		{
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->refreshMsg.reqMsgKey, reqKeyattrib) == false)
				return false;
			rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_HAS_REQ_MSG_KEY;

			break;
		}
		case 'n':	// Sequence Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->refreshMsg.seqNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								       &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslRefreshMsgApplyHasSeqNum(&rsslMsgPtr->refreshMsg);
			break;
		}
		case 'm':	// Part Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->refreshMsg.partNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									&_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslRefreshMsgApplyHasPartNum(&rsslMsgPtr->refreshMsg);
			break;
		}
		case 'p':	// Permission Data
		{
			(*tokPtr)++;
			 bufPtr = &rsslMsgPtr->refreshMsg.permData;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslRefreshMsgApplyHasPermData(&rsslMsgPtr->refreshMsg);
			break;
		}
		case 'i':	// Post User Info
		{
			(*tokPtr)++;
			if (populatePostUserInfo(tokPtr,&rsslMsgPtr->refreshMsg.postUserInfo) == false)
				return false;
			rsslRefreshMsgApplyHasPostUserInfo(&rsslMsgPtr->refreshMsg);
			break;
		}
		case 'e':	// Extended Hdr
		{
			(*tokPtr)++;
			 bufPtr = &rsslMsgPtr->refreshMsg.extendedHeader;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslRefreshMsgApplyHasExtendedHdr(&rsslMsgPtr->refreshMsg);
			break;
		}
		default:
		{
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
		}
	}
	return true;
}

bool jsonToRwfConverter::processStatusMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	int ret = 0;
	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}
	(*tokPtr)++;

	while ((*tokPtr) < _tokensEndPtr && 
		   (*tokPtr)->end < msgTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'b':
		{
			// Msg Base (just skip)
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_OBJECT)
				skipObject(tokPtr);
			else
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 's':	// State
		{
			(*tokPtr)++;
			if (populateState(tokPtr, &rsslMsgPtr->statusMsg.state) == false)
				return false;
			rsslStatusMsgApplyHasState(&rsslMsgPtr->statusMsg);
			break;
		}
		case 'u':	// Private Stream
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslStatusMsgApplyPrivateStream(&rsslMsgPtr->statusMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;

		}
		case 'o':	// Qualified Stream
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslStatusMsgApplyQualifiedStream(&rsslMsgPtr->statusMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 't':	// Trash Cache
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslStatusMsgApplyClearCache(&rsslMsgPtr->statusMsg);
			(*tokPtr)++;
			break;
		}
		case 'g':	// Group Id
		{
			(*tokPtr)++;
			 bufPtr = &rsslMsgPtr->statusMsg.groupId;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslStatusMsgApplyHasGroupId(&rsslMsgPtr->statusMsg);
			break;
		}
		case 'k':	// Key
		{
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, attribPtr) == false)
				return false;
			rsslStatusMsgApplyHasMsgKey(&rsslMsgPtr->statusMsg);
			break;
		}
		case 'r':	// Request Key
		{
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->statusMsg.reqMsgKey, reqKeyattrib) == false)
				return false;
			rsslMsgPtr->statusMsg.flags |= RSSL_STMF_HAS_REQ_MSG_KEY;
			break;
		}
		case 'i':	// Post User Info
		{
			(*tokPtr)++;
			if (populatePostUserInfo(tokPtr,&rsslMsgPtr->statusMsg.postUserInfo) == false)
				return false;
			rsslStatusMsgApplyHasPostUserInfo(&rsslMsgPtr->statusMsg);
			break;
		}
		case 'p':	// Permission Data
		{
			(*tokPtr)++;
			 bufPtr = &rsslMsgPtr->statusMsg.permData;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslStatusMsgApplyHasPermData(&rsslMsgPtr->statusMsg);
			break;
		}
		case 'e':	// Extended Header
		{
			(*tokPtr)++;
			 bufPtr = &rsslMsgPtr->statusMsg.extendedHeader;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslStatusMsgApplyHasExtendedHdr(&rsslMsgPtr->statusMsg);
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			// Need to wait until the end to encode payload
			// Save the data start token index and move past
			*dataPtr = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

bool jsonToRwfConverter::processUpdateMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	int ret = 0;
	RsslBuffer *bufPtr;
	jsmntok_t *msgTok = *tokPtr;
	jsmntok_t *confTok = 0;

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	(*tokPtr)++;

	while ((*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < msgTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'b':
		{
			// Msg Base (just skip)
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				skipObject(tokPtr);
			}
			else
				return false;
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			// Need to wait until the end to encode payload
			// Save the data start token index and move past
			*dataPtr = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'k':	// Key
		{
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, attribPtr) == false)
				return false;
			rsslUpdateMsgApplyHasMsgKey(&rsslMsgPtr->updateMsg);
			break;
		}

		case 'u':	// Update Type
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->updateMsg.updateType = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									  &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			break;
		}
		case 'f':	// Do Not Conflate
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] != '0')
				rsslUpdateMsgApplyDoNotConflate(&rsslMsgPtr->updateMsg);
			(*tokPtr)++;
			break;
		}
		case 'r':	// Do Not Ripple
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] != '0')
				rsslUpdateMsgApplyDoNotRipple(&rsslMsgPtr->updateMsg);
			(*tokPtr)++;
			break;
		}
		case 'x':	// Do Not Cache
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] != '0')
				rsslUpdateMsgApplyDoNotCache(&rsslMsgPtr->updateMsg);
			(*tokPtr)++;
			break;
		}
		case 'p':	// Permission Data
		{
			(*tokPtr)++;
			bufPtr = &rsslMsgPtr->updateMsg.permData;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslUpdateMsgApplyHasPermData(&rsslMsgPtr->updateMsg);
			break;
		}
		case 'n':	// Sequence Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->updateMsg.seqNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								      &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslUpdateMsgApplyHasSeqNum(&rsslMsgPtr->updateMsg);
			break;
		}
		case 'c':	// Conflation Info
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			confTok = *tokPtr;
			(*tokPtr)++;

			while ((*tokPtr) < _tokensEndPtr &&
			       (*tokPtr)->end < confTok->end)
			{
				if ((*tokPtr)->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				switch (_jsonMsg[(*tokPtr)->start])
				{
				case 'c':	// Conflation Count
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
						return false;
					}
					rsslMsgPtr->updateMsg.conflationCount = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
											       &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
					break;

				}
				case 't':	// Conflation Time
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
						return false;
					}
					rsslMsgPtr->updateMsg.conflationTime = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
											      &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
					break;
				}
				default:
					unexpectedParameter(*tokPtr, __LINE__, __FILE__);
					return false;
				}
			}
			rsslUpdateMsgApplyHasConfInfo(&rsslMsgPtr->updateMsg);
			break;
		}
		case 'o':	// Discardable
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] != '0')
				rsslUpdateMsgApplyDiscardable(&rsslMsgPtr->updateMsg);
			(*tokPtr)++;
			break;
		}
		case 'i':	// Post User Info
		{
			(*tokPtr)++;
			if (populatePostUserInfo(tokPtr,&rsslMsgPtr->updateMsg.postUserInfo) == false)
				return false;
			rsslUpdateMsgApplyHasPostUserInfo(&rsslMsgPtr->updateMsg);
			break;
		}
		case 'e':	// Extended Header
		{
			(*tokPtr)++;
			bufPtr = &rsslMsgPtr->updateMsg.extendedHeader;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslUpdateMsgApplyHasExtendedHdr(&rsslMsgPtr->updateMsg);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

bool jsonToRwfConverter::processCloseMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	int ret = 0;
	RsslBuffer *bufPtr;

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	jsmntok_t *msgTok = *tokPtr;

	(*tokPtr)++;

	while ((*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < msgTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
			case 'b':
			{
				// Msg Base (just skip)
				(*tokPtr)++;
				if ((*tokPtr)->type == JSMN_OBJECT)
					skipObject(tokPtr);
				else
				{
					unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				break;
			}
			case 'a':
			{
				(*tokPtr)++;
				if ((*tokPtr)->type == JSMN_PRIMITIVE)
				{
					if (_jsonMsg[(*tokPtr)->start] == '1')
					  rsslCloseMsgSetAck(&rsslMsgPtr->closeMsg);
					(*tokPtr)++;
				}
				else
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				break;
			}
			case 'e':	// Extended Header
			{
				(*tokPtr)++;
				bufPtr = &rsslMsgPtr->closeMsg.extendedHeader;
				if (processBuffer(tokPtr, &bufPtr, 0) == false)
					return false;
				rsslCloseMsgSetHasExtendedHdr(&rsslMsgPtr->closeMsg);
				break;
			}
			case 'g':	// Batch Close
			{
				// Batch
				(*tokPtr)++;
				if ((*tokPtr)->type == JSMN_PRIMITIVE)	// Batch
				{
					if (_jsonMsg[(*tokPtr)->start] == '1')
						rsslCloseMsgSetHasBatch(&rsslMsgPtr->closeMsg);
					(*tokPtr)++;
				}
				else
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				break;
			}
			case 'd':	// Data
			{
				(*tokPtr)++;
				// Need to wait until the end to encode payload
				// Save the data start token index and move past
				*dataPtr = *tokPtr;
				if ((*tokPtr)->type != JSMN_OBJECT)
				{
					unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				skipObject(tokPtr);
				break;
			}
			default:
			{
				unexpectedParameter(*tokPtr, __LINE__, __FILE__);
				return false;
			}
		}
	}

	return true;
}

bool jsonToRwfConverter::processAckMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	int ret = 0;

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	(*tokPtr)++;

	while ((*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < msgTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'b':
		{
			// Msg Base (just skip)
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_OBJECT)
				skipObject(tokPtr);
			else
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'i':
		{
			(*tokPtr)++;
			rsslMsgPtr->ackMsg.ackId = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								  &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			break;
		}
		case 'u':	// Private Stream
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslAckMsgApplyPrivateStream(&rsslMsgPtr->ackMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;

		}
		case 'k':	// Key
		{
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, attribPtr) == false)
				return false;
			rsslAckMsgApplyHasMsgKey(&rsslMsgPtr->ackMsg);
			break;
		}
		case 'n':	// Sequence Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->ackMsg.seqNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								   &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslAckMsgApplyHasSeqNum(&rsslMsgPtr->ackMsg);
			break;
		}
		case 'c':	// Nak Code
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->ackMsg.nakCode = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								    &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslAckMsgApplyHasNakCode(&rsslMsgPtr->ackMsg);
			break;
		}
		case 't':	// Nak Text
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_STRING)
			{
				rsslAckMsgApplyHasText(&rsslMsgPtr->ackMsg);
				rsslMsgPtr->ackMsg.text.length = (*tokPtr)->end - (*tokPtr)->start;
				rsslMsgPtr->ackMsg.text.data = &_jsonMsg[(*tokPtr)->start];
				(*tokPtr)++;
				break;
			}
			if ((*tokPtr)->type == JSMN_PRIMITIVE && _jsonMsg[(*tokPtr)->start] == 'n')
			{
				rsslAckMsgApplyHasText(&rsslMsgPtr->ackMsg);
				rsslMsgPtr->ackMsg.text.length = 0;
				rsslMsgPtr->ackMsg.text.data = 0;
				(*tokPtr)++;
				break;
			}
			unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			// Need to wait until the end to encode payload
			// Save the data start token index and move past
			*dataPtr = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'e':	// Extended header
		{
			(*tokPtr)++;
			bufPtr = &rsslMsgPtr->ackMsg.extendedHeader;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslAckMsgApplyHasExtendedHdr(&rsslMsgPtr->ackMsg);
			break;
		}
		case 'o':	// Qualified Stream
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
			{
				if (_jsonMsg[(*tokPtr)->start] == '1')
					rsslAckMsgApplyQualifiedStream(&rsslMsgPtr->ackMsg);
				(*tokPtr)++;
			}
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}	// End of switch

	}	// End of while

	return true;
}

bool jsonToRwfConverter::processGenericMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{

	int ret = 0;

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	// Default is complete
	rsslGenericMsgApplyMessageComplete(&rsslMsgPtr->genericMsg);

	(*tokPtr)++;

	while ((*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < msgTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'b':
		{
			// Msg Base (just skip)
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_OBJECT)
				skipObject(tokPtr);
			else
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			// Need to wait until the end to encode payload
			// Save the data start token index and move past
			*dataPtr = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'c':	// Complete
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '0')
				rsslMsgPtr->genericMsg.flags &= ~RSSL_GNMF_MESSAGE_COMPLETE;
			(*tokPtr)++;
			break;
		}
		case 'k':	// Key
		{
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, attribPtr) == false)
				return false;
			rsslGenericMsgApplyHasMsgKey(&rsslMsgPtr->genericMsg);
			break;
		}
		case 'r':	// Request Key
		{
			(*tokPtr)++;
			if (processKey(tokPtr, &rsslMsgPtr->genericMsg.reqMsgKey, reqKeyattrib) == false)
				return false;
			rsslMsgPtr->genericMsg.flags |= RSSL_GNMF_HAS_REQ_MSG_KEY;
			break;
		}
		case 'n':	// Sequence Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->genericMsg.seqNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								       &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslGenericMsgApplyHasSeqNum(&rsslMsgPtr->genericMsg);
			break;
		}
		case 's':	// Secondary Sequence Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->genericMsg.secondarySeqNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								       &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslGenericMsgApplyHasSecSeqNum(&rsslMsgPtr->genericMsg);
			break;
		}
		case 'm':	// Part Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->genericMsg.partNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									&_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslGenericMsgApplyHasPartNum(&rsslMsgPtr->genericMsg);
			break;
		}
		case 'p':	// Permission Data
		{
			(*tokPtr)++;
			bufPtr = &rsslMsgPtr->genericMsg.permData;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslGenericMsgApplyHasPermData(&rsslMsgPtr->genericMsg);
			break;
		}
		case 'e':	// Extended header
		{
			(*tokPtr)++;
			bufPtr = &rsslMsgPtr->genericMsg.extendedHeader;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslGenericMsgApplyHasExtendedHdr(&rsslMsgPtr->genericMsg);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

bool jsonToRwfConverter::processPostMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	rsslPostMsgApplyPostComplete(&rsslMsgPtr->postMsg);

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	(*tokPtr)++;

	while ((*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < msgTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'b':
		{
			// Msg Base (just skip)
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_OBJECT)
				skipObject(tokPtr);
			else
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			break;
		}
		case 'i':	// Post User Info
		{
			(*tokPtr)++;
			if (populatePostUserInfo(tokPtr,&rsslMsgPtr->postMsg.postUserInfo) == false)
				return false;
			break;
		}
		case 'k':	// Key
		{
			(*tokPtr)++;

			if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, attribPtr) == false)
				return false;
			rsslPostMsgApplyHasMsgKey(&rsslMsgPtr->postMsg);
			break;
		}
		case 'n':	// Sequence Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->postMsg.seqNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								    &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslPostMsgApplyHasSeqNum(&rsslMsgPtr->postMsg);
			break;
		}
		case 'm':	// Part Number
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->postMsg.partNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								     &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslPostMsgApplyHasPartNum(&rsslMsgPtr->postMsg);
			break;
		}
		case 'c':	// Complete
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '0')
				rsslMsgPtr->postMsg.flags &= ~RSSL_PSMF_POST_COMPLETE;
			(*tokPtr)++;
			break;
		}
		case 'u':	// Post Id
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->postMsg.postId = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								    &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslPostMsgApplyHasPostId(&rsslMsgPtr->postMsg);
			break;
		}
		case 'a':	// Acknowledgement
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslPostMsgApplyAck(&rsslMsgPtr->postMsg);
			(*tokPtr)++;
			break;
		}
		case 'r':	// Post User Rights
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			rsslMsgPtr->postMsg.postUserRights = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
																&_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			rsslPostMsgApplyHasPostUserRights(&rsslMsgPtr->postMsg);
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			// Need to wait until the end to encode payload
			// Save the data start token index and move past
			*dataPtr = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'e':	// Extended Header
		{
			(*tokPtr)++;
			bufPtr = &rsslMsgPtr->postMsg.extendedHeader;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslPostMsgApplyHasExtendedHdr(&rsslMsgPtr->postMsg);
			break;
		}
		case 'p':	// Permission Data
		{
			(*tokPtr)++;
			 bufPtr = &rsslMsgPtr->postMsg.permData;
			if (processBuffer(tokPtr, &bufPtr, 0) == false)
				return false;
			rsslPostMsgApplyHasPermData(&rsslMsgPtr->postMsg);
			break;
		}
		default:
		{
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
		}
	}
	return true;
}

bool jsonToRwfConverter::processFieldList(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslFieldList fieldList;
	jsmntok_t *fieldListTok = *tokPtr;
	jsmntok_t *standardData = 0;
	jsmntok_t *setData = 0;
	jsmntok_t *entryTok = 0;
	jsmntok_t *dataTok = 0;
	jsmntok_t *infoTok = 0;
	jsmntok_t *tmpTok = 0;

	rsslClearFieldList(&fieldList);

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < fieldListTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'i':	// Info
		{
			rsslFieldListApplyHasInfo(&fieldList);
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			infoTok = (*tokPtr);
			(*tokPtr)++;
			while( (*tokPtr) < _tokensEndPtr &&
			       (*tokPtr)->end < infoTok->end)
			{
				if ((*tokPtr)->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				switch (_jsonMsg[(*tokPtr)->start])
				{
				case 'd':	// DictionaryId
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
						return false;
					}
					fieldList.dictionaryId = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
										&_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
					break;
				}
				case 'n':	// FieldList Number
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
						return false;
					}
					fieldList.fieldListNum = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
										&_jsonMsg[(*tokPtr)->end]);
						(*tokPtr)++;
						break;
					}
				default:
					unexpectedParameter(*tokPtr, __LINE__, __FILE__);
					return false;
				}
			}
			break;
		}
		case 's':	// Set Data
		{
			rsslFieldListApplyHasSetData(&fieldList);
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			setData = *tokPtr;

			// Need to parse out the setId here
			tmpTok = setData + 1;
			for (int i = 0; i < setData->size; i++)
			{
				if (tmpTok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__);
					return false;
				}
				switch (_jsonMsg[tmpTok->start])
				{
				case 'i': // Set Id
				{
					tmpTok++;
					if (tmpTok->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
						return false;
					}
					fieldList.setId = rtr_atoi_size(&_jsonMsg[tmpTok->start],
													&_jsonMsg[tmpTok->end]);
					rsslFieldListApplyHasSetId(&fieldList);
					tmpTok++;
					i = setData->size; // Break out of for Loop
					break;
				}
				case 'd':	// The Set Data
				{
					tmpTok++;
					if (tmpTok->type != JSMN_ARRAY)
					{
						unexpectedTokenType(JSMN_ARRAY, tmpTok, __LINE__, __FILE__);
						return false;
					}
					skipArray(&tmpTok);
				}
				default:
					unexpectedParameter(tmpTok, __LINE__, __FILE__);
					return false;
				}
			}
			// Skip the Data array to process later
			if ((*tokPtr)->type != JSMN_OBJECT)
				return false;
			skipObject(tokPtr);
			break;
		}
		case 'd':	// Standard data
		{
			rsslFieldListApplyHasStandardData(&fieldList);
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
				return false;
			// Skip the Data array to process later
			standardData = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		}
	}
	if ((_rsslRet = rsslEncodeFieldListInit(&_iter, &fieldList, (RsslLocalFieldSetDefDb *)setDb, 0)) < RSSL_RET_SUCCESS )
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (setData)
	{
		if (processSetFieldEntries(setData, &fieldList, (RsslLocalFieldSetDefDb *)setDb) == false)
			return false;
	}
	if (standardData)
	{
		if (processStandardFieldEntries(standardData, &fieldList) == false)
			return false;
	}
	if ((_rsslRet = rsslEncodeFieldListComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS )
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool jsonToRwfConverter::processStandardFieldEntries(jsmntok_t *dataTok, RsslFieldList *fieldListPtr)
{
	RsslFieldEntry fieldEntry;
	RsslFieldId fieldId;
	jsmntok_t *fieldTok = (dataTok + 1);
	void *voidPtr = 0;
	RsslBuffer *bufPtr;
	const RsslDictionaryEntry *def;
	int numEntries = 0;

	while( fieldTok < _tokensEndPtr &&
	       fieldTok->end < dataTok->end)
	{
		if (fieldTok->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, fieldTok, __LINE__, __FILE__);
			return false;
		}
		fieldId = rtr_atoi_size(&_jsonMsg[fieldTok->start],
					&_jsonMsg[fieldTok->end]);
		fieldTok++;

		def = _dictionaryList[0]->entriesArray[fieldId];
		if (def)
		{
			rsslClearFieldEntry(&fieldEntry);
			voidPtr = 0;
			bufPtr = &fieldEntry.encData;
			fieldEntry.fieldId = fieldId;
			fieldEntry.dataType = def->rwfType;

			if (fieldEntry.dataType < RSSL_DT_SET_PRIMITIVE_MAX)
			{
				if (fieldEntry.dataType != RSSL_DT_ARRAY)
				{
					// Normal primitive processing
					if (processPrimitive(fieldEntry.dataType, &fieldTok,
							     &bufPtr, &voidPtr) == false)
						return false;
					if ((_rsslRet = rsslEncodeFieldEntry(&_iter, &fieldEntry, voidPtr)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
				}
				else
				{
					// Array is more of a container than a primitive so treat it that way
					if ((_rsslRet = rsslEncodeFieldEntryInit(&_iter, &fieldEntry, 0)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
					if (processPrimitive(fieldEntry.dataType, &fieldTok, &bufPtr, &voidPtr) == false)
						return false;
					if ((_rsslRet = rsslEncodeFieldEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
				}
			}
			else
			{
					if ((_rsslRet = rsslEncodeFieldEntryInit(&_iter, &fieldEntry, 0)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}

					if (processContainer(&fieldTok, fieldEntry.dataType, 0) == false)
						return false;

					if ((_rsslRet = rsslEncodeFieldEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
			}
		}
	}	// End While

	return true;
}

bool jsonToRwfConverter::processSetFieldEntries(jsmntok_t *tokPtr,
												RsslFieldList *fieldListPtr,
												RsslLocalFieldSetDefDb *setDefDbPtr)
{
	RsslFieldEntry fieldEntry;
	RsslBuffer *bufPtr;

	jsmntok_t *setDataTok = tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *tmpTok = 0;
	void *voidPtr;

	RsslFieldSetDefEntry* rsslFieldSetDefEntryPtr = 0;

	tokPtr++;
	while( tokPtr < _tokensEndPtr &&
		   tokPtr->end < setDataTok->end)
	{
		if (tokPtr->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[tokPtr->start])
		{
		case 'i':	// Set Identifier
		{
			tokPtr++;
			//We already parsed this before
			tokPtr++;
			break;
		}
		case 'd':	// The Set Data
		{
			tokPtr++;
			if (tokPtr->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, tokPtr, __LINE__, __FILE__);
				return false;
			}
			dataTok = tokPtr;
			skipArray(&tokPtr);
			break;
		}
		default:
			unexpectedParameter(tokPtr, __LINE__, __FILE__);
			return false;
		}
	}

	if (setDefDbPtr->definitions[fieldListPtr->setId].setId != fieldListPtr->setId)
	{
		error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
		return false;
	}
	rsslFieldSetDefEntryPtr = setDefDbPtr->definitions[fieldListPtr->setId].pEntries;

	if (rsslFieldSetDefEntryPtr == 0)
	{
		error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (setDefDbPtr->definitions[fieldListPtr->setId].count != dataTok->size)
	{
		error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
		return false;
	}

	if ( dataTok == 0 )
	{
		error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
		return false;
	}
	if (dataTok->type != JSMN_ARRAY)
	{
		unexpectedTokenType(JSMN_ARRAY, dataTok, __LINE__, __FILE__);
		return false;
	}

	// Encode the set data
	tmpTok = dataTok;
	tmpTok++;
	for (int i = 0; i < dataTok->size; i++)
	{
		// Make sure our type is in the valid set data range
		if ( rsslFieldSetDefEntryPtr[i].dataType < RSSL_DT_SET_PRIMITIVE_MIN ||
			 rsslFieldSetDefEntryPtr[i].dataType > RSSL_DT_SET_PRIMITIVE_MAX)
		{
			error(INVALID_PRIMITIVE_TYPE, __LINE__, __FILE__);
			return false;
		}

		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = rsslFieldSetDefEntryPtr[i].fieldId;
		fieldEntry.dataType = _primitiveEncodeTypeTable[rsslFieldSetDefEntryPtr[i].dataType];

		bufPtr = &fieldEntry.encData;
		voidPtr = 0;

		if (processPrimitive(fieldEntry.dataType, &tmpTok,
							 &bufPtr, &voidPtr) == false)
			return false;

		if ((_rsslRet = rsslEncodeFieldEntry(&_iter, &fieldEntry, voidPtr)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

bool jsonToRwfConverter::processElementList(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslElementList elementList;
	RsslElementEntry elementEntry = RSSL_INIT_ELEMENT_ENTRY;
	jsmntok_t *elementListTok = *tokPtr;
	jsmntok_t *stdDataTok = 0;
	jsmntok_t *setData = 0;
	jsmntok_t *tmpTok;


	rsslClearElementList(&elementList);

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < elementListTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'i':	// Info
		{
			rsslElementListApplyHasInfo(&elementList);
			(*tokPtr)++;
			break;
		}
		case 's':	// Set Data
		{
			rsslElementListApplyHasSetData(&elementList);
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			setData = *tokPtr;

			// Need to parse out the setId here
			tmpTok = setData + 1;
			for (int i = 0; i < setData->size; i++)
			{
				if (tmpTok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__);
					return false;
				}
				switch (_jsonMsg[tmpTok->start])
				{
				case 'i': // Set Id
				{
					tmpTok++;
					if (tmpTok->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
						return false;
					}
					elementList.setId = rtr_atoi_size(&_jsonMsg[tmpTok->start],
													  &_jsonMsg[tmpTok->end]);
					rsslElementListApplyHasSetId(&elementList);
					tmpTok++;
					i = setData->size; // Break out of for Loop
					break;
				}
				case 'd':	// The Set Data
				{
					tmpTok++;
					if (tmpTok->type != JSMN_ARRAY)
					{
						unexpectedTokenType(JSMN_ARRAY, tmpTok, __LINE__, __FILE__);
						return false;
					}
					skipArray(&tmpTok);
				}
				default:
					unexpectedParameter(tmpTok, __LINE__, __FILE__);
					return false;
				}
			}
			// Skip the SetData object to process later
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			skipObject(tokPtr);
			break;
		}
		case 'd':	// Standard Data
		{
			rsslElementListApplyHasStandardData(&elementList);
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the Data array to process later
			stdDataTok = *tokPtr;
			skipArray(tokPtr);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}

	if ( (_rsslRet = rsslEncodeElementListInit(&_iter, &elementList, (RsslLocalElementSetDefDb*)setDb, 0)) < RSSL_RET_SUCCESS )
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	if (setData)
	{
		if (processSetElementEntries(setData, &elementList, (RsslLocalElementSetDefDb*)setDb) == false)
			return false;
	}

	if (stdDataTok)
	{
		if (processStandardElementEntries(stdDataTok, &elementList) == false)
			return false;
	}

	if ((_rsslRet = rsslEncodeElementListComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	return true;
}
bool jsonToRwfConverter::processSetElementEntries(jsmntok_t *tokPtr,
												  RsslElementList* elementListPtr,
												  RsslLocalElementSetDefDb *setDefDbPtr)
{
	RsslElementEntry elementEntry;
	RsslBuffer *bufPtr;

	jsmntok_t *setDataTok = tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *tmpTok = 0;
	void *voidPtr;

	RsslElementSetDefEntry* rsslElementSetDefEntryPtr = 0;

	tokPtr++;
	while( tokPtr < _tokensEndPtr &&
		   tokPtr->end < setDataTok->end)
	{
		if (tokPtr->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[tokPtr->start])
		{
		case 'i':	// Set Identifier
		{
			tokPtr++;
			//We already parsed this before
			tokPtr++;
			break;
		}
		case 'd':	// The Set Data
		{
			tokPtr++;
			if (tokPtr->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, tokPtr, __LINE__, __FILE__);
				return false;
			}
			dataTok = tokPtr;
			skipArray(&tokPtr);
			break;
		}
		default:
			unexpectedParameter(tokPtr, __LINE__, __FILE__);
			return false;
		}
	}

	if (setDefDbPtr->definitions[elementListPtr->setId].setId != elementListPtr->setId)
	{
		error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
		return false;
	}

	rsslElementSetDefEntryPtr = setDefDbPtr->definitions[elementListPtr->setId].pEntries;

	if (rsslElementSetDefEntryPtr == 0)
	{
		error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (setDefDbPtr->definitions[elementListPtr->setId].count != dataTok->size)
	{
		error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
		return false;
	}

	if ( dataTok == 0)
	{
		error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (dataTok->type != JSMN_ARRAY)
	{
		unexpectedTokenType(JSMN_ARRAY, dataTok, __LINE__, __FILE__);
		return false;
	}

	// Encode the set data
	tmpTok = dataTok;
	tmpTok++;
	for (int i = 0; i < dataTok->size; i++)
	{
		// Make sure our type is in the valid set data range
		if ( rsslElementSetDefEntryPtr[i].dataType < RSSL_DT_SET_PRIMITIVE_MIN ||
			 rsslElementSetDefEntryPtr[i].dataType > RSSL_DT_SET_PRIMITIVE_MAX)
		{
			error(INVALID_PRIMITIVE_TYPE, __LINE__, __FILE__);
			return false;
		}
		rsslClearElementEntry(&elementEntry);
		elementEntry.name.data = rsslElementSetDefEntryPtr[i].name.data;
		elementEntry.name.length = rsslElementSetDefEntryPtr[i].name.length;
		elementEntry.dataType = _primitiveEncodeTypeTable[rsslElementSetDefEntryPtr[i].dataType];

		bufPtr = &elementEntry.encData;
		voidPtr = 0;

		if (processPrimitive(elementEntry.dataType, &tmpTok,
							 &bufPtr, &voidPtr) == false)
			return false;

		if ((_rsslRet = rsslEncodeElementEntry(&_iter, &elementEntry, voidPtr)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}
bool jsonToRwfConverter::processStandardElementEntries(jsmntok_t *stdDataTok, RsslElementList *elementListPtr)
{
	int numEntries;
	jsmntok_t *tmpTok;
	jsmntok_t *entryTok;
	jsmntok_t *dataTok;
	RsslElementEntry elementEntry;
	RsslBuffer *bufPtr;
	void *voidPtr;

	tmpTok = stdDataTok + 1;
	numEntries = stdDataTok->size;
	for (int i = 0; i < numEntries; i++)
	{
		if (tmpTok->type != JSMN_OBJECT)
		{
			unexpectedTokenType(JSMN_OBJECT, tmpTok, __LINE__, __FILE__);
			return false;
		}
		entryTok = tmpTok;
		tmpTok++;

		rsslClearElementEntry(&elementEntry);
		while( tmpTok < _tokensEndPtr &&
			   tmpTok->end < entryTok->end )
		{
			if (tmpTok->type != JSMN_STRING)
			{
				unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__);
				return false;
			}
			switch (_jsonMsg[tmpTok->start])
			{
			case 'n':	// Name
			{
				tmpTok++;
				if (tmpTok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__);
					return false;
				}
				elementEntry.name.data = &_jsonMsg[tmpTok->start];
				elementEntry.name.length = tmpTok->end - tmpTok->start;
				tmpTok++;
				break;
			}
			case 't':	// Type
			{
				tmpTok++;
				if (tmpTok->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
					return false;
				}
				elementEntry.dataType = rtr_atoi_size(&_jsonMsg[tmpTok->start],
													  &_jsonMsg[tmpTok->end]);
				tmpTok++;
				break;
			}
			case 'd':	// Data
			{
				tmpTok++;
				dataTok = tmpTok;

				voidPtr = 0;
				if (elementEntry.dataType < RSSL_DT_SET_PRIMITIVE_MAX)
				{
					if (elementEntry.dataType != RSSL_DT_ARRAY)
					{
						// Normal primitive processing
						bufPtr = &elementEntry.encData;
						if (processPrimitive(elementEntry.dataType, &tmpTok,
											 &bufPtr, &voidPtr) == false)
							return false;

						if (rsslEncodeElementEntry(&_iter, &elementEntry, voidPtr) < RSSL_RET_SUCCESS)
							return false;
					}
					else
					{
						// Array is more of a container than a primitive so treat it that way
						if ((_rsslRet = rsslEncodeElementEntryInit(&_iter, &elementEntry, 0)) < RSSL_RET_SUCCESS)
						{
							error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
							return false;
						}
						bufPtr = &elementEntry.encData;
						if (processPrimitive(elementEntry.dataType, &tmpTok,
											 &bufPtr, &voidPtr) == false)
							return false;
						if ((_rsslRet = rsslEncodeElementEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
						{
							error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
							return false;
						}
					}
				}
				else	// Container
				{
					if ((_rsslRet = rsslEncodeElementEntryInit(&_iter, &elementEntry, 0)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
					if (processContainer(&tmpTok, elementEntry.dataType, 0) == false)
						return false;
					if ((_rsslRet = rsslEncodeElementEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
				}
				break;
			}
			default:
				unexpectedParameter(tmpTok, __LINE__, __FILE__);
				return false;
			}	// switch
		}	// while
	}	// for
	return true;
}
bool jsonToRwfConverter::processFilterList(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslFilterList filterList = RSSL_INIT_FILTER_LIST;
	RsslFilterEntry filterEntry;
	RsslBuffer *bufPtr;
	jsmntok_t *filterListTok = *tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *entryTok = 0;
	jsmntok_t *tmpTok = 0;
	jsmntok_t *entryDataTok = 0;
	int i = 0;

	rsslClearFilterList(&filterList);

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < filterListTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'f':	// Data Format
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			filterList.containerType = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								 &_jsonMsg[(*tokPtr)->end]) + RSSL_DT_CONTAINER_TYPE_MIN;
			(*tokPtr)++;
			break;
		}
		case 'h':	// Total Count Hint
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			filterList.totalCountHint = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								  &_jsonMsg[(*tokPtr)->end]);
			rsslFilterListApplyHasTotalCountHint(&filterList);
			(*tokPtr)++;
			break;
		}
		case 'p':	// Permission Per Entry
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslFilterListApplyHasPerEntryPermData(&filterList);
			(*tokPtr)++;
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the Data array to process later
			dataTok = *tokPtr;
			skipArray(tokPtr);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}

	if ((_rsslRet = rsslEncodeFilterListInit(&_iter, &filterList)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (dataTok)
	{
		tmpTok = dataTok;
		tmpTok++;

		for (i = 0; i < dataTok->size; i++)
		  {
		  	if (tmpTok->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, tmpTok, __LINE__, __FILE__);
				return false;
			}

			entryDataTok = 0;
			rsslClearFilterEntry(&filterEntry);
			filterEntry.containerType = filterList.containerType;
			entryTok = tmpTok;
			tmpTok++;
			while ((tmpTok < _tokensEndPtr &&
				tmpTok->end < entryTok->end))
			{
				if (tmpTok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__);
					return false;
				}

				switch (_jsonMsg[(tmpTok)->start])
				{
				case 'i':	// Filter Id
				{
					tmpTok++;
					if (tmpTok->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
						return false;
					}
					filterEntry.id = rtr_atoi_size(&_jsonMsg[tmpTok->start],
								       &_jsonMsg[tmpTok->end]);
					tmpTok++;
					break;
				}
				case 'a':	// Action
				{
					tmpTok++;
					if (tmpTok->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
						return false;
					}
					filterEntry.action = rtr_atoi_size(&_jsonMsg[tmpTok->start],
								   &_jsonMsg[tmpTok->end]);
					tmpTok++;
					break;
				}
				case 'f':	// Data Format
				{
					tmpTok++;
					if (tmpTok->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
						return false;
					}
					filterEntry.containerType = rtr_atoi_size(&_jsonMsg[tmpTok->start],
									  &_jsonMsg[tmpTok->end]) + RSSL_DT_CONTAINER_TYPE_MIN;
					rsslFilterEntryCheckHasContainerType(&filterEntry);
					tmpTok++;
					break;
				}
				case 'p':	// Permission Data
				{
					tmpTok++;
					bufPtr = &filterEntry.permData;
					if (processBuffer(&tmpTok, &bufPtr, 0) == false)
						return false;
					rsslFilterEntrySetHasPermData(&filterEntry);
					break;
				}
				case 'd':	// Filter Entry Data
				{
					tmpTok++;
					// Need to wait until the end to encode payload
					// Save the data start token index and move past
					entryDataTok = tmpTok;
					skipObject(&tmpTok);
					break;
				}
				default:
					unexpectedParameter(tmpTok, __LINE__, __FILE__);
					return false;
				} // End Case

			} // End While

			// Encode each Filter Entry

			if (entryDataTok)
			{
				if ((_rsslRet = rsslEncodeFilterEntryInit(&_iter, &filterEntry, 0)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
				if (processContainer(&entryDataTok, filterEntry.containerType, 0) == false)
					return false;
				if ((_rsslRet = rsslEncodeFilterEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			else
			{
				if ((_rsslRet = rsslEncodeFilterEntry(&_iter, &filterEntry)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
		} // End for
	} // End if

	if ((_rsslRet = rsslEncodeFilterListComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}
bool jsonToRwfConverter::processVector(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslVector vector = RSSL_INIT_VECTOR;
	RsslVectorEntry vectorEntry;
	void* localSetDb = 0;
	jsmntok_t *vectorTok = *tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *summaryTok = 0;
	jsmntok_t *entryTok = 0;
	jsmntok_t *setDefTok = 0;
	jsmntok_t *tmpTok = 0;
	jsmntok_t *entryDataTok = 0;
	RsslBuffer *bufPtr;
	int i = 0;

	rsslClearVector(&vector);

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < vectorTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'f':	// Entry Data Format
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			vector.containerType = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
												 &_jsonMsg[(*tokPtr)->end]) + RSSL_DT_CONTAINER_TYPE_MIN;
			(*tokPtr)++;
			break;
		}
		case 'h':	// Total Count Hint
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			vector.totalCountHint = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
												  &_jsonMsg[(*tokPtr)->end]);
			rsslVectorApplyHasTotalCountHint(&vector);
			(*tokPtr)++;
			break;
		}
		case 'o':	// Supports Sorting (ordered)
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslVectorApplySupportsSorting(&vector);
			(*tokPtr)++;
			break;
		}
		case 'p':	// Permission Per Entry
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslVectorApplyHasPerEntryPermData(&vector);
			(*tokPtr)++;
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the Data array to process later
			dataTok = *tokPtr;
			skipArray(tokPtr);
			break;
		}
		case 's':	// Summary Data
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the Data array to process later
			rsslVectorApplyHasSummaryData(&vector);
			summaryTok = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'l':	// Set Definitions
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the defs to process later
			rsslVectorApplyHasSetDefs(&vector);
			setDefTok = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}

	if ((_rsslRet = rsslEncodeVectorInit(&_iter, &vector, 0, 0)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (setDefTok)
	{
		switch (vector.containerType)
		{
		case RSSL_DT_ELEMENT_LIST:
		{
			if (localSetDb = processElementSetDef(setDefTok))
			{
				if ( (_rsslRet = rsslEncodeLocalElementSetDefDb(&_iter, (RsslLocalElementSetDefDb*)localSetDb)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			break;
		}
		case RSSL_DT_FIELD_LIST:
		{
			if (localSetDb = processFieldSetDef(setDefTok))
			{
				if ( (_rsslRet = rsslEncodeLocalFieldSetDefDb(&_iter, (RsslLocalFieldSetDefDb*)localSetDb)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			break;
		}
		default:
			error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
			return false;
		}

		if ( (_rsslRet = rsslEncodeVectorSetDefsComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

	}

	if (summaryTok)
	{
		if (processContainer(&summaryTok, vector.containerType, 0) == false)
			return false;
		if ((_rsslRet = rsslEncodeVectorSummaryDataComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}
	if (dataTok)
	{
		tmpTok = dataTok;
		tmpTok++;
		for (i = 0; i < dataTok->size; i++)
		  {
		  	if (tmpTok->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, tmpTok, __LINE__, __FILE__);
				return false;
			}

			entryDataTok = 0;
			rsslClearVectorEntry(&vectorEntry);
			entryTok = tmpTok;
			tmpTok++;
			while ((tmpTok < _tokensEndPtr &&
				tmpTok->end < entryTok->end))
			{
				if (tmpTok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__);
					return false;
				}

				switch (_jsonMsg[(tmpTok)->start])
				{
				case 'a':	// Action
				{
					tmpTok++;
					if (tmpTok->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
						return false;
					}
					vectorEntry.action = rtr_atoi_size(&_jsonMsg[tmpTok->start],
													   &_jsonMsg[tmpTok->end]);
					tmpTok++;
					break;
				}
				case 'i':	// Index
				{
					tmpTok++;
					if (tmpTok->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
						return false;
					}
					vectorEntry.index = rtr_atoi_size(&_jsonMsg[tmpTok->start],
													  &_jsonMsg[tmpTok->end]);
					tmpTok++;
					break;
				}
				case 'd':	// Vector Entry Data
				{
					tmpTok++;
					// Need to wait until the end to encode payload
					// Save the data start token index and move past
					entryDataTok = tmpTok;
					skipObject(&tmpTok);
					break;
				}
				case 'p':	// Permission Data
				{
					tmpTok++;
					bufPtr = &vectorEntry.permData;
					if (processBuffer(&tmpTok, &bufPtr, 0) == false)
						return false;
					rsslVectorEntryApplyHasPermData(&vectorEntry);
					break;
				}
				default:
					unexpectedParameter(tmpTok, __LINE__, __FILE__);
					return false;
				} // End Case

			} // End While

			// Encode each Vector Entry

			if (entryDataTok)
			{
				if ((_rsslRet = rsslEncodeVectorEntryInit(&_iter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
				if (processContainer(&entryDataTok, vector.containerType, localSetDb) == false)
					return false;
				if ((_rsslRet = rsslEncodeVectorEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			else
			{
				if ((_rsslRet = rsslEncodeVectorEntry(&_iter, &vectorEntry)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
		} // End for
	} // End if(dataTok)

	if ((_rsslRet = rsslEncodeVectorComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool jsonToRwfConverter::processMap(jsmntok_t ** const tokPtr, void* setDb)
{
	void* localSetDb = 0;
	//	RsslLocalElementSetDefDb* _elementSetDefDb = 0;
	//	RsslLocalFieldSetDefDb* _fieldSetDefDb = 0;
	RsslMap map = RSSL_INIT_MAP;
	void* encKeyPtr = 0;
	RsslBuffer *bufPtr;
	RsslBuffer buffer;
	RsslMapEntry mapEntry;

	jsmntok_t *mapTok = *tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *setDefTok = 0;
	jsmntok_t *summaryTok = 0;
	jsmntok_t *entryTok = 0;
	jsmntok_t *tmpTok = 0;
	jsmntok_t *entryDataTok = 0;

	int i = 0;

	rsslClearMap(&map);

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < mapTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'f':	// Entry Data Format
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			map.containerType = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
											  &_jsonMsg[(*tokPtr)->end]) + RSSL_DT_CONTAINER_TYPE_MIN;
			(*tokPtr)++;
			break;
		}
		case 'k':	// Key Primitive Type
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			map.keyPrimitiveType = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
												 &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			break;
		}
		case 'i':	// Key Field Id
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			map.keyFieldId = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
										   &_jsonMsg[(*tokPtr)->end]);
			rsslMapApplyHasKeyFieldId(&map);
			(*tokPtr)++;
			break;
		}
		case 'h':	// Total Count Hint
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			map.totalCountHint = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
											   &_jsonMsg[(*tokPtr)->end]);
			rsslMapApplyHasTotalCountHint(&map);
			(*tokPtr)++;
			break;
		}
		case 'p':	// Permission Per Entry
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if (_jsonMsg[(*tokPtr)->start] == '1')
				rsslMapApplyHasPerEntryPermData(&map);
			(*tokPtr)++;
			break;
		}
		case 'd':	// Data
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the Data array to process later
			dataTok = *tokPtr;
			skipArray(tokPtr);
			break;
		}
		case 's':	// Summary Data
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip summary Data to process Later
			rsslMapApplyHasSummaryData(&map);
			summaryTok = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'l':	// Set Definitions
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the defs to process later
			rsslMapApplyHasSetDefs(&map);
			setDefTok = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}

	if ((_rsslRet = rsslEncodeMapInit(&_iter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (setDefTok)
	{
		switch (map.containerType)
		{
		case RSSL_DT_ELEMENT_LIST:
		{
			if (localSetDb = processElementSetDef(setDefTok))
			{
				if ( (_rsslRet = rsslEncodeLocalElementSetDefDb(&_iter, (RsslLocalElementSetDefDb*)localSetDb)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			break;
		}
		case RSSL_DT_FIELD_LIST:
		{
			if (localSetDb = processFieldSetDef(setDefTok))
			{
				if ( (_rsslRet = rsslEncodeLocalFieldSetDefDb(&_iter, (RsslLocalFieldSetDefDb*)localSetDb)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			break;
		}
		default:
			error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
			return false;
		}

		if ( (_rsslRet = rsslEncodeMapSetDefsComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}
	if (summaryTok)
	{
		if (processContainer(&summaryTok, map.containerType, localSetDb) == false)
			return false;
		if ((_rsslRet = rsslEncodeMapSummaryDataComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}
	if (dataTok)
	{
		tmpTok = dataTok;
		tmpTok++;
		for (i = 0; i < dataTok->size; i++)
		  {
		  	if (tmpTok->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, tmpTok, __LINE__, __FILE__);
				return false;
			}

			entryDataTok = 0;
			rsslClearMapEntry(&mapEntry);
			encKeyPtr = 0;
			bufPtr = &buffer;
			entryTok = tmpTok;
			tmpTok++;
			while ((tmpTok < _tokensEndPtr &&
				tmpTok->end < entryTok->end))
			{
				if (tmpTok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__);
					return false;
				}

				switch (_jsonMsg[(tmpTok)->start])
				{
				case 'a':	// Action
				{
					tmpTok++;
					if (tmpTok->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__);
						return false;
					}
					mapEntry.action = rtr_atoi_size(&_jsonMsg[tmpTok->start],
													&_jsonMsg[tmpTok->end]);
					tmpTok++;
					break;
				}
				case 'k':	// Key
				{
					tmpTok++;
					if (processPrimitive(map.keyPrimitiveType, &tmpTok,
										 &bufPtr, &encKeyPtr) == false)
						return false;
					if (encKeyPtr == 0)
						encKeyPtr = bufPtr;
					break;
				}
				case 'p':	// Permission Data
				{
					tmpTok++;
					bufPtr = &mapEntry.permData;
					if (processBuffer(&tmpTok, &bufPtr, 0) == false)
						return false;
					rsslMapEntryApplyHasPermData(&mapEntry);
					break;
				}
				case 'd':	// Map Entry Data
				{
					tmpTok++;
					// Need to wait until the end to encode payload
					// Save the data start token index and move past
					entryDataTok = tmpTok;
					skipObject(&tmpTok);
					break;
				}
				default:
					unexpectedParameter(tmpTok, __LINE__, __FILE__);
					return false;
				} // End Case

			} // End While

			// Encode each Map Entry

			if (entryDataTok)
			{
				if ((_rsslRet = rsslEncodeMapEntryInit(&_iter, &mapEntry, encKeyPtr, 0)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
				if (processContainer(&entryDataTok, map.containerType, localSetDb) == false)
					return false;
				if ((_rsslRet = rsslEncodeMapEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			else
			{
				if ((_rsslRet = rsslEncodeMapEntry(&_iter, &mapEntry, encKeyPtr)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
		} // End for
	} // End if(dataTok)

	if ((_rsslRet = rsslEncodeMapComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}
bool jsonToRwfConverter::processSeries(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslSeries series = RSSL_INIT_SERIES;
	RsslSeriesEntry seriesEntry;
	void* localSetDb = 0;
	jsmntok_t *seriesTok = *tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *tmpTok = 0;
	jsmntok_t *summaryTok = 0;
	jsmntok_t *setDefTok = 0;
	int i = 0;

	rsslClearSeries(&series);

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < seriesTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'f':	// Entry Data Format
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			series.containerType = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
												 &_jsonMsg[(*tokPtr)->end]) + RSSL_DT_CONTAINER_TYPE_MIN;
			(*tokPtr)++;
			break;
		}
		case 's':	// Summary Data
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the Summary Data to process later
			rsslSeriesApplyHasSummaryData(&series);
			summaryTok = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'h':	// Total Count Hint
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			series.totalCountHint = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
												  &_jsonMsg[(*tokPtr)->end]);
			rsslSeriesApplyHasTotalCountHint(&series);
			(*tokPtr)++;
			break;
		}
		case 'l':	// Set Definitions
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			// Skip the defs to process later
			rsslSeriesApplyHasSetDefs(&series);
			setDefTok = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		case 'd':	// Map Entry Data
		{
			(*tokPtr)++;
			// Need to wait until the end to encode payload
			// Save the data start token index and move past
			dataTok = *tokPtr;
			skipObject(tokPtr);
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
			break;
		}	// End Case
	}	// End While

	if ((_rsslRet = rsslEncodeSeriesInit(&_iter, &series, 0, 0)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (setDefTok)
	{
		switch (series.containerType)
		{
		case RSSL_DT_ELEMENT_LIST:
		{
			if (localSetDb = processElementSetDef(setDefTok))
			{
				if ( (_rsslRet = rsslEncodeLocalElementSetDefDb(&_iter, (RsslLocalElementSetDefDb*)localSetDb)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			break;
		}
		case RSSL_DT_FIELD_LIST:
		{
			if (localSetDb = processFieldSetDef(setDefTok))
			{
				if ( (_rsslRet = rsslEncodeLocalFieldSetDefDb(&_iter, (RsslLocalFieldSetDefDb*)localSetDb)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			break;
		}
		default:
			error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
			return false;
		}

		if ( (_rsslRet = rsslEncodeSeriesSetDefsComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}

	if (summaryTok)
	{
		if (processContainer(&summaryTok, series.containerType, 0) == false)
			return false;
		if ((_rsslRet = rsslEncodeSeriesSummaryDataComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}

	if (dataTok)
	{
		tmpTok = dataTok;
		tmpTok++;
		for (i = 0; i < dataTok->size; i++)
		{
		  	if (tmpTok->type != JSMN_OBJECT)
				return false;
			rsslClearSeriesEntry(&seriesEntry);
			if ((_rsslRet = rsslEncodeSeriesEntryInit(&_iter, &seriesEntry, 0) < RSSL_RET_SUCCESS))
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
			if (processContainer(&tmpTok, series.containerType, localSetDb) == false)
				return false;
			if ((_rsslRet = rsslEncodeSeriesEntryComplete(&_iter, RSSL_TRUE) < RSSL_RET_SUCCESS))
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
		}
	}
	if ((_rsslRet = rsslEncodeSeriesComplete(&_iter, RSSL_TRUE) < RSSL_RET_SUCCESS))
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}
bool jsonToRwfConverter::processMsg(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslJsonMsg jsonMsg;
	rsslClearJsonMsg(&jsonMsg);

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}
	return processMessage(tokPtr, &jsonMsg);

}
//////////////////////////////////////////////////////////////////////
// Primitives
//////////////////////////////////////////////////////////////////////

bool jsonToRwfConverter::processReal(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{

	int i, expVal;
	bool foundE = false;
	jsmntok_t *realTok;

	*ptrBufPtr = 0;
	*ptrVoidPtr = &_realVar;

	rsslClearReal(&_realVar);
	switch ((*tokPtr)->type)
	{
	case JSMN_OBJECT:
	{
		realTok = *tokPtr;
		(*tokPtr)++;
		while( (*tokPtr) < _tokensEndPtr &&
		       (*tokPtr)->end < realTok->end)
		{
			switch (_jsonMsg[(*tokPtr)->start])
			{
			case 'v':
			{
				(*tokPtr)++;
				if (_jsonMsg[(*tokPtr)->start] != 'n')
				{
					_realVar.value = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								       &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				break;
			}
			case 'h':
			{
				(*tokPtr)++;
				if (_jsonMsg[(*tokPtr)->start] != 'n')
				{

					_realVar.hint = rtr_atoui_size(&_jsonMsg[(*tokPtr)->start],
								       &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else
				{
					unexpectedParameter(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				break;
			}
			case 'b':
			{
				(*tokPtr)++;
				if (_jsonMsg[(*tokPtr)->start] != '0')
					_realVar.isBlank = 1;
				(*tokPtr)++;
				break;
			}
			default:
			{
				unexpectedParameter(*tokPtr, __LINE__, __FILE__);
				return false;
			}
			}	// End switch

		}	// End While
		break;
	}
	case JSMN_PRIMITIVE:
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

		// See if we have an 'e'
		for (i = (*tokPtr)->start; i < (*tokPtr)->end; i++)
		{
			if (_jsonMsg[i] == 'e' || _jsonMsg[i] == 'E')
			{
				foundE = true;
				break;
			}
		}
		if (foundE)
		{
			_realVar.value = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start], &_jsonMsg[i]);
			if (_jsonMsg[i+1] == '-')
			{
				expVal = rtr_atoi_size(&_jsonMsg[i+2], &_jsonMsg[(*tokPtr)->end]);
				if (expVal >= NEG_EXP_MIN && expVal <= NEG_EXP_MAX)
					_realVar.hint = _negExponentTable[expVal];
				else
				{
					unexpectedParameter(*tokPtr, __LINE__, __FILE__);
					return false;
				}
			}
			else
			{
				expVal = rtr_atoi_size(&_jsonMsg[i+1], &_jsonMsg[(*tokPtr)->end]);
				if (expVal >= POS_EXP_MIN && expVal <= POS_EXP_MAX)
				{
					_realVar.hint = _posExponentTable[expVal];
				}
			}
		}

		else
			_realVar.value = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],&_jsonMsg[(*tokPtr)->end]);

		(*tokPtr)++;
		break;
	}
	default:
	{
		unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
		return false;
	}
	}	// End switch

	return true;
}

bool jsonToRwfConverter::processDate(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	*ptrBufPtr = 0;
	*ptrVoidPtr = 0;

	switch ((*tokPtr)->type)
	{
	case JSMN_PRIMITIVE:
	{
		if (_jsonMsg[(*tokPtr)->start] != 'n')
		{
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
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
		*ptrVoidPtr = &_dateVar;
		rsslClearDate(&_dateVar);
		return populateDate(tokPtr, &_dateVar);
	}
	default:
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}
}

bool jsonToRwfConverter::processTime(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	*ptrBufPtr = 0;
	*ptrVoidPtr = 0;

	switch ((*tokPtr)->type)
	{
	case JSMN_PRIMITIVE:
	{
		if (_jsonMsg[(*tokPtr)->start] != 'n')
		{
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
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
		*ptrVoidPtr = &_timeVar;
		rsslClearTime(&_timeVar);
		return populateTime(tokPtr, &_timeVar);
	}
	default:
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}
}
bool jsonToRwfConverter::processDateTime(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	*ptrBufPtr = 0;
	*ptrVoidPtr = 0;
	jsmntok_t *dateTimeTok;

	switch ((*tokPtr)->type)
	{
	case JSMN_PRIMITIVE:
	{
		if (_jsonMsg[(*tokPtr)->start] != 'n')
		{
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
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
		dateTimeTok = *tokPtr;
		*ptrVoidPtr = &_dateTimeVar;
		(*tokPtr)++;
		rsslClearDateTime(&_dateTimeVar);

		while( (*tokPtr) < _tokensEndPtr &&
		       (*tokPtr)->end < dateTimeTok->end)
		{
			if ((*tokPtr)->type != JSMN_STRING)
			{
				unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
				return false;
			}

			switch (_jsonMsg[(*tokPtr)->start])
			{
			case 'd':	// Date
			{
				(*tokPtr)++;
				if (populateDate(tokPtr, &_dateTimeVar.date) == false)
					return false;
				break;
			}
			case 't':
			{
				(*tokPtr)++;
				if (populateTime(tokPtr, &_dateTimeVar.time) == false)
					return false;
				break;
			}
			default:
				unexpectedParameter(*tokPtr, __LINE__, __FILE__);
				return false;
			}
		}
		return true;
	}
	default:
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}
}
bool jsonToRwfConverter::processArray(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	RsslBuffer buf;
	RsslBuffer *bufPtr;
	jsmntok_t *dataTok;
	RsslArray array;
	rsslClearArray(&array);
	void *voidPtr;
	int i = 0;

	jsmntok_t* arrayTok = *tokPtr;

	if ((*tokPtr)->type == JSMN_PRIMITIVE)
	{
		if (_jsonMsg[(*tokPtr)->start] == 'n')
		{
			(*tokPtr)++;
			return true;
		}
		else
		{
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}
	(*tokPtr)++;

	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < arrayTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'l':	// length
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				array.itemLength = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								 &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
				break;
			}
		case 't':	// Type
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				array.primitiveType = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								    &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
				break;
			}
		case 'd':	// Data
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_ARRAY)
				{
					unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__);
					return false;
				}

				if ((_rsslRet = rsslEncodeArrayInit(&_iter, &array)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
				dataTok = *tokPtr;
				(*tokPtr)++;
				for (i = 0; i < dataTok->size; i++)
				{
					voidPtr = 0;
					bufPtr = &buf;
					if (processPrimitive(array.primitiveType, tokPtr, &bufPtr, &voidPtr) == false)
						return false;
					rsslEncodeArrayEntry(&_iter, bufPtr, voidPtr);
				}
				if ((_rsslRet = rsslEncodeArrayComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
				break;
			}
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////
//
// Helper functions
//
//////////////////////////////////////////////////////////////////////

bool jsonToRwfConverter::processKey(jsmntok_t ** const tokPtr, RsslMsgKey *keyPtr, jsmntok_t ** const attribTokPtr)
{
	RsslUInt16 flags = RSSL_MKF_NONE;

	jsmntok_t *keyTok = *tokPtr;

	if ((*tokPtr)->type != JSMN_OBJECT)
		return false;

	(*tokPtr)++;

	while( (*tokPtr) < _tokensEndPtr &&
			(*tokPtr)->end < keyTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 's':  // Service Id
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				keyPtr->serviceId =
					rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								  &_jsonMsg[(*tokPtr)->end]);
				flags |= RSSL_MKF_HAS_SERVICE_ID;
				(*tokPtr)++;
				break;
			}
		case 'n':	// Item Name
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				keyPtr->name.length =  (*tokPtr)->end - (*tokPtr)->start;
				keyPtr->name.data = &_jsonMsg[(*tokPtr)->start];
				flags |=  RSSL_MKF_HAS_NAME;
				(*tokPtr)++;
				break;
			}
		case 't':	// Name Type
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				keyPtr->nameType =
					rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								  &_jsonMsg[(*tokPtr)->end]);
				flags |= RSSL_MKF_HAS_NAME_TYPE;
				(*tokPtr)++;
				break;
			}
		case 'x':	// Filter
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				keyPtr->filter =
					rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								  &_jsonMsg[(*tokPtr)->end]);
				flags |= RSSL_MKF_HAS_FILTER;
				(*tokPtr)++;
				break;
			}
		case 'i':	// Identifier
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				keyPtr->identifier =
					rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
								  &_jsonMsg[(*tokPtr)->end]);
				flags |= RSSL_MKF_HAS_IDENTIFIER;
				(*tokPtr)++;
				break;
			}
		case 'a':  //  Attrib Info
			{
				flags |= RSSL_MKF_HAS_ATTRIB;
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_OBJECT)
				{
					unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				jsmntok_t *attribTok = *tokPtr;
				(*tokPtr)++;

				while( (*tokPtr) < _tokensEndPtr && 
						(*tokPtr)->end < attribTok->end)
				{
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
						return false;
					}
					switch (_jsonMsg[(*tokPtr)->start])
					{
					case 'f':  // format
					{
						(*tokPtr)++;
						if ((*tokPtr)->type != JSMN_PRIMITIVE)
						{
							unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
							return false;
						}
						keyPtr->attribContainerType =
							rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
										   &_jsonMsg[(*tokPtr)->end])  + RSSL_DT_CONTAINER_TYPE_MIN;
						(*tokPtr)++;
						break;
					}
					case 'd':  // data
					{
						(*tokPtr)++;
						*attribTokPtr = *tokPtr;
						skipObject(tokPtr);
						break;
					}
					default:
						unexpectedParameter(*tokPtr, __LINE__, __FILE__);
						return false;
					}
				}
			}
		}
	}

	if ((keyPtr->nameType == RDM_LOGIN_USER_COOKIE || keyPtr->nameType == 5) &&
		!(flags & RSSL_MKF_HAS_NAME))
	{
		// User name is contained in a cookie, however RSSL will not keep the nametype unless
		// a name is provided.
		keyPtr->name.length =  1;
		keyPtr->name.data = &_nullUserName;
		flags |=  RSSL_MKF_HAS_NAME;
	}

	keyPtr->flags = flags;

	return true;
}
bool jsonToRwfConverter::populateQos(jsmntok_t ** const tokPtr, RsslQos *qosPtr)
{
	int i = 0;
	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}
	int qosSize = (*tokPtr)->size;
	(*tokPtr)++;

	for (i = 0; i <  qosSize; i+=2)
	{
		if ((*tokPtr)->type != JSMN_STRING)
			return false;
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 't':	// Timeliness
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				qosPtr->timeliness =	rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
													   &_jsonMsg[(*tokPtr)->end]);

				(*tokPtr)++;
				break;

			}
		case 'r':	// Rate
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}

				qosPtr->rate = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
											  &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
				break;
			}
		case 'd':	// Dynamic
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				if (_jsonMsg[(*tokPtr)->start] == '1')
					qosPtr->dynamic = 1;
				(*tokPtr)++;
				break;
			}
		case 'i':	// Time Info
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				qosPtr->timeInfo = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
												  &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
				break;
			}
		case 's':	// Rate Info
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				qosPtr->rateInfo = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
												  &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
				break;
			}
		default:
			{
				unexpectedParameter(*tokPtr, __LINE__, __FILE__);
				return false;
			}
		}
	}
	return true;
}


bool jsonToRwfConverter::populateState(jsmntok_t ** const tokPtr, RsslState* statePtr)
{
	int len = 0;
	if ( (*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	jsmntok_t *stateTok = *tokPtr;
	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < stateTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 's':	// Stream State
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}

			if ( (*tokPtr)->end - (*tokPtr)->start == 1 )
				statePtr->streamState = _jsonMsg[(*tokPtr)->start] - 0x30;
			else
			{
				unexpectedParameter(*tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}

		case 'd':	// Data State
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			if ( (*tokPtr)->end - (*tokPtr)->start == 1 )
				statePtr->dataState = _jsonMsg[(*tokPtr)->start] - 0x30;
			else
			{
				unexpectedParameter(*tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}
		case 'c':	// State Code
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			len = (*tokPtr)->end - (*tokPtr)->start;
			statePtr->code = 0;
			switch (len)
			{
			case 2:
				statePtr->code = (((_jsonMsg[(*tokPtr)->start] - 0x30) * 10) + (_jsonMsg[(*tokPtr)->start+1] - 0x30));
				break;
			case 1:
				statePtr->code = _jsonMsg[(*tokPtr)->start] - 0x30;
				break;
			default:
				unexpectedParameter(*tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}
		case 't':	// Text
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_STRING)
			{
				statePtr->text.length = (*tokPtr)->end - (*tokPtr)->start;
				statePtr->text.data = &_jsonMsg[(*tokPtr)->start];
				(*tokPtr)++;
				break;
			}
			if ((*tokPtr)->type == JSMN_PRIMITIVE && _jsonMsg[(*tokPtr)->start] == 'n')
			{
				statePtr->text.length = 0;
				statePtr->text.data = 0;
				(*tokPtr)++;
				break;
			}
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

bool jsonToRwfConverter::populatePostUserInfo(jsmntok_t ** const tokPtr, RsslPostUserInfo *userInfoPtr)
{
	int len = 0;
	if ( (*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	jsmntok_t *infoTok = *tokPtr;
	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < infoTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'a':	// Post User Addr
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}

			userInfoPtr->postUserAddr = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								   &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
			break;
		}
		case 'u':	// UserId
		{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
					return false;
				}
				userInfoPtr->postUserId = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									 &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

bool jsonToRwfConverter::populateDate(jsmntok_t ** const tokPtr, RsslDate *datePtr)
{
	jsmntok_t *dateTok = *tokPtr;
	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < dateTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'm':	// Month
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE )
				datePtr->month = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								&_jsonMsg[(*tokPtr)->end]);
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}

		case 'd':	// Day
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
				datePtr->day = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
							      &_jsonMsg[(*tokPtr)->end]);
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}
		case 'y':	// Year
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
				datePtr->year = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
							       &_jsonMsg[(*tokPtr)->end]);
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

bool jsonToRwfConverter::populateTime(jsmntok_t ** const tokPtr , RsslTime *timePtr)
{
	jsmntok_t *timeTok = *tokPtr;

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < timeTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'h':	// Hour
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
				timePtr->hour = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
							       &_jsonMsg[(*tokPtr)->end]);
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}
		case 'm':	// minutes
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
				timePtr->minute = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								 &_jsonMsg[(*tokPtr)->end]);
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}
		case 's':	// Seconds
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
				timePtr->second = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								 &_jsonMsg[(*tokPtr)->end]);
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}
		case 'x':	// Milliseconds
		{
			(*tokPtr)++;
			if ((*tokPtr)->type == JSMN_PRIMITIVE)
				timePtr->millisecond = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								      &_jsonMsg[(*tokPtr)->end]);
			else
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
				return false;
			}
			(*tokPtr)++;
			break;
		}
		default:
			unexpectedParameter(*tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

RsslLocalElementSetDefDb* jsonToRwfConverter::getElementSetDefDb()
{
	RsslElementSetDefEntry* defEntryPtr;
	if (_elementSetDefDbs[_iter._encodingLevel] == 0)
	{
		_elementSetDefDbs[_iter._encodingLevel] = new RsslLocalElementSetDefDb;
		defEntryPtr = new RsslElementSetDefEntry[255*(RSSL_ELEMENT_SET_MAX_LOCAL_ID+1)];
		_elementSetDefDbs[_iter._encodingLevel]->entries.data = (char*)defEntryPtr;
		_elementSetDefDbs[_iter._encodingLevel]->entries.length = sizeof(RsslElementSetDefEntry) * 255 *(RSSL_ELEMENT_SET_MAX_LOCAL_ID+1);
		int i = 0;
		for (int j = 0; j <= RSSL_ELEMENT_SET_MAX_LOCAL_ID; j++)
		{
			_elementSetDefDbs[_iter._encodingLevel]->definitions[j].pEntries = &defEntryPtr[i];
			i+= 255;
			_elementSetDefDbs[_iter._encodingLevel]->definitions[j].count = 0;
			_elementSetDefDbs[_iter._encodingLevel]->definitions[j].setId = RSSL_ELEMENT_SET_BLANK_ID;
		}
	}
	else
	{
		for (int j = 0; j <= RSSL_ELEMENT_SET_MAX_LOCAL_ID; j++)
		{
			_elementSetDefDbs[_iter._encodingLevel]->definitions[j].count = 0;
			_elementSetDefDbs[_iter._encodingLevel]->definitions[j].setId = RSSL_ELEMENT_SET_BLANK_ID;
		}
	}
	return 	_elementSetDefDbs[_iter._encodingLevel];
}

RsslLocalFieldSetDefDb* jsonToRwfConverter::getFieldSetDefDb()
{
	RsslFieldSetDefEntry* defEntryPtr;
	if (_fieldSetDefDbs[_iter._encodingLevel] == 0)
	{
		_fieldSetDefDbs[_iter._encodingLevel] = new RsslLocalFieldSetDefDb;
		defEntryPtr = new RsslFieldSetDefEntry[255*(RSSL_FIELD_SET_MAX_LOCAL_ID+1)];
		_fieldSetDefDbs[_iter._encodingLevel]->entries.data = (char*)defEntryPtr;
		_fieldSetDefDbs[_iter._encodingLevel]->entries.length = sizeof(RsslFieldSetDefEntry) * 255 *(RSSL_FIELD_SET_MAX_LOCAL_ID+1);
		int i = 0;
		for (int j = 0; j <= RSSL_FIELD_SET_MAX_LOCAL_ID; j++)
		{
			_fieldSetDefDbs[_iter._encodingLevel]->definitions[j].pEntries = &defEntryPtr[i];
			i+= 255;
			_fieldSetDefDbs[_iter._encodingLevel]->definitions[j].count = 0;
			_fieldSetDefDbs[_iter._encodingLevel]->definitions[j].setId = RSSL_FIELD_SET_BLANK_ID;
		}
	}
	else
	{
		for (int j = 0; j <= RSSL_FIELD_SET_MAX_LOCAL_ID; j++)
		{
			_fieldSetDefDbs[_iter._encodingLevel]->definitions[j].count = 0;
			_fieldSetDefDbs[_iter._encodingLevel]->definitions[j].setId = RSSL_FIELD_SET_BLANK_ID;
		}
	}
	return 	_fieldSetDefDbs[_iter._encodingLevel];
}

RsslLocalElementSetDefDb* jsonToRwfConverter::processElementSetDef(jsmntok_t *tokPtr)
{
	RsslLocalElementSetDefDb* setDefDbPtr = getElementSetDefDb();
	jsmntok_t *setDefTok = tokPtr;
	jsmntok_t *dataTok;
	jsmntok_t *defTok;
	jsmntok_t *setTok;
	int setId;

	int setDefCount = 0;
	int i,j,k;

	tokPtr++;
	while (tokPtr < _tokensEndPtr &&
		   tokPtr->end < setDefTok->end)
	{
		if (tokPtr->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
			return 0;
		}

		switch (_jsonMsg[tokPtr->start])
		{
		case 'c':	// Count (Number of Set Defiinitions)
		{
			tokPtr++;
			if (tokPtr->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
				return 0;
			}
			setDefCount = rtr_atoi_size( &_jsonMsg[tokPtr->start], &_jsonMsg[tokPtr->end]);
			tokPtr++;
			break;
		}
		case 'd':	// The array of set definitions
		{
			tokPtr++;
			if (tokPtr->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, tokPtr, __LINE__, __FILE__);
				return 0;
			}
			dataTok = tokPtr;
			tokPtr++;
			for (i = 0; i < dataTok->size; i++)
			{
				if (tokPtr->type != JSMN_OBJECT)
				{
					unexpectedTokenType(JSMN_OBJECT, tokPtr, __LINE__, __FILE__);
					return 0;
				}
				defTok = tokPtr;
				tokPtr++;
				while (tokPtr < _tokensEndPtr &&
					   tokPtr->end < defTok->end)
				{
					if (tokPtr->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
						return 0;
					}
					switch (_jsonMsg[tokPtr->start])
					{
					case 'i':	// ID (The set Identifier)
					{
						tokPtr++;
						if (tokPtr->type != JSMN_PRIMITIVE)
						{
							unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
							return 0;
						}
						setId = rtr_atoi_size( &_jsonMsg[tokPtr->start], &_jsonMsg[tokPtr->end]);
						if (setId > RSSL_ELEMENT_SET_MAX_LOCAL_ID)
						{
							error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
							return 0;
						}
						setDefDbPtr->definitions[setId].setId = setId;
						tokPtr++;
						break;
					}
					case 'c':	// Count (Number of Fields in the definiton)
					{
						tokPtr++;
						if (tokPtr->type != JSMN_PRIMITIVE)
						{
							unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
							return 0;
						}
						setDefDbPtr->definitions[setId].count = rtr_atoi_size( &_jsonMsg[tokPtr->start],
																				   &_jsonMsg[tokPtr->end]);
						tokPtr++;
						break;
					}

					case 's':	// The array of set definitions
					{
						tokPtr++;
						if (tokPtr->type != JSMN_ARRAY)
						{
							unexpectedTokenType(JSMN_ARRAY, tokPtr, __LINE__, __FILE__);
							return 0;
						}
						setTok = tokPtr;
						tokPtr++;
						for (j = 0; j < setTok->size; j++)
						{
							if (tokPtr->type != JSMN_OBJECT)
							{
								unexpectedTokenType(JSMN_OBJECT, tokPtr, __LINE__, __FILE__);
								return 0;
							}
							tokPtr++;
							for (k = 0; k < 2; k++)
							{
								if (tokPtr->type != JSMN_STRING)
								{
									unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
									return 0;
								}
								switch (_jsonMsg[tokPtr->start])
								{
								case 'n':	// Element Name
								{
									tokPtr++;
									if (tokPtr->type != JSMN_STRING)
									{
										unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
										return 0;
									}
									setDefDbPtr->definitions[setId].pEntries[j].name.data = &_jsonMsg[tokPtr->start];
									setDefDbPtr->definitions[setId].pEntries[j].name.length = tokPtr->end - tokPtr->start;
									tokPtr++;
									break;
								}
								case 't':	// Type
								{
									tokPtr++;
									if (tokPtr->type != JSMN_PRIMITIVE)
									{
										unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
										return 0;
									}
									setDefDbPtr->definitions[setId].pEntries[j].dataType =
											rtr_atoi_size( &_jsonMsg[tokPtr->start],
													   &_jsonMsg[tokPtr->end]);
									tokPtr++;
									break;
								}
								default:
									unexpectedParameter(tokPtr, __LINE__, __FILE__);
									return 0;
								}
							} // End of (for k = 0...)
						} // End of (for j = 0...)
						break;
					}	// End of case 's':
					default:
						unexpectedParameter(tokPtr, __LINE__, __FILE__);
						return 0;
					}
				}
			}
			tokPtr++;
			break;
		}	// End of case 'd'
		default:
			unexpectedParameter(tokPtr, __LINE__, __FILE__);
			return 0;
		}
	}
	return setDefDbPtr;
}

RsslLocalFieldSetDefDb* jsonToRwfConverter::processFieldSetDef(jsmntok_t *tokPtr)
{
	RsslLocalFieldSetDefDb* setDefDbPtr = getFieldSetDefDb();
	jsmntok_t *setDefTok = tokPtr;
	jsmntok_t *dataTok;
	jsmntok_t *defTok;
	jsmntok_t *setTok;
	int setId;



	int setDefCount = 0;
	int i,j,k;

	tokPtr++;
	while (tokPtr < _tokensEndPtr &&
		   tokPtr->end < setDefTok->end)
	{
		if (tokPtr->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
			return 0;
		}

		switch (_jsonMsg[tokPtr->start])
		{
		case 'c':	// Count (Number of Set Defiinitions)
		{
			tokPtr++;
			if (tokPtr->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
				return 0;
			}
			setDefCount = rtr_atoi_size( &_jsonMsg[tokPtr->start],
								 &_jsonMsg[tokPtr->end]);
			tokPtr++;
			break;
		}
		case 'd':	// The array of set definitions
		{
			tokPtr++;
			if (tokPtr->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, tokPtr, __LINE__, __FILE__);
				return 0;
			}
			dataTok = tokPtr;
			tokPtr++;
			for (i = 0; i < dataTok->size; i++)
			{
				if (tokPtr->type != JSMN_OBJECT)
				{
					unexpectedTokenType(JSMN_OBJECT, tokPtr, __LINE__, __FILE__);
					return 0;
				}
				defTok = tokPtr;
				tokPtr++;
				while (tokPtr < _tokensEndPtr &&
					   tokPtr->end < defTok->end)
				{
					if (tokPtr->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
						return 0;
					}
					switch (_jsonMsg[tokPtr->start])
					{
					case 'i':	// ID (The set Identifier)
					{
						tokPtr++;
						if (tokPtr->type != JSMN_PRIMITIVE)
						{
							unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
							return 0;
						}
						setId = rtr_atoi_size( &_jsonMsg[tokPtr->start],
												   &_jsonMsg[tokPtr->end]);
						if (setId > RSSL_FIELD_SET_MAX_LOCAL_ID)
						{
							error(SET_DEFINITION_ERROR, __LINE__, __FILE__);
							return 0;
						}
						setDefDbPtr->definitions[setId].setId = setId;
						tokPtr++;
						break;
					}
					case 'c':	// Count (Number of Fields in the definiton)
					{
						tokPtr++;
						if (tokPtr->type != JSMN_PRIMITIVE)
						{
							unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
							return 0;
						}
						setDefDbPtr->definitions[setId].count = rtr_atoi_size( &_jsonMsg[tokPtr->start],
																				   &_jsonMsg[tokPtr->end]);
						tokPtr++;
						break;
					}

					case 's':	// The array of set definitions
					{
						tokPtr++;
						if (tokPtr->type != JSMN_ARRAY)
						{
							unexpectedTokenType(JSMN_ARRAY, tokPtr, __LINE__, __FILE__);
							return 0;
						}
						setTok = tokPtr;
						tokPtr++;
						for (j = 0; j < setTok->size; j++)
						{
							if (tokPtr->type != JSMN_OBJECT)
							{
								unexpectedTokenType(JSMN_OBJECT, tokPtr, __LINE__, __FILE__);
								return 0;
							}
							tokPtr++;
							for (k = 0; k < 2; k++)
							{
								if (tokPtr->type != JSMN_STRING)
								{
									unexpectedTokenType(JSMN_STRING, tokPtr, __LINE__, __FILE__);
									return 0;
								}
								switch (_jsonMsg[tokPtr->start])
								{
								case 'f':	// Field Id
								{
									tokPtr++;
									if (tokPtr->type != JSMN_PRIMITIVE)
									{
										unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
									return 0;
									}
										setDefDbPtr->definitions[setId].pEntries[j].fieldId =
											rtr_atoi_size( &_jsonMsg[tokPtr->start],
														   &_jsonMsg[tokPtr->end]);
									tokPtr++;
									break;
								}
								case 't':	// Type
								{
									tokPtr++;
									if (tokPtr->type != JSMN_PRIMITIVE)
									{
										unexpectedTokenType(JSMN_PRIMITIVE, tokPtr, __LINE__, __FILE__);
										return 0;
									}
									setDefDbPtr->definitions[setId].pEntries[j].dataType =
											rtr_atoi_size( &_jsonMsg[tokPtr->start],
														   &_jsonMsg[tokPtr->end]);
									tokPtr++;
									break;
								}
								default:
									unexpectedParameter(tokPtr, __LINE__, __FILE__);
									return 0;
								}
							} // End of (for k = 0...)
						} // End of (for j = 0...)
						break;
					}	// End of case 's':
					default:
						unexpectedParameter(tokPtr, __LINE__, __FILE__);
						return 0;
					}
				}
			}
			tokPtr++;
			break;
		}	// End of case 'd'
		default:
			unexpectedParameter(tokPtr, __LINE__, __FILE__);
			return 0;
		}
	}
	return setDefDbPtr;
}

RsslBuffer* jsonToRwfConverter::errorText()
{
	if (_error == false)
		return 0;

	//memset(_errorText.data,'0',_errorTextLen);
	_errorText.length = 0;

	switch (_errorCode)
	{
		case MEM_ALLOC_FAILURE:
			_errorText.length = snprintf(_errorText.data,
										ERROR_TEXT_MAX, "JSON Converter memory allocation Error: %s Line %d",
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
				free((void*)singleToken);
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
