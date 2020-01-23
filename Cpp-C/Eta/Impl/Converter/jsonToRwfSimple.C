/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


#include <limits.h>
#include <math.h>
#ifdef Linux
#include <stdlib.h>
#endif

#include "rtr/rtratoi.h"
#include "rtr/rjcsmallstr.h"

#include "rtr/jsonToRwfSimple.h"
#include "rtr/jsonSimpleDefs.h"

jsonToRwfSimple::jsonToRwfSimple(int bufSize, unsigned int flags, RsslUInt16 srvcId, int numTokens, int incSize) :
	_defaultServiceId(srvcId),
	_viewTokPtr(0),
	_batchReqTokPtr(0),
	_batchCloseTokPtr(0),
	jsonToRwfBase(bufSize, flags, numTokens, incSize)
{
}

jsonToRwfSimple::~jsonToRwfSimple()
{
}

bool jsonToRwfSimple::encodeMsgPayload(RsslMsg *rsslMsgPtr, jsmntok_t *dataTokPtr)
{
	int msgLen = rsslGetEncodedBufferLength(&_iter);
	rsslMsgPtr->msgBase.encDataBody.data = _outBuf.data + msgLen;

	if (_viewTokPtr || _batchReqTokPtr || _batchCloseTokPtr)
	{
		if (!encodeBatchView(rsslMsgPtr))
		{
			rsslClearBuffer(&rsslMsgPtr->msgBase.encDataBody);
			return false;
		}
	}

	else if (dataTokPtr)
	{
		if (processContainer(&dataTokPtr, rsslMsgPtr->msgBase.containerType, 0) == false)
		{
			rsslClearBuffer(&rsslMsgPtr->msgBase.encDataBody);
			return false;
		}
	}
	rsslMsgPtr->msgBase.encDataBody.length = rsslGetEncodedBufferLength(&_iter) - msgLen ;
	return true;
}

bool jsonToRwfSimple::encodeBatchView(RsslMsg *rsslMsgPtr)
{
	void *voidPtr = 0;
	RsslBuffer *bufPtr = 0;
	RsslBuffer buf;
	RsslUInt u64;
	RsslInt i64;
	jsmntok_t *tok = 0;
	RsslArray array;
	RsslElementList el;
	RsslElementEntry ee = RSSL_INIT_ELEMENT_ENTRY;
	const RsslDictionaryEntry *def;


	rsslClearElementList(&el);
	rsslElementListApplyHasStandardData(&el);

	if ( (_rsslRet = rsslEncodeElementListInit(&_iter, &el, 0, 0)) < RSSL_RET_SUCCESS )
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	if (_batchReqTokPtr)
	{
		rsslClearElementEntry(&ee);
		ee.name = RSSL_ENAME_BATCH_ITEM_LIST;
		ee.dataType = RSSL_DT_ARRAY;
		if ((_rsslRet = rsslEncodeElementEntryInit(&_iter, &ee, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		rsslClearArray(&array);
		array.primitiveType = RSSL_DT_ASCII_STRING;
		if ((_rsslRet = rsslEncodeArrayInit(&_iter, &array)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		tok = _batchReqTokPtr;
		tok++;
		bufPtr = &buf;
		for (int i = 0; i < _batchReqTokPtr->size; i++)
		{
			if (processAsciiString(&tok, &bufPtr, &voidPtr) == false)
				return false;
			if ((_rsslRet = rsslEncodeArrayEntry(&_iter, &buf, voidPtr)) < RSSL_RET_SUCCESS)
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
		}
		if ((_rsslRet = rsslEncodeArrayComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		if ((_rsslRet = rsslEncodeElementEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}	// End of Batch Request

	if (_viewTokPtr)
	{
		// Assume RDM_VIEW_TYPE_FIELD_ID_LIST for now
		rsslClearElementEntry(&ee);
		ee.name = RSSL_ENAME_VIEW_TYPE;
		ee.dataType = RSSL_DT_UINT;
		u64 = RDM_VIEW_TYPE_FIELD_ID_LIST;
		if ((_rsslRet = rsslEncodeElementEntry(&_iter, &ee, &u64)) < RSSL_RET_SUCCESS)
		{
			// MJD error
			return false;
		}

		rsslClearElementEntry(&ee);
		ee.name = RSSL_ENAME_VIEW_DATA;
		ee.dataType = RSSL_DT_ARRAY;
		if ((_rsslRet = rsslEncodeElementEntryInit(&_iter, &ee, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		rsslClearArray(&array);
		array.primitiveType = RSSL_DT_INT;
		if ((_rsslRet = rsslEncodeArrayInit(&_iter, &array)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		tok = _viewTokPtr;
		// MJD move fidName here
		tok++;
		for (int i = 0; i < _viewTokPtr->size; i++)
		{
			switch (tok->type)
			{
			case JSMN_STRING:
				{
					// Look up by name
					RsslBuffer fidName = { (tok->end - tok->start), &_jsonMsg[tok->start]};
					if ((def = rsslDictionaryGetEntryByFieldName(_dictionaryList[0], &fidName)))
					{
						i64 = def->fid;

						if ((_rsslRet =rsslEncodeArrayEntry(&_iter, bufPtr, &i64)) < RSSL_RET_SUCCESS)
						{
							error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
							return false;
						}
					}
					tok++;
					break;
				}
			case JSMN_PRIMITIVE:
				{
					if (processInteger(&tok, &bufPtr, &voidPtr) == false)
						return false;

					if ((_rsslRet = rsslEncodeArrayEntry(&_iter, bufPtr, voidPtr)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			default:
				{
					unexpectedTokenType(JSMN_STRING, tok, __LINE__, __FILE__);
					return false;
				}
			}
		}
		if ((_rsslRet = rsslEncodeArrayComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		if ((_rsslRet = rsslEncodeElementEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	} // End of View

	if (_batchCloseTokPtr)
	{
		rsslClearElementEntry(&ee);
		ee.name = RSSL_ENAME_BATCH_STREAMID_LIST;
		ee.dataType = RSSL_DT_ARRAY;
		if ((_rsslRet = rsslEncodeElementEntryInit(&_iter, &ee, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		rsslClearArray(&array);
		array.primitiveType = RSSL_DT_INT;
		if ((_rsslRet = rsslEncodeArrayInit(&_iter, &array)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		tok = _batchCloseTokPtr;
		tok++;
		bufPtr = &buf;
		for (int i = 0; i < _batchCloseTokPtr->size; i++)
		{
			if (processInteger(&tok, &bufPtr, &voidPtr) == false)
				return false;
			// Using the first non-zero streamId being closed as the streamId
			// of the batch close request.
			if (rsslMsgPtr->msgBase.streamId == 0)
			{
				rsslReplaceStreamId(&_iter, *(RsslInt32*)voidPtr);
				rsslMsgPtr->msgBase.streamId = *(RsslInt32*)voidPtr;
			}
			if ((_rsslRet = rsslEncodeArrayEntry(&_iter, bufPtr, voidPtr)) < RSSL_RET_SUCCESS)
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}


		}
		if ((_rsslRet = rsslEncodeArrayComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}

		if ((_rsslRet = rsslEncodeElementEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}	// End of Batch Close

	if ((_rsslRet = rsslEncodeElementListComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool jsonToRwfSimple::processMessage(jsmntok_t ** const msgTok, RsslJsonMsg *jsonMsgPtr)
{
	jsmntok_t *tok = *msgTok;

	jsmntok_t *dataTokPtr = 0;
	jsmntok_t *attribTokPtr = 0;
	jsmntok_t *reqKeyattrib = 0;
	jsmntok_t *curMsgTok = *msgTok;
	RsslMsg *rsslMsgPtr = &jsonMsgPtr->jsonRsslMsg.rsslMsg;

	// MJD SIMPLIFIED defaults
	rsslMsgPtr->msgBase.msgClass = RSSL_MC_REQUEST;
	rsslMsgPtr->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	rsslMsgPtr->msgBase.containerType = RSSL_DT_NO_DATA;
	jsonMsgPtr->msgBase.msgClass = RSSL_JSON_MC_RSSL_MSG;

	tok++;
	while(tok < _tokensEndPtr && tok->end < curMsgTok->end)
	{
		if (tok->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, tok, __LINE__, __FILE__);
				return false;
		}
		switch (_jsonMsg[tok->start])
		{
		case 'I':
			{
				if (compareStrings(tok, JSON_ID))
				{
					tok++;
					if (tok->type == JSMN_PRIMITIVE)
					{
						rsslMsgPtr->msgBase.streamId =
							rtr_atoi_size( &_jsonMsg[tok->start],
										   &_jsonMsg[tok->end]);
						tok++;
					}
					else if (tok->type == JSMN_ARRAY)
					{

						// MJD - error if we already have a data
						// Assume for now that this is a batch close request
						// We'll error out later if it's not a close message
						rsslMsgPtr->msgBase.streamId = 0;
						rsslCloseMsgSetHasBatch(&rsslMsgPtr->closeMsg);
						_batchCloseTokPtr = dataTokPtr = tok;
						rsslMsgPtr->msgBase.containerType = RSSL_DT_ELEMENT_LIST;
						skipObject(&tok);
					}
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__, &JSON_ID);
						return false;
					}

				}
				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(tok, JSON_TYPE))
				{
					tok++;
					switch (tok->type)
					{
					case JSMN_STRING:
						{
							switch (_jsonMsg[tok->start])
							{
							case 'R':
								{
									if (compareStrings(tok, RSSL_OMMSTR_MC_REQUEST))
										rsslMsgPtr->msgBase.msgClass = RSSL_MC_REQUEST;
									else if (compareStrings(tok, RSSL_OMMSTR_MC_REFRESH))
										rsslMsgPtr->msgBase.msgClass = RSSL_MC_REFRESH;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
										return false;
									}
									break;
								}
							case 'C':
								{
									if (compareStrings(tok, RSSL_OMMSTR_MC_CLOSE))
										rsslMsgPtr->msgBase.msgClass = RSSL_MC_CLOSE;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
										return false;
									}
									break;
								}
							case 'S':
								{
									{
										if (compareStrings(tok, RSSL_OMMSTR_MC_STATUS))
											rsslMsgPtr->msgBase.msgClass = RSSL_MC_STATUS;
										else
										{
											unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
											return false;
										}
									}
									break;
								}
							case 'U':
								{
									if (compareStrings(tok, RSSL_OMMSTR_MC_UPDATE))
										rsslMsgPtr->msgBase.msgClass = RSSL_MC_UPDATE;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
										return false;
									}
									break;
								}
							case 'A':
								{
									if (compareStrings(tok, RSSL_OMMSTR_MC_ACK))
										rsslMsgPtr->msgBase.msgClass = RSSL_MC_ACK;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
										return false;
									}
									break;
								}
							case 'G':
								{
									if (compareStrings(tok, RSSL_OMMSTR_MC_GENERIC))
										rsslMsgPtr->msgBase.msgClass = RSSL_MC_GENERIC;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
										return false;
									}
									break;
								}
							case 'P':
								{
									if (compareStrings(tok, RSSL_OMMSTR_MC_POST))
										rsslMsgPtr->msgBase.msgClass = RSSL_MC_POST;
									else if (compareStrings(tok, JSON_PING))
										jsonMsgPtr->msgBase.msgClass = RSSL_JSON_MC_PING;
									else if (compareStrings(tok, JSON_PONG))
										jsonMsgPtr->msgBase.msgClass = RSSL_JSON_MC_PONG;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
										return false;
									}
									break;
								}
							case 'E':
							{
								if (compareStrings(tok, JSON_ERROR))
									jsonMsgPtr->msgBase.msgClass = RSSL_JSON_MC_ERROR;
								else
								{
									unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
									return false;
								}
								break;
							}
							default:
								{
									unexpectedParameter(tok, __LINE__, __FILE__, &JSON_TYPE);
									return false;
								}
							} // End of switch
							break;
						}  // End of case JSMN_STRING:
					case JSMN_PRIMITIVE:
						{
							rsslMsgPtr->msgBase.msgClass =
								rtr_atoi_size( &_jsonMsg[tok->start],
											   &_jsonMsg[tok->end]);

							break;
						}
					default:
						{
							unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__, &JSON_TYPE);
							return false;
						}
					}
					tok++;
				}
				else if (compareStrings(tok, JSON_TEXT))
				{
					tok++;
					skipObject(&tok);
				}
				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(tok, JSON_DOMAIN))
				{
					tok++;
					switch (tok->type)
					{
					case JSMN_STRING:
						{
							switch (_jsonMsg[tok->start])
							{
							case 'M':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_MARKET_PRICE))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_MARKET_BY_ORDER))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_MARKET_BY_ORDER;
									else if(compareStrings(tok, RSSL_OMMSTR_DMT_MARKET_BY_PRICE))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
									else if(compareStrings(tok, RSSL_OMMSTR_DMT_MARKET_MAKER))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_MARKET_MAKER;
									else if(compareStrings(tok, RSSL_OMMSTR_DMT_MARKET_BY_TIME))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_MARKET_BY_TIME;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'L':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_LOGIN))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_LOGIN;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'A':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_ANALYTICS))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_ANALYTICS;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'F':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_FORECAST))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_FORECAST;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'E':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_ECONOMIC_INDICATOR))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_ECONOMIC_INDICATOR;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'N':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_NEWS_TEXT_ANALYTICS))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_NEWS_TEXT_ANALYTICS;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'C':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_CONTRIBUTION))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_CONTRIBUTION;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'S':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_SOURCE))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_SOURCE;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_SYMBOL_LIST))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_SYMBOL_LIST;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_SERVICE_PROVIDER_STATUS))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_SERVICE_PROVIDER_STATUS;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_STORY))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_STORY;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_SYSTEM))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_SYSTEM;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'D':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_DICTIONARY))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_DICTIONARY;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'T':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_TRANSACTION))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_TRANSACTION;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'Y':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_YIELD_CURVE))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_YIELD_CURVE;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'P':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_PROVIDER_ADMIN))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_PROVIDER_ADMIN;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_POLL))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_POLL;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'R':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_REPLAYHEADLINE))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_REPLAYHEADLINE;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_REPLAYSTORY))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_REPLAYSTORY;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_REFERENCE))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_REFERENCE;

									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							case 'H':
								{
									if (compareStrings(tok, RSSL_OMMSTR_DMT_HISTORY))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_HISTORY;
									else if (compareStrings(tok, RSSL_OMMSTR_DMT_HEADLINE))
										rsslMsgPtr->msgBase.domainType = RSSL_DMT_HEADLINE;
									else
									{
										unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
										return false;
									}
									break;
								}
							default:
								{
									unexpectedParameter(tok, __LINE__, __FILE__, &JSON_DOMAIN);
									return false;
								}
							}
							// Just in Case :)
							break;
						}
					case JSMN_PRIMITIVE:
						{
							rsslMsgPtr->msgBase.domainType =
								rtr_atoi_size( &_jsonMsg[tok->start],
											   &_jsonMsg[tok->end]);
							break;
						}
					default:
						{
							unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__, &JSON_DOMAIN);
							return false;
						}
					}
					tok++;
				}
				else if (compareStrings(tok, JSON_DEBUG))
				{
					tok++;

					jsmntok_t *debugTok = tok;

					if (tok->type != JSMN_OBJECT)
						return false;

					tok++;

					while( tok < _tokensEndPtr 
							&& tok->end < debugTok->end)
					{
						if (tok->type != JSMN_STRING)
						{
							unexpectedTokenType(JSMN_STRING, tok, __LINE__, __FILE__, &JSON_DEBUG);
							return false;
						}
						switch (_jsonMsg[tok->start])
						{
						case 'F':
							{
								if (compareStrings(tok, JSON_FILE)) // File
								{
									tok++;
									if(tok->type != JSMN_STRING)
									{
										unexpectedTokenType(JSMN_STRING, tok, __LINE__, __FILE__, &JSON_FILE);
										return false;
									}
									tok++;
								}
								else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
								{
									unexpectedKey(tok, __LINE__, __FILE__, &JSON_KEY);
									return false;
								}
				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
						case 'L':
							{
								if (compareStrings(tok, JSON_LINE)) // LINE
								{
									tok++;
									if(tok->type != JSMN_PRIMITIVE)
									{
										unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__, &JSON_LINE);
										return false;
									}
									tok++;
								}
								else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
								{
									unexpectedKey(tok, __LINE__, __FILE__, &JSON_KEY);
									return false;
								}
								else
								{
									tok++;
									skipObject(&tok);
								}
								break;
							}
						case 'O':
							{
								if (compareStrings(tok, JSON_OFFSET)) // Offset
								{
									tok++;
									if(tok->type != JSMN_PRIMITIVE)
									{
										unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__, &JSON_OFFSET);
										return false;
									}
									tok++;
								}
								else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
								{
									unexpectedKey(tok, __LINE__, __FILE__, &JSON_KEY);
									return false;
								}
								else
								{
									tok++;
									skipObject(&tok);
								}
								break;
							}
						case 'M':
							{
								if (compareStrings(tok, JSON_MESSAGE)) // Message
								{
									tok++;
									if(tok->type != JSMN_STRING)
									{
										unexpectedTokenType(JSMN_STRING, tok, __LINE__, __FILE__, &JSON_MESSAGE);
										return false;
									}
									tok++;
								}
								else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
								{
									unexpectedKey(tok, __LINE__, __FILE__, &JSON_KEY);
									return false;
								}
								else
								{
									tok++;
									skipObject(&tok);
								}
								break;
							}
						default:
							{
								if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
								{
									unexpectedKey(tok, __LINE__, __FILE__, &JSON_KEY);
									return false;
								}
								else
								{
									tok++;
									skipObject(&tok);
								}
								break;
							}
						}
					}
				}
				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(tok, JSON_ELEMENTS))
				{
					if (dataTokPtr)
					{
						// MJD Error We've already found a container
						return false;
					}
					tok++;
					dataTokPtr = tok;
					rsslMsgPtr->msgBase.containerType = RSSL_DT_ELEMENT_LIST;
					skipObject(&tok);
				}
				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(tok, JSON_FIELDS))
				{
					if (dataTokPtr)
					{
						// MJD Error We've already found a container
						return false;
					}
					tok++;
					dataTokPtr = tok;
					rsslMsgPtr->msgBase.containerType = RSSL_DT_FIELD_LIST;
					skipObject(&tok);
				}
				else if (compareStrings(tok, JSON_FILTERLIST))
				{
					if (dataTokPtr)
					{
						// MJD Error We've already found a container
						return false;
					}
					tok++;
					dataTokPtr = tok;
					rsslMsgPtr->msgBase.containerType = RSSL_DT_FILTER_LIST;
					skipObject(&tok);
				}
				else
				{
					tok++;
					skipObject(&tok);
				}
			break;
		}
		case 'M':
			{
				if (compareStrings(tok, JSON_MAP))
				{
					if (dataTokPtr)
					{
						// MJD Error We've already found a container
						return false;
					}
					tok++;
					dataTokPtr = tok;
					rsslMsgPtr->msgBase.containerType = RSSL_DT_MAP;
					skipObject(&tok);
				}
				else if (compareStrings(tok, JSON_MESSAGE))
				{
					if (dataTokPtr)
					{
						// MJD Error We've already found a container
						return false;
					}
					tok++;
					dataTokPtr = tok;
					rsslMsgPtr->msgBase.containerType = RSSL_DT_MSG;
					skipObject(&tok);
				}

				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
		case 'S':
			{
				if (compareStrings(tok, JSON_SERIES))
				{
					if (dataTokPtr)
					{
						// MJD Error We've already found a container
						return false;
					}
					tok++;
					dataTokPtr = tok;
					rsslMsgPtr->msgBase.containerType = RSSL_DT_SERIES;
					skipObject(&tok);
				}
				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
		case 'J': // Json
			{
				if (compareStrings(tok, RSSL_OMMSTR_DT_JSON))
				{
					if (dataTokPtr)
					{
						// MJD Error We've already found a container
						return false;
					}
					tok++;
					dataTokPtr = tok;
					rsslMsgPtr->msgBase.containerType = RSSL_DT_JSON;
					skipObject(&tok);
				}
				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
		case 'V': // Vector
			{
				if (compareStrings(tok, RSSL_OMMSTR_DT_VECTOR))
				{
					if (dataTokPtr)
					{
						// MJD Error We've already found a container
						return false;
					}
					tok++;
					dataTokPtr = tok;
					rsslMsgPtr->msgBase.containerType = RSSL_DT_VECTOR;
					skipObject(&tok);
				}
				else
				{
					tok++;
					skipObject(&tok);
				}
				break;
			}
		case 'X': // Xml
		{
			if (compareStrings(tok, JSON_XML))
			{
			  if (dataTokPtr)
		{
			    // MJD Error We've already found a container
				return false;
			  }
			  tok++;
			  dataTokPtr = tok;
			  rsslMsgPtr->msgBase.containerType = RSSL_DT_XML;
			  skipObject(&tok);
			}
			else
			{
				tok++;
				skipObject(&tok);
			}
			break;
		}
		case 'O': // Opaque
		{
			if (compareStrings(tok, JSON_OPAQUE))
			{
			  if (dataTokPtr)
		{
			    // MJD Error We've already found a container
			return false;
		}
			  tok++;
			  dataTokPtr = tok;
			  rsslMsgPtr->msgBase.containerType = RSSL_DT_OPAQUE;
			  skipObject(&tok);
			}
			else
			{
				tok++;
				skipObject(&tok);
			}
			break;
		}
		default:
			{
				tok++;
				skipObject(&tok);
				break;
			}
		}
	}

	if(jsonMsgPtr->msgBase.msgClass == RSSL_JSON_MC_PING || jsonMsgPtr->msgBase.msgClass == RSSL_JSON_MC_PONG || jsonMsgPtr->msgBase.msgClass == RSSL_JSON_MC_ERROR) {
		skipObject(msgTok);
		return true;
	}

	return encodeRsslMsg(rsslMsgPtr, msgTok, dataTokPtr,attribTokPtr, reqKeyattrib);
}

bool jsonToRwfSimple::processRequestMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	bool hasKey = false;
	bool hasId = false;
	int ret = 0;
	//	int i = 0;
	// int prioritySize = (*tokPtr)->size;
	int size = 0;
	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	// Assume streaming (default)
	rsslRequestMsgApplyStreaming(&rsslMsgPtr->requestMsg);

	// Key in Updates is true by default expecting Login and Dictionary domains according to RDM Usage guide.
	if ( (rsslMsgPtr->requestMsg.msgBase.domainType != RSSL_DMT_LOGIN) && (rsslMsgPtr->requestMsg.msgBase.domainType != RSSL_DMT_DICTIONARY) )
	{
		rsslRequestMsgApplyMsgKeyInUpdates(&rsslMsgPtr->requestMsg);
	}

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
			case 'C':
			{
				if (compareStrings(*tokPtr, JSON_CONFINFOINUPDATES))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_CONFINFOINUPDATES);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->requestMsg.flags |= RSSL_RQMF_CONF_INFO_IN_UPDATES;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'I':
			{
				if (compareStrings(*tokPtr, JSON_ID))
				{
					hasId = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TYPE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DOMAIN))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (compareStrings(*tokPtr, JSON_EXTHDR))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_EXTHDR);
						return false;
					}
					bufPtr = &rsslMsgPtr->requestMsg.extendedHeader;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslRequestMsgApplyHasExtendedHdr(&rsslMsgPtr->requestMsg);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_FIELDS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_FILTERLIST))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'M':
			{
				if (compareStrings(*tokPtr, JSON_MAP))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_MESSAGE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'J':
			{
				if (compareStrings(*tokPtr, RSSL_OMMSTR_DT_JSON))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'K':
			{
				if (compareStrings(*tokPtr, JSON_KEY))
				{
					hasKey = true;
					(*tokPtr)++;
					if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
					if(_batchReqTokPtr)
					{
						// The key contained an array of item names
						// We'll add these to the message payload when we encode the message.
						rsslMsgPtr->msgBase.containerType = RSSL_DT_ELEMENT_LIST;
						*dataPtr = _batchReqTokPtr;
						rsslRequestMsgApplyHasBatch(&rsslMsgPtr->requestMsg);
					}
				}
				else if (compareStrings(*tokPtr, JSON_KEYINUPDATES))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_KEYINUPDATES);
						return false;
					}
					if (!isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->requestMsg.flags &= ~RSSL_RQMF_MSG_KEY_IN_UPDATES;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'V':
			{
				if (compareStrings(*tokPtr, JSON_VIEW))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_ARRAY)
					{
						unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__, &JSON_VIEW);
						return false;
					}
					// Save the view start token index and move past
					rsslMsgPtr->msgBase.containerType = RSSL_DT_ELEMENT_LIST;
					rsslRequestMsgApplyHasView(&rsslMsgPtr->requestMsg);
					// MJD - error if data or view already set ???
					_viewTokPtr = *tokPtr;
					*dataPtr = _viewTokPtr;
					skipObject(tokPtr);
				}
				else if (compareStrings(*tokPtr, JSON_VECTOR))
				{
					   (*tokPtr)++;
					   skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'S':
			{
				if (compareStrings(*tokPtr, JSON_STREAMING))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_STREAMING);
						return false;
					}
					if (!isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->requestMsg.flags &= ~RSSL_RQMF_STREAMING;
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_SERIES))
				{
					   (*tokPtr)++;
					   skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'R':
			{
				if (compareStrings(*tokPtr, JSON_REFRESH))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_REFRESH);
						return false;
					}
					if (!isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->requestMsg.flags |= RSSL_RQMF_NO_REFRESH;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'P':
			{
				if (compareStrings(*tokPtr, JSON_PRIVATE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_PRIVATE);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslRequestMsgApplyPrivateStream(&rsslMsgPtr->requestMsg);
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_PAUSE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_PAUSE);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->requestMsg.flags |= RSSL_RQMF_PAUSE;
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_PRIORITY))
				{
					bool hasClass = false;
					bool hasCount = false;
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_PRIORITY);
						return false;
					}
					int prioritySize = (*tokPtr)->size;
					(*tokPtr)++;
					for (int i = 0; i <  prioritySize; i+=2)
					{
						if ((*tokPtr)->type != JSMN_STRING)
						{
							unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_PRIORITY);
							return false;
						}
						if (compareStrings(*tokPtr, JSON_COUNT))
						{
							hasCount = true;
							(*tokPtr)++;
							if ((*tokPtr)->type != JSMN_PRIMITIVE)
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_COUNT);
								return false;
							}
							rsslMsgPtr->requestMsg.priorityCount =
								rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
											   &_jsonMsg[(*tokPtr)->end]);
							rsslRequestMsgApplyHasPriority(&rsslMsgPtr->requestMsg);
							(*tokPtr)++;

						}
						else if (compareStrings(*tokPtr, JSON_CLASS))
						{
							hasClass = true;
							(*tokPtr)++;
							if ((*tokPtr)->type != JSMN_PRIMITIVE)
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_CLASS);
								return false;
							}
							rsslMsgPtr->requestMsg.priorityClass =
								rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
											   &_jsonMsg[(*tokPtr)->end]);
							rsslRequestMsgApplyHasPriority(&rsslMsgPtr->requestMsg);
							(*tokPtr)++;
						}
						else
						{
							unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_PRIORITY);
							return false;
						}
					}
					if(!hasCount) {
						missingKey(JSON_COUNT, __LINE__, __FILE__, &JSON_PRIORITY);
						return false;
				}
					if(!hasClass) {
						missingKey(JSON_CLASS, __LINE__, __FILE__, &JSON_PRIORITY);
						return false;
					}
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'Q':
			{
				if (compareStrings(*tokPtr, JSON_QOS))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_QOS);
						return false;
					}
					if (!populateQos(tokPtr, &rsslMsgPtr->requestMsg.qos))
					{
						return false;
					}
					rsslRequestMsgApplyHasQos(&rsslMsgPtr->requestMsg);
				}
				else if (compareStrings(*tokPtr, JSON_QUALIFIED))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_QUALIFIED);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->requestMsg.flags |= RSSL_RQMF_QUALIFIED_STREAM;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'W':
			{
				if (compareStrings(*tokPtr, JSON_WORSTQOS))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_WORSTQOS);
						return false;
					}
					if (!populateQos(tokPtr, &rsslMsgPtr->requestMsg.worstQos))
					{
						return false;
					}
					rsslRequestMsgApplyHasWorstQos(&rsslMsgPtr->requestMsg);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'X':
		  {
		    if (compareStrings(*tokPtr, JSON_XML))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		case 'O':
		  {
		    if (compareStrings(*tokPtr, JSON_OPAQUE))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
						return false;
					}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		}  // End of Switch
	}	// End of While


	if(!hasKey) {
		missingKey(JSON_KEY, __LINE__, __FILE__);
		return false;
	}
	if(!hasId) {
		missingKey(JSON_ID, __LINE__, __FILE__);
		return false;
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
	}

	return true;
}

bool jsonToRwfSimple::processRefreshMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	bool hasId = false;
	bool hasType = false;

	// Flag defaults
	rsslMsgPtr->refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_CLEAR_CACHE;

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	// Default State to OK/OPEN
	rsslMsgPtr->refreshMsg.state.dataState =  RSSL_DATA_OK;
	rsslMsgPtr->refreshMsg.state.streamState =  RSSL_STREAM_OPEN;

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
		case 'I':
			{
				if (compareStrings(*tokPtr, JSON_ID))
				{
					hasId = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TYPE))
				{
					hasType = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DOMAIN))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (compareStrings(*tokPtr, JSON_DONOTCACHE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_DONOTCACHE);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_DO_NOT_CACHE;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_FIELDS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_FILTERLIST))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'M':
			{
				if (compareStrings(*tokPtr, JSON_MAP))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_MESSAGE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'J':
			{
				if (compareStrings(*tokPtr, RSSL_OMMSTR_DT_JSON))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'K':
			{
				if (compareStrings(*tokPtr, JSON_KEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_KEY);
						return false;
					}
					if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
					rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_HAS_MSG_KEY;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_EXTHDR))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_EXTHDR);
						return false;
					}
					bufPtr = &rsslMsgPtr->refreshMsg.extendedHeader;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslRefreshMsgApplyHasExtendedHdr(&rsslMsgPtr->refreshMsg);
				}
				else if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'P':
			{
				if (compareStrings(*tokPtr, JSON_POSTUSERINFO))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
						return false;
					}
					if (!populatePostUserInfo(tokPtr, &rsslMsgPtr->refreshMsg.postUserInfo))
						return false;
					rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_HAS_POST_USER_INFO;
				}
				else if (compareStrings(*tokPtr, JSON_PERMDATA))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_PERMDATA);
						return false;
					}
					bufPtr = &rsslMsgPtr->refreshMsg.permData;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_HAS_PERM_DATA;
				}
				else if (compareStrings(*tokPtr, JSON_PRIVATE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_PRIVATE);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_PRIVATE_STREAM;
					}
					(*tokPtr)++;
				}
                else if (compareStrings(*tokPtr, JSON_PARTNUMBER))
                {
                    (*tokPtr)++;
                    if((*tokPtr)->type != JSMN_PRIMITIVE)
                    {
					    unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
						return false;
                    }
                    rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_HAS_PART_NUM;
					rsslMsgPtr->refreshMsg.partNum = 
							rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
										   &_jsonMsg[(*tokPtr)->end]);

					(*tokPtr)++;

                }
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'S':
			{
				if (compareStrings(*tokPtr, JSON_STATE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_STATE);
						return false;
					}
					if (!populateState(tokPtr, &rsslMsgPtr->refreshMsg.state))
						return false;
				}
				else if (compareStrings(*tokPtr, JSON_SEQNUM))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_SEQNUM);
						return false;
					}
					rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_HAS_SEQ_NUM;
					rsslMsgPtr->refreshMsg.seqNum =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									   &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_SOLICITED))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_SOLICITED);
						return false;
					}
					if (!isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_SERIES))
				{
					   (*tokPtr)++;
					   skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'C':
			{
				if (compareStrings(*tokPtr, JSON_COMPLETE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_COMPLETE);
						return false;
					}
					if (!isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->refreshMsg.flags &= ~RSSL_RFMF_REFRESH_COMPLETE;
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_CLEARCACHE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_CLEARCACHE);
						return false;
					}
					if (!isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->refreshMsg.flags &= ~RSSL_RFMF_CLEAR_CACHE;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'Q':
			{
				if (compareStrings(*tokPtr, JSON_QOS))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_QOS);
						return false;
					}

					if (!populateQos(tokPtr, &rsslMsgPtr->refreshMsg.qos))
					{
						return false;
					}
					rsslRefreshMsgApplyHasQos(&rsslMsgPtr->refreshMsg);
				}
				else if (compareStrings(*tokPtr, JSON_QUALIFIED))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_QUALIFIED);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_QUALIFIED_STREAM;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'R':
			{
				if (compareStrings(*tokPtr, JSON_REQKEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_REQKEY);
						return false;
					}
					if (processKey(tokPtr, &rsslMsgPtr->refreshMsg.reqMsgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
					rsslMsgPtr->refreshMsg.flags |= RSSL_RFMF_HAS_REQ_MSG_KEY;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'V': // Vector
		{
			 if (compareStrings(*tokPtr, JSON_VECTOR))
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			 {
				   unexpectedKey(*tokPtr, __LINE__, __FILE__);
				   return false;
			 }
			 else
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 break;
		}
		case 'X':
		  {
		    if (compareStrings(*tokPtr, JSON_XML))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		case 'O':
		  {
		    if (compareStrings(*tokPtr, JSON_OPAQUE))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
						return false;
					}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		}  // End of switch (_jsonMsg[(*tokPtr)->start])
	}	// End of while

	if(!hasId) {
		missingKey(JSON_ID, __LINE__, __FILE__);
		return false;
	}
	if(!hasType) {
		missingKey(JSON_TYPE, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool jsonToRwfSimple::processStatusMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	// Flag defaults
	// None

	bool hasId = false;
	bool hasType = false;

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
		case 'I':
			{
				if (compareStrings(*tokPtr, JSON_ID))
				{
					hasId = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TYPE))
				{
					hasType = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DOMAIN))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_FIELDS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_FILTERLIST))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'M':
			{
				if (compareStrings(*tokPtr, JSON_MAP))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_MESSAGE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'J':
			{
				if (compareStrings(*tokPtr, RSSL_OMMSTR_DT_JSON))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'K':
			{
				if (compareStrings(*tokPtr, JSON_KEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_KEY);
						return false;
					}
					if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
					rsslMsgPtr->statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'R':
			{
				if (compareStrings(*tokPtr, JSON_REQKEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_REQKEY);
						return false;
					}
					if (processKey(tokPtr, &rsslMsgPtr->statusMsg.reqMsgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
					rsslMsgPtr->statusMsg.flags |= RSSL_STMF_HAS_REQ_MSG_KEY;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_EXTHDR))
				{
					(*tokPtr)++;

					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_EXTHDR);
						return false;
					}
					bufPtr = &rsslMsgPtr->statusMsg.extendedHeader;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->statusMsg.flags |= RSSL_STMF_HAS_EXTENDED_HEADER;
				}
				else if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'P':
			{
				if (compareStrings(*tokPtr, JSON_POSTUSERINFO))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
						return false;
					}
					if (!populatePostUserInfo(tokPtr, &rsslMsgPtr->statusMsg.postUserInfo))
						return false;
					rsslMsgPtr->statusMsg.flags |= RSSL_STMF_HAS_POST_USER_INFO;
				}
				else if (compareStrings(*tokPtr, JSON_PERMDATA))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_PERMDATA);
						return false;
					}
					bufPtr = &rsslMsgPtr->statusMsg.permData;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->statusMsg.flags |= RSSL_STMF_HAS_PERM_DATA;
				}
				else if (compareStrings(*tokPtr, JSON_PRIVATE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_PRIVATE);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->statusMsg.flags |= RSSL_STMF_PRIVATE_STREAM;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'S':
			{
				if (compareStrings(*tokPtr, JSON_STATE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_STATE);
						return false;
					}
					if (!populateState(tokPtr, &rsslMsgPtr->statusMsg.state))
						return false;
					rsslMsgPtr->statusMsg.flags |= RSSL_STMF_HAS_STATE;
				}
				else if (compareStrings(*tokPtr, JSON_SERIES))
				{
				   (*tokPtr)++;
				   skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'C':
			{
				if (compareStrings(*tokPtr, JSON_CLEARCACHE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_CLEARCACHE);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->statusMsg.flags |= RSSL_STMF_CLEAR_CACHE;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'Q':
			{
				if (compareStrings(*tokPtr, JSON_QUALIFIED))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_QUALIFIED);
						return false;
					}

					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->statusMsg.flags |= RSSL_STMF_QUALIFIED_STREAM;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'V': // Vector
		{
			 if (compareStrings(*tokPtr, JSON_VECTOR))
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			 {
				   unexpectedKey(*tokPtr, __LINE__, __FILE__);
				   return false;
			 }
			 else
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 break;
		}
		case 'X':
		  {
		    if (compareStrings(*tokPtr, JSON_XML))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		case 'O':
		  {
		    if (compareStrings(*tokPtr, JSON_OPAQUE))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
				(*tokPtr)++;
				skipObject(tokPtr);
				}
				break;
			}
		}
	}

	if(!hasId) {
		missingKey(JSON_ID, __LINE__, __FILE__);
		return false;
	}
	if(!hasType) {
		missingKey(JSON_TYPE, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool jsonToRwfSimple::processUpdateMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	// Flag defaults
	// None

	bool hasId = false;
	bool hasType = false;

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
		case 'I':
			{
				if (compareStrings(*tokPtr, JSON_ID))
				{
					hasId = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TYPE))
				{
					hasType = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_FIELDS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_FILTERLIST))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'M':
			{
				if (compareStrings(*tokPtr, JSON_MAP))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_MESSAGE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'J':
			{
				if (compareStrings(*tokPtr, RSSL_OMMSTR_DT_JSON))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'K':
			{
				if (compareStrings(*tokPtr, JSON_KEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_KEY);
						return false;
					}
					if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
					rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_HAS_MSG_KEY;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_EXTHDR))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_EXTHDR);
						return false;
					}
					bufPtr = &rsslMsgPtr->updateMsg.extendedHeader;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_HAS_EXTENDED_HEADER;

				}
				else if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'P':
			{
				if (compareStrings(*tokPtr, JSON_POSTUSERINFO))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
						return false;
					}
					if (!populatePostUserInfo(tokPtr, &rsslMsgPtr->updateMsg.postUserInfo))
						return false;
					rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_HAS_POST_USER_INFO;
				}
				else if (compareStrings(*tokPtr, JSON_PERMDATA))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_PERMDATA);
						return false;
					}
					bufPtr = &rsslMsgPtr->updateMsg.permData;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_HAS_PERM_DATA;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'S':
			{
				if (compareStrings(*tokPtr, JSON_SEQNUM))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_SEQNUM);
						return false;
					}

					rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_HAS_SEQ_NUM;
					rsslMsgPtr->updateMsg.seqNum =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									   &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_SERIES))
				{
				   (*tokPtr)++;
				   skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'C':
			{
				if (compareStrings(*tokPtr, JSON_CONFINFO))
				{
					bool hasCount = false;
					bool hasTime = false;

					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_CONFINFO);
						return false;
					}

					rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_HAS_CONF_INFO;
					jsmntok_t *confTok = *tokPtr;
					(*tokPtr)++;

					while( (*tokPtr) <  _tokensEndPtr &&
							(*tokPtr)->end < confTok->end)
					{
						if ((*tokPtr)->type != JSMN_STRING)
						{
							unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_CONFINFO);
							return false;
						}
						if (compareStrings(*tokPtr, JSON_COUNT))
						{
							hasCount = true;
							(*tokPtr)++;
							if ((*tokPtr)->type != JSMN_PRIMITIVE)
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_CONFINFO);
								return false;
							}

							rsslMsgPtr->updateMsg.conflationCount =
								rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
											  &_jsonMsg[(*tokPtr)->end]);
							(*tokPtr)++;
						}
						if (compareStrings(*tokPtr, JSON_TIME))
						{
							hasTime = true;
							(*tokPtr)++;
							if ((*tokPtr)->type != JSMN_PRIMITIVE)
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_CONFINFO);
								return false;
							}

							rsslMsgPtr->updateMsg.conflationTime =
								rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
											  &_jsonMsg[(*tokPtr)->end]);
							(*tokPtr)++;
						}
					}
					if(!hasCount) {
						missingKey(JSON_COUNT, __LINE__, __FILE__, &JSON_CONFINFO);
						return false;
				}
					if(!hasTime) {
						missingKey(JSON_TIME, __LINE__, __FILE__, &JSON_CONFINFO);
						return false;
					}
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DONOTCONFLATE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_DONOTCONFLATE);
						return false;
					}

					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_DO_NOT_CONFLATE;
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_DONOTCACHE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_DONOTCACHE);
						return false;
					}

					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_DO_NOT_CACHE;
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_DONOTRIPPLE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_DONOTRIPPLE);
						return false;
					}

					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_DO_NOT_RIPPLE;
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_DISCARDABLE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_DISCARDABLE);
						return false;
					}

					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->updateMsg.flags |= RSSL_UPMF_DISCARDABLE;
					}
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_DOMAIN))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'U':
			{
				if (compareStrings(*tokPtr, JSON_UPDATETYPE))
				{
					(*tokPtr)++;
					switch ((*tokPtr)->type)
					{
					case JSMN_PRIMITIVE:
						{
							rsslMsgPtr->updateMsg.updateType =
								rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
											  &_jsonMsg[(*tokPtr)->end]);
							(*tokPtr)++;
							break;
						}
					case JSMN_STRING:
						{
							if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_QUOTE))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_TRADE))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_TRADE;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_UNSPECIFIED))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_UNSPECIFIED;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_NEWS_ALERT))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_NEWS_ALERT;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_VOLUME_ALERT))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_VOLUME_ALERT;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_ORDER_INDICATION))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_ORDER_INDICATION;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_CLOSING_RUN))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_CLOSING_RUN;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_CORRECTION))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_CORRECTION;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_MARKET_DIGEST))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_MARKET_DIGEST;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_QUOTES_TRADE))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTES_TRADE;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_MULTIPLE))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_MULTIPLE;
							else if (compareStrings(*tokPtr, RDM_OMMSTR_UPD_EVENT_TYPE_VERIFY))
								rsslMsgPtr->updateMsg.updateType = RDM_UPD_EVENT_TYPE_VERIFY;
							else
							{
								unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_UPDATETYPE);
								return false;
							}
							(*tokPtr)++;
							break;
						}
					default:
						{
							unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_UPDATETYPE);
							return false;
						}
					}
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'V': // Vector
		{
			 if (compareStrings(*tokPtr, JSON_VECTOR))
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			 {
				   unexpectedKey(*tokPtr, __LINE__, __FILE__);
				   return false;
			 }
			 else
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 break;
		}
		case 'X':
		  {
		    if (compareStrings(*tokPtr, JSON_XML))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		case 'O':
		  {
		    if (compareStrings(*tokPtr, JSON_OPAQUE))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
				(*tokPtr)++;
				skipObject(tokPtr);
				}
				break;
			}
		}  // End of switch (_jsonMsg[(*tokPtr)->start])
	}	// End of while

	if(!hasId) {
		missingKey(JSON_ID, __LINE__, __FILE__);
		return false;
	}
	if(!hasType) {
		missingKey(JSON_TYPE, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool jsonToRwfSimple::processCloseMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	int ret = 0;
	RsslBuffer *bufPtr;
	jsmntok_t *msgTok = *tokPtr;

	bool hasId = false;
	bool hasType = false;

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
		return false;
	}

	// Set close batch if streamId was found to be an array in processMessage()
	if (_batchCloseTokPtr)
	{
		rsslCloseMsgSetHasBatch(&rsslMsgPtr->closeMsg);
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
		case 'I':
			{
				if (compareStrings(*tokPtr, JSON_ID))
				{
					hasId = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TYPE))
				{
					hasType = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DOMAIN))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_FIELDS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (compareStrings(*tokPtr, JSON_FILTERLIST))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'M':
			{
				if (compareStrings(*tokPtr, JSON_MAP))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (compareStrings(*tokPtr, JSON_MESSAGE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'J':
			{
				if (compareStrings(*tokPtr, RSSL_OMMSTR_DT_JSON))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'A':
			{
				if (compareStrings(*tokPtr, JSON_ACK))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->closeMsg.flags |= RSSL_CLMF_ACK;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (compareStrings(*tokPtr, JSON_EXTHDR))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_EXTHDR);
						return false;
					}
					bufPtr = &rsslMsgPtr->closeMsg.extendedHeader;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->closeMsg.flags |= RSSL_CLMF_HAS_EXTENDED_HEADER;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'S': // Series
		{
			 if (compareStrings(*tokPtr, JSON_SERIES))
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			 {
				   unexpectedKey(*tokPtr, __LINE__, __FILE__);
				   return false;
			 }
			 else
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 break;
		}
		case 'V': // Vector
		{
			 if (compareStrings(*tokPtr, JSON_VECTOR))
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			 {
				   unexpectedKey(*tokPtr, __LINE__, __FILE__);
				   return false;
			 }
			 else
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 break;
		}
		case 'X':
		  {
		    if (compareStrings(*tokPtr, JSON_XML))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		case 'O':
		  {
		    if (compareStrings(*tokPtr, JSON_OPAQUE))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
				(*tokPtr)++;
				skipObject(tokPtr);
			}
		}
	}
	}	// End of While

	if(!hasId) {
		missingKey(JSON_ID, __LINE__, __FILE__);
		return false;
	}
	if(!hasType) {
		missingKey(JSON_TYPE, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool jsonToRwfSimple::processAckMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	bool hasAckId = false;
	bool hasId = false;
	bool hasType = false;

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
		case 'I':
			{
				if (compareStrings(*tokPtr, JSON_ID))
				{
					hasId = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TYPE))
				{
					hasType = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (compareStrings(*tokPtr, JSON_TEXT))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_TEXT);
						return false;
					}
					bufPtr = &rsslMsgPtr->ackMsg.text;
					if (processAsciiString(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->ackMsg.flags |= RSSL_AKMF_HAS_TEXT;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DOMAIN))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_FIELDS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_FILTERLIST))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'M':
			{
				if (compareStrings(*tokPtr, JSON_MAP))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_MESSAGE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'J':
			{
				if (compareStrings(*tokPtr, RSSL_OMMSTR_DT_JSON))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'K':
			{
				if (compareStrings(*tokPtr, JSON_KEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_KEY);
						return false;
					}
					rsslMsgPtr->ackMsg.flags |= RSSL_AKMF_HAS_MSG_KEY;
					if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'S':
			{
				if (compareStrings(*tokPtr, JSON_SEQNUM))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_SEQNUM);
						return false;
					}
					rsslMsgPtr->ackMsg.flags |= RSSL_AKMF_HAS_SEQ_NUM;
					rsslMsgPtr->ackMsg.seqNum =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									   &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_SERIES))
				{
				   (*tokPtr)++;
				   skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'A':
			{
				if (compareStrings(*tokPtr, JSON_ACKID))
				{
					hasAckId = true;
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_ACKID);
						return false;
					}
					rsslMsgPtr->ackMsg.ackId =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									   &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'P':
			{
				if (compareStrings(*tokPtr, JSON_PRIVATE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_PRIVATE);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->ackMsg.flags |= RSSL_AKMF_PRIVATE_STREAM;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'Q':
			{
				if (compareStrings(*tokPtr, JSON_QUALIFIED))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_QUALIFIED);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->ackMsg.flags |= RSSL_AKMF_QUALIFIED_STREAM;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_EXTHDR))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_EXTHDR);
						return false;
					}
					bufPtr = &rsslMsgPtr->ackMsg.extendedHeader;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->ackMsg.flags |= RSSL_AKMF_HAS_EXTENDED_HEADER;
				}
				else if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
						return false;
					}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'N':
			{
				if (compareStrings(*tokPtr, JSON_NAKCODE))
				{
					rsslMsgPtr->ackMsg.flags |= RSSL_AKMF_HAS_NAK_CODE;
					(*tokPtr)++;
					if ((*tokPtr)->type == JSMN_STRING)
					{
						switch (_jsonMsg[(*tokPtr)->start])
						{
						case 'N':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_NONE))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_NONE;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_NO_RESOURCES))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_NO_RESOURCES;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_NO_RESPONSE))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_NO_RESPONSE;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_NOT_OPEN))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_NOT_OPEN;
								break;
							}
						case 'A':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_ACCESS_DENIED))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_ACCESS_DENIED;
								break;
							}
						case 'D':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_DENIED_BY_SRC))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_DENIED_BY_SRC;
								break;
							}
						case 'S':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_SOURCE_DOWN))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_SOURCE_DOWN;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_SOURCE_UNKNOWN))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_SOURCE_UNKNOWN;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_SYMBOL_UNKNOWN))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_SYMBOL_UNKNOWN;
								break;
							}
						case 'G':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_GATEWAY_DOWN))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_GATEWAY_DOWN;
								break;
							}
						case 'I':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_NAKC_INVALID_CONTENT))
									rsslMsgPtr->ackMsg.nakCode = RSSL_NAKC_INVALID_CONTENT;
								break;
							}
						default:
							{
								rsslMsgPtr->ackMsg.nakCode = 0;
								break;
							}
						}
					}
					else if ((*tokPtr)->type == JSMN_PRIMITIVE)
					{
						if (_jsonMsg[(*tokPtr)->start] != 'n' &&
							_jsonMsg[(*tokPtr)->start] != 't' &&
							_jsonMsg[(*tokPtr)->start] != 'f')
							rsslMsgPtr->ackMsg.nakCode =
								rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
											   &_jsonMsg[(*tokPtr)->end]);
						else
							rsslMsgPtr->ackMsg.nakCode = 0;
					}
					else
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_NAKCODE);
						return false;
					}
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
				}
				(*tokPtr)++;
				break;
			}
		case 'V': // Vector
		{
			 if (compareStrings(*tokPtr, JSON_VECTOR))
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			 {
				   unexpectedKey(*tokPtr, __LINE__, __FILE__);
				   return false;
			 }
			 else
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 break;
		}
		case 'X':
		  {
		    if (compareStrings(*tokPtr, JSON_XML))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		case 'O':
		  {
		    if (compareStrings(*tokPtr, JSON_OPAQUE))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
				(*tokPtr)++;
				skipObject(tokPtr);
			}
				break;
		}
	}
	}

	if(!hasAckId) {
		missingKey(JSON_ACKID, __LINE__, __FILE__);
		return false;
	}
	if(!hasId) {
		missingKey(JSON_ID, __LINE__, __FILE__);
		return false;
	}
	if(!hasType) {
		missingKey(JSON_TYPE, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool jsonToRwfSimple::processGenericMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	jsmntok_t *msgTok = *tokPtr;
	RsslBuffer *bufPtr;

	bool hasId = false;
	bool hasType = false;

	// Defaults
	rsslMsgPtr->genericMsg.flags = RSSL_GNMF_MESSAGE_COMPLETE;

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
		case 'I':
			{
				if (compareStrings(*tokPtr, JSON_ID))
				{
					hasId = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TYPE))
				{
					hasType = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DOMAIN))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_FIELDS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_FILTERLIST))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'M':
			{
				if (compareStrings(*tokPtr, JSON_MAP))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_MESSAGE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'J':
			{
				if (compareStrings(*tokPtr, RSSL_OMMSTR_DT_JSON))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'K':
			{
				if (compareStrings(*tokPtr, JSON_KEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_KEY);
						return false;
					}
					rsslMsgPtr->genericMsg.flags |= RSSL_GNMF_HAS_MSG_KEY;
					if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'R':
			{
				if (compareStrings(*tokPtr, JSON_REQKEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_REQKEY);
						return false;
					}

					if (processKey(tokPtr, &rsslMsgPtr->genericMsg.reqMsgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
					rsslMsgPtr->genericMsg.flags |= RSSL_GNMF_HAS_REQ_MSG_KEY;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'C':
			{
				if (compareStrings(*tokPtr, JSON_COMPLETE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_COMPLETE);
						return false;
					}
					if (!isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->genericMsg.flags &= ~RSSL_GNMF_MESSAGE_COMPLETE;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'P':
			{
				if (compareStrings(*tokPtr, JSON_PARTNUMBER))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_PARTNUMBER);
						return false;
					}
					rsslMsgPtr->genericMsg.flags |= RSSL_GNMF_HAS_PART_NUM;
					rsslMsgPtr->genericMsg.partNum =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									   &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_PERMDATA))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_PERMDATA);
						return false;
					}
					bufPtr = &rsslMsgPtr->genericMsg.permData;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->genericMsg.flags |= RSSL_GNMF_HAS_PERM_DATA;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'S':
			{
				if (compareStrings(*tokPtr, JSON_SEQNUM))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_SEQNUM);
						return false;
					}
					rsslMsgPtr->genericMsg.flags |= RSSL_GNMF_HAS_SEQ_NUM;
					rsslMsgPtr->genericMsg.seqNum =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									   &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_SECSEQNUM))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_SECSEQNUM);
						return false;
					}
					rsslMsgPtr->genericMsg.flags |= RSSL_GNMF_HAS_SECONDARY_SEQ_NUM;
					rsslMsgPtr->genericMsg.secondarySeqNum =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									   &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_SERIES))
				{
				   (*tokPtr)++;
				   skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_EXTHDR))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_EXTHDR);
						return false;
					}
					bufPtr = &rsslMsgPtr->genericMsg.extendedHeader;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslMsgPtr->genericMsg.flags |= RSSL_GNMF_HAS_EXTENDED_HEADER;
				}
				else if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'V': // Vector
		{
			 if (compareStrings(*tokPtr, JSON_VECTOR))
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			 {
				   unexpectedKey(*tokPtr, __LINE__, __FILE__);
				   return false;
			 }
			 else
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 break;
		}
		case 'X':
		  {
		    if (compareStrings(*tokPtr, JSON_XML))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		case 'O':
		  {
		    if (compareStrings(*tokPtr, JSON_OPAQUE))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
				(*tokPtr)++;
				skipObject(tokPtr);
			}
				break;
		}
	}
	}

	if(!hasId) {
		missingKey(JSON_ID, __LINE__, __FILE__);
		return false;
	}
	if(!hasType) {
		missingKey(JSON_TYPE, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool jsonToRwfSimple::processPostMsg(jsmntok_t ** const tokPtr, RsslMsg *rsslMsgPtr, jsmntok_t ** const dataPtr, jsmntok_t ** const attribPtr, jsmntok_t ** const reqKeyattrib)
{
	RsslBuffer *bufPtr;
	jsmntok_t *msgTok = *tokPtr;

	bool hasId = false;
	bool hasType = false;

	// Defaults
	rsslMsgPtr->postMsg.flags = RSSL_PSMF_POST_COMPLETE;

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
		case 'I':
			{
				if (compareStrings(*tokPtr, JSON_ID))
				{
					hasId = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TYPE))
				{
					hasType = true;
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DOMAIN))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_FIELDS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_FILTERLIST))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'M':
			{
				if (compareStrings(*tokPtr, JSON_MAP))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if(compareStrings(*tokPtr, JSON_MESSAGE))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'J':
			{
				if (compareStrings(*tokPtr, RSSL_OMMSTR_DT_JSON))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'K':
			{
				if (compareStrings(*tokPtr, JSON_KEY))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_KEY);
						return false;
					}
					rsslMsgPtr->postMsg.flags |= RSSL_PSMF_HAS_MSG_KEY;
					if (processKey(tokPtr, &rsslMsgPtr->msgBase.msgKey, rsslMsgPtr->msgBase.domainType, attribPtr) == false)
						return false;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'P':
		{
			if (compareStrings(*tokPtr, JSON_POSTUSERINFO))
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_OBJECT)
				{
					unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
					return false;
				}
				if (!populatePostUserInfo(tokPtr, &rsslMsgPtr->postMsg.postUserInfo))
					return false;
			}
			else if (compareStrings(*tokPtr, JSON_POSTID))
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_POSTID);
					return false;
				}
				rsslMsgPtr->postMsg.flags |= RSSL_PSMF_HAS_POST_ID;
				rsslMsgPtr->postMsg.postId =
					rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								   &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
			}
			else if (compareStrings(*tokPtr, JSON_PARTNUMBER))
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_PARTNUMBER);
					return false;
				}
				rsslMsgPtr->postMsg.flags |= RSSL_PSMF_HAS_PART_NUM;
				rsslMsgPtr->postMsg.partNum =
					rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								   &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
			}
			else if (compareStrings(*tokPtr, JSON_POSTUSERRIGHTS))
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_POSTUSERRIGHTS);
					return false;
				}
				rsslMsgPtr->postMsg.flags |= RSSL_PSMF_HAS_POST_USER_RIGHTS;
				rsslMsgPtr->postMsg.postUserRights =
					rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
								   &_jsonMsg[(*tokPtr)->end]);
				(*tokPtr)++;
			}
			else if (compareStrings(*tokPtr, JSON_PERMDATA))
			{
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_PERMDATA);
					return false;
				}
				bufPtr = &rsslMsgPtr->postMsg.permData;
				if (processBuffer(tokPtr, &bufPtr, 0) == false)
					return false;
				rsslMsgPtr->postMsg.flags |= RSSL_PSMF_HAS_PERM_DATA;
			}
			else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			{
				unexpectedKey(*tokPtr, __LINE__, __FILE__);
				return false;
			}
			else
			{
				(*tokPtr)++;
				skipObject(tokPtr);
			}
			break;
		}
		case 'S':
			{
				if (compareStrings(*tokPtr, JSON_SEQNUM))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_SEQNUM);
						return false;
					}
					rsslMsgPtr->postMsg.flags |= RSSL_PSMF_HAS_SEQ_NUM;
					rsslMsgPtr->postMsg.seqNum =
						rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									   &_jsonMsg[(*tokPtr)->end]);
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_SERIES))
				{
				   (*tokPtr)++;
				   skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'C':
			{
				if (compareStrings(*tokPtr, JSON_COMPLETE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_COMPLETE);
						return false;
					}
					if (!isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->postMsg.flags &= ~RSSL_PSMF_POST_COMPLETE;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}

		case 'A':
			{
				if (compareStrings(*tokPtr, JSON_ACK))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_ACK);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						rsslMsgPtr->postMsg.flags |= RSSL_PSMF_ACK;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_EXTHDR))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_EXTHDR);
						return false;
					}
					bufPtr = &rsslMsgPtr->postMsg.extendedHeader;
					if (processBuffer(tokPtr, &bufPtr, 0) == false)
						return false;
					rsslPostMsgApplyHasExtendedHdr(&rsslMsgPtr->postMsg);
				}
				else if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'V': // Vector
		{
			 if (compareStrings(*tokPtr, JSON_VECTOR))
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			 {
				   unexpectedKey(*tokPtr, __LINE__, __FILE__);
				   return false;
			 }
			 else
			 {
				   (*tokPtr)++;
				   skipObject(tokPtr);
			 }
			 break;
		}
		case 'X':
		  {
		    if (compareStrings(*tokPtr, JSON_XML))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		case 'O':
		  {
		    if (compareStrings(*tokPtr, JSON_OPAQUE))
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		    {
		      unexpectedKey(*tokPtr, __LINE__, __FILE__);
		      return false;
		    }
		    else
		    {
		      (*tokPtr)++;
		      skipObject(tokPtr);
		    }
		    break;
		  }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				else
				{
				(*tokPtr)++;
				skipObject(tokPtr);
				}
				break;
			}
		}
	}

	if(!hasId) {
		missingKey(JSON_ID, __LINE__, __FILE__);
		return false;
	}
	if(!hasType) {
		missingKey(JSON_TYPE, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool jsonToRwfSimple::processFieldList(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	RsslFieldId fieldId;
	void *voidPtr = 0;
	RsslBuffer *bufPtr;
	const RsslDictionaryEntry *def;

	jsmntok_t *fieldListTok = *tokPtr;

	// MJD - handle empty case

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_FIELDS);
		return false;
	}
	(*tokPtr)++;

	rsslClearFieldList(&fieldList);
	rsslFieldListApplyHasStandardData(&fieldList);

	if ((_rsslRet = rsslEncodeFieldListInit(&_iter, &fieldList, 0, 0)) < RSSL_RET_SUCCESS )
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < fieldListTok->end)
	{
		rsslClearFieldEntry(&fieldEntry);

		switch ((*tokPtr)->type)
		{
		case JSMN_STRING:
			{
				RsslBuffer fidName = { ((*tokPtr)->end - (*tokPtr)->start), &_jsonMsg[(*tokPtr)->start]};
				def = rsslDictionaryGetEntryByFieldName(_dictionaryList[0], &fidName);
				if (def)
					fieldEntry.fieldId = def->fid;
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_FIDS)
				{
					unexpectedFid(*tokPtr, __LINE__, __FILE__, &JSON_FIELDS);
					return false;
				}
				break;
			}
		case JSMN_PRIMITIVE:
			{
				fieldEntry.fieldId = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
												   &_jsonMsg[(*tokPtr)->end]);
				def = _dictionaryList[0]->entriesArray[fieldEntry.fieldId];
				if (!def && _flags & JSON_FLAG_CATCH_UNEXPECTED_FIDS)
				{
					unexpectedFid(*tokPtr, __LINE__, __FILE__, &JSON_FIELDS);
					return false;
				}
				break;
			}
		default:
			{
				unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_FIELDS);
				return false;
			}
		}
		(*tokPtr)++;

		if (def)
		{
			voidPtr = 0;
			bufPtr = &fieldEntry.encData;

			fieldEntry.dataType = def->rwfType;

			if (fieldEntry.dataType < RSSL_DT_SET_PRIMITIVE_MAX)
			{
				if (fieldEntry.dataType != RSSL_DT_ARRAY)
				{
					// Normal primitive processing
					if (processPrimitive(fieldEntry.dataType, tokPtr,
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
					if (processPrimitive(fieldEntry.dataType, tokPtr, &bufPtr, &voidPtr) == false)
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

					if (processContainer(tokPtr, fieldEntry.dataType, 0) == false)
						return false;

					if ((_rsslRet = rsslEncodeFieldEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
			}
		}
	}

	if ((_rsslRet = rsslEncodeFieldListComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS )
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool jsonToRwfSimple::processElementList(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	//	jsmntok_t *elementListTok = *tokPtr;
	jsmntok_t *dataTok;
	RsslBuffer *bufPtr;
	void *voidPtr;

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_ELEMENTS);
		return false;
	}

	rsslClearElementList(&elementList);
	rsslElementListApplyHasStandardData(&elementList);
	if ( (_rsslRet = rsslEncodeElementListInit(&_iter, &elementList, (RsslLocalElementSetDefDb*)setDb, 0)) < RSSL_RET_SUCCESS )
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	dataTok = *tokPtr;
	skipObject(tokPtr);
	int numEntries = dataTok->size;
	dataTok++;
	for (int i = 0; i < numEntries; i+=2)
	{
		if (dataTok->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, dataTok, __LINE__, __FILE__, &JSON_ELEMENTS);
			return false;
		}
		rsslClearElementEntry(&elementEntry);
		elementEntry.name.data = &_jsonMsg[dataTok->start];
		elementEntry.name.length = dataTok->end - dataTok->start;
		dataTok++;

		if (dataTok->type == JSMN_STRING)
		{
			voidPtr = 0;
			elementEntry.dataType = RSSL_DT_ASCII_STRING;
			bufPtr = &elementEntry.encData;
			if (processPrimitive(elementEntry.dataType, &dataTok,
								 &bufPtr, &voidPtr) == false)
				return false;

			if (rsslEncodeElementEntry(&_iter, &elementEntry, voidPtr) < RSSL_RET_SUCCESS)
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
		}
		else if (dataTok->type == JSMN_PRIMITIVE)
		{
			if (_jsonMsg[dataTok->start] == 'n' || _jsonMsg[dataTok->start] == 't' || _jsonMsg[dataTok->start] == 'f')
			{
				unexpectedParameter(dataTok, __LINE__, __FILE__);
				return false;
			}
			voidPtr = 0;
			elementEntry.dataType = RSSL_DT_UINT;
			bufPtr = &elementEntry.encData;
			if (processPrimitive(elementEntry.dataType, &dataTok,
								 &bufPtr, &voidPtr) == false)
				return false;
			if (rsslEncodeElementEntry(&_iter, &elementEntry, voidPtr) < RSSL_RET_SUCCESS)
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
		}
		else if (dataTok->type == JSMN_OBJECT)
		{
			int size = dataTok->size;
			jsmntok_t *tmpTok = NULL;
			bool hasType = false;
			dataTok++;
			for (int i = 0; i < size; i+=2)
			{
				if (dataTok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, dataTok, __LINE__, __FILE__, &JSON_ELEMENTS);
					return false;
				}
				if (compareStrings(dataTok, JSON_TYPE))
				{
					dataTok++;
					if (!getDataType(dataTok, &elementEntry.dataType))
						return false;
					dataTok++;
					hasType = true;
				}
				else if (compareStrings(dataTok, JSON_DATA))
				{
					dataTok++;
					tmpTok = dataTok;
					skipObject(&dataTok);
				}
			}

			if (tmpTok == NULL)
			{
				missingKey(JSON_DATA, __LINE__, __FILE__, &elementEntry.name);
				return false;
			}
			if (!hasType)
			{
				missingKey(JSON_TYPE, __LINE__, __FILE__, &elementEntry.name);
				return false;
			}

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
		}
		else
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
	if ((_rsslRet = rsslEncodeElementListComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool jsonToRwfSimple::processFilterList(jsmntok_t ** const tokPtr, void* setDb)
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
	bool foundValidToken;

	rsslClearFilterList(&filterList);

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_FILTERLIST);
		return false;
	}

	// Set default filterList container type to Element List
	filterList.containerType = RSSL_DT_ELEMENT_LIST;

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < filterListTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_FILTERLIST);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'E':	// Entries
			{
				if (compareStrings(*tokPtr, JSON_ENTRIES))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_ARRAY)
					{
						unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__, &JSON_ENTRIES);
						return false;
					}
					dataTok = *tokPtr;
					skipObject(tokPtr);
					break;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS){
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_FILTERLIST);
					return false;
			}
				else
			{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
			}
		case 'C':	// CountHint
			{
				if (compareStrings(*tokPtr, JSON_COUNTHINT))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_COUNTHINT);
						return false;
					}
					filterList.totalCountHint = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
									&_jsonMsg[(*tokPtr)->end]);
					rsslFilterListApplyHasTotalCountHint(&filterList);
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_FILTERLIST);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_FILTERLIST);
					return false;
				}
				else
				{
				(*tokPtr)++;
				skipObject(tokPtr);
			}
		}
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
				unexpectedTokenType(JSMN_OBJECT, tmpTok, __LINE__, __FILE__, &JSON_FILTERLIST);
				return false;
			}

			entryDataTok = 0;
			rsslClearFilterEntry(&filterEntry);
			filterEntry.containerType = filterList.containerType;
			filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
			entryTok = tmpTok;
			tmpTok++;
			while ((tmpTok < _tokensEndPtr &&
				tmpTok->end < entryTok->end))
			{
				if (tmpTok->type != JSMN_STRING)
				{
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
					return false;
				}

				foundValidToken = false;

				switch (_jsonMsg[(tmpTok)->start])
				{
				case 'I':	// Filter Id
					{
						if (compareStrings(tmpTok, JSON_ID))
						{
							foundValidToken = true;
							tmpTok++;
							if (tmpTok->type != JSMN_PRIMITIVE)
							{
								unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__, &JSON_ID);
								return false;
							}
							filterEntry.id = rtr_atoi_size(&_jsonMsg[tmpTok->start],
														   &_jsonMsg[tmpTok->end]);
							tmpTok++;
						}
						break;
					}
				case 'A':	// Action
					{
						if (compareStrings(tmpTok, JSON_ACTION)) //
						{
							foundValidToken = true;
							tmpTok++;
							switch (tmpTok->type)
							{
							case JSMN_STRING:
								{
									if (compareStrings(tmpTok, RSSL_OMMSTR_FTEA_UPDATE_ENTRY))
									{
										filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
									}
									else if (compareStrings(tmpTok, RSSL_OMMSTR_FTEA_SET_ENTRY))
									{
										filterEntry.action = RSSL_FTEA_SET_ENTRY;
									}
									else if (compareStrings(tmpTok, RSSL_OMMSTR_FTEA_CLEAR_ENTRY))
									{
										filterEntry.action = RSSL_FTEA_CLEAR_ENTRY;
									}
									else
									{
										unexpectedParameter(tmpTok, __LINE__, __FILE__, &JSON_ACTION);
										return false;
									}
									break;
								}
							case JSMN_PRIMITIVE:
								{
									filterEntry.action = rtr_atoi_size(&_jsonMsg[tmpTok->start],
																	   &_jsonMsg[tmpTok->end]);
									break;
								}
							default:
								{
									unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__, &JSON_ACTION);
									return false;
								}
							}
							tmpTok++;
						}
						break;
					}
				case 'P':	// Permission Data
					{
						if (compareStrings(tmpTok, JSON_PERMDATA))
						{
							foundValidToken = true;
							tmpTok++;
							if (tmpTok->type != JSMN_STRING)
							{
								unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__);
								return false;
							}
							bufPtr = &filterEntry.permData;
							if (processBuffer(&tmpTok, &bufPtr, 0) == false)
								return false;
							rsslFilterEntryApplyHasPermData(&filterEntry);
						}
						break;
					}
				default:
					break;
				} // End Case

				if (foundValidToken)
					continue; // Go to the beginning of while

				// Look for containers.
				if (!getContainerType(tmpTok, &filterEntry.containerType))
				{
					if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
						return false;
					else
					{
						(*tokPtr)++;
						skipObject(tokPtr);
						continue;
					}
				}

				tmpTok++;
				entryDataTok = tmpTok;
				skipObject(&tmpTok);

			} // End While

			// Encode each Filter Entry

			if (entryDataTok && filterEntry.action != RSSL_FTEA_CLEAR_ENTRY)
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

bool jsonToRwfSimple::processVector(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslVector vector = RSSL_INIT_VECTOR;
	RsslVectorEntry vectorEntry;
	RsslContainerType summaryDataContainerType = RSSL_DT_UNKNOWN;
	RsslContainerType vectorEntryContainerType = RSSL_DT_UNKNOWN;
	jsmntok_t *vectorTok = *tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *summaryTok = 0;
	jsmntok_t *entryTok = 0;
	jsmntok_t *tmpTok = 0;
	jsmntok_t *entryDataTok = 0;
	RsslBuffer *bufPtr;
	int i = 0;
	bool foundValidToken;

	rsslClearVector(&vector);

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_VECTOR);
		return false;
	}

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < vectorTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_VECTOR);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
			case 'S':
			{
				if (compareStrings(*tokPtr, JSON_SUMMARY)) // Summary Data
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_SUMMARY);
						return false;
					}

					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_SUMMARY);
						return false;
					}

					if (!getContainerType(*tokPtr, &summaryDataContainerType))
					{
						if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
							return false;
						else
						{
							(*tokPtr)++;
							skipObject(tokPtr);
							continue;
						}
					}

					(*tokPtr)++;
					summaryTok = *tokPtr;
					skipObject(tokPtr);
				}
				else if (compareStrings(*tokPtr, JSON_SUPPORTSORTING)) // Support Sorting
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_SUPPORTSORTING);
						return false;
					}

					if (isTokenTrue(*tokPtr))
					{
						rsslVectorApplySupportsSorting(&vector);
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_VECTOR);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
			case 'C':
			{
				if (compareStrings(*tokPtr, JSON_COUNTHINT)) // Total Count Hint
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_COUNTHINT);
						return false;
					}

					vector.totalCountHint = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
													  &_jsonMsg[(*tokPtr)->end]);
					rsslVectorApplyHasTotalCountHint(&vector);
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_VECTOR);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
			case 'E':
			{
				if (compareStrings(*tokPtr, JSON_ENTRIES)) // Vector Entries
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_ARRAY)
					{
						unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__, &JSON_ENTRIES);
						return false;
					}
					dataTok = *tokPtr;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_VECTOR);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
			default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_VECTOR);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
			}
		}
	}

	if (summaryTok)
	{
		rsslVectorApplyHasSummaryData(&vector);
		vector.containerType = summaryDataContainerType;

		if ((_rsslRet = rsslEncodeVectorInit(&_iter, &vector, 0, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
		if (processContainer(&summaryTok, summaryDataContainerType, 0) == false)
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
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
					return false;
				}

				foundValidToken = false;

				// Look for Action, Index and PermData
				switch (_jsonMsg[(tmpTok)->start])
				{
					case 'A':	// Action
					{
						if (compareStrings(tmpTok, JSON_ACTION))
						{
							tmpTok++;
							if (tmpTok->type != JSMN_STRING)
							{
								unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__, &JSON_ACTION);
								return false;
							}

							if (!getVectorAction(tmpTok, &vectorEntry.action))
							{
								unexpectedParameter(tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
								return false;
							}

							tmpTok++;
							foundValidToken = true;
						}
						break;
					}
					case 'I':	// Index
					{
						if (compareStrings(tmpTok, JSON_INDEX))
						{
							tmpTok++;
							if (tmpTok->type != JSMN_PRIMITIVE)
							{
								unexpectedTokenType(JSMN_PRIMITIVE, tmpTok, __LINE__, __FILE__, &JSON_INDEX);
								return false;
							}

							vectorEntry.index = rtr_atoi_size(&_jsonMsg[tmpTok->start],
															  &_jsonMsg[tmpTok->end]);
							tmpTok++;
							foundValidToken = true;
						}
						break;
					}
					case 'P':	// Permission Data
					{
						if (compareStrings(tmpTok, JSON_PERMDATA))
						{
							tmpTok++;
							if (tmpTok->type != JSMN_STRING)
							{
								unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__, &JSON_PERMDATA);
								return false;
							}
							bufPtr = &vectorEntry.permData;
							if (processBuffer(&tmpTok, &bufPtr, 0) == false)
								return false;
							rsslVectorEntryApplyHasPermData(&vectorEntry);
							foundValidToken = true;
						}
						break;
					}
					default:
						break;
				} // End Case

				if (foundValidToken)
					continue; // Go to the beginning of while

				// Look for containers.
				if (!getContainerType(tmpTok, &vectorEntryContainerType))
				{
					if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
						return false;
					else
					{
						(*tokPtr)++;
						skipObject(tokPtr);
						continue;
					}
				}

				if (summaryTok)
				{
					if (vectorEntryContainerType != summaryDataContainerType)
					{
						//vector entry is not in the same data type as the summary data
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
				}
				else if (vector.containerType == RSSL_DT_UNKNOWN)
				{
					//First time only if the summary data is not present
					vector.containerType = vectorEntryContainerType;
					if ((_rsslRet = rsslEncodeVectorInit(&_iter, &vector, 0, 0)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
				}
				else if (vector.containerType != vectorEntryContainerType)
				{
					//All vector entries are not in the same data type
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}

				tmpTok++;
				entryDataTok = tmpTok;
				skipObject(&tmpTok);
			} // End While - vectorEntry

			// Encode each vectorEntry
			if (entryDataTok)
			{
				if ((_rsslRet = rsslEncodeVectorEntryInit(&_iter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
				if (processContainer(&entryDataTok, vector.containerType, 0) == false)
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
	else if (!summaryTok)
	{
		vector.containerType = RSSL_DT_NO_DATA;
		if ((_rsslRet = rsslEncodeVectorInit(&_iter, &vector, 0, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}

	if ((_rsslRet = rsslEncodeVectorComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool jsonToRwfSimple::processMap(jsmntok_t ** const tokPtr, void* setDb)
{
	bool hasKeyType = false;

	RsslMap map = RSSL_INIT_MAP;
	void* encKeyPtr = 0;
	RsslBuffer *bufPtr;
	RsslBuffer buffer;
	RsslMapEntry mapEntry;
	RsslContainerType summaryContainerType;
	RsslContainerType mapEntryContainerType = RSSL_DT_UNKNOWN;
	jsmntok_t *mapTok = *tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *summaryTok = 0;
	jsmntok_t *entryTok = 0;
	jsmntok_t *tmpTok = 0;
	jsmntok_t *entryDataTok = 0;
	bool foundValidToken;

	int i = 0;

	rsslClearMap(&map);

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_MAP);
		return false;
	}

	(*tokPtr)++;

	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < mapTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_MAP);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
			case 'S':	 // Summary
			{
				if (compareStrings(*tokPtr, JSON_SUMMARY)) // Summary
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_SUMMARY);
						return false;
					}

					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_SUMMARY);
						return false;
					}

					if (!getContainerType(*tokPtr, &summaryContainerType))
					{
						if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
							return false;
						else
						{
							(*tokPtr)++;
							skipObject(tokPtr);
							continue;
						}
					}

					(*tokPtr)++;
					summaryTok = *tokPtr;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_MAP);
					return false;
				}
				else
				{
					unexpectedParameter(*tokPtr, __LINE__, __FILE__);
					return false;
				}
				break;
			}
			case 'E':	// Entries
			{
				if (compareStrings(*tokPtr, JSON_ENTRIES)) // Map Entries
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_ARRAY)
					{
						unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__, &JSON_ENTRIES);
						return false;
					}
					dataTok = *tokPtr;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_MAP);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
			case 'K':	// KeyType or KeyFieldId
			{
				if (compareStrings(*tokPtr, JSON_KEYTYPE))
				{
					hasKeyType = true;
					(*tokPtr)++;
					if (!getDataType(*tokPtr, &map.keyPrimitiveType))
						return false;
					(*tokPtr)++;
				}
				else if (compareStrings(*tokPtr, JSON_KEYFIELDID))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_KEYFIELDID);
						return false;
					}
					map.keyFieldId = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
												   &_jsonMsg[(*tokPtr)->end]);
					rsslMapApplyHasKeyFieldId(&map);
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_MAP);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
			case 'C':	// CountHint
			{
				if (compareStrings(*tokPtr, JSON_COUNTHINT))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_COUNTHINT);
						return false;
					}
					map.totalCountHint = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
									&_jsonMsg[(*tokPtr)->end]);
					rsslMapApplyHasTotalCountHint(&map);
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_MAP);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
			default:
			if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			{
				unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_MAP);
				return false;
			}
			else
			{
				(*tokPtr)++;
				skipObject(tokPtr);
			}
		} // End of case
	} // End of while

	if(!hasKeyType)
	{
		missingKey(JSON_KEYTYPE, __LINE__, __FILE__, &JSON_MAP);
		return false;
	}
	if (summaryTok)
	{
		rsslMapApplyHasSummaryData(&map);
		map.containerType = summaryContainerType;

		if ((_rsslRet = rsslEncodeMapInit(&_iter, &map, 0, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
		if (processContainer(&summaryTok, summaryContainerType, 0) == false)
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
			bool hasEntriesKey = false;
			bool hasEntriesAction = false;

			if (tmpTok->type != JSMN_OBJECT)
			{
				unexpectedTokenType(JSMN_OBJECT, tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
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
					unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
					return false;
				}

				foundValidToken = false;

				switch (_jsonMsg[(tmpTok)->start])
				{
				case 'A':	// Action
					{
						if (compareStrings(tmpTok, JSON_ACTION))
						{
							hasEntriesAction = true;
							foundValidToken = true;
							tmpTok++;
							if (tmpTok->type != JSMN_STRING)
							{
								unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__, &JSON_ACTION);
								return false;
							}
							switch (_jsonMsg[(tmpTok)->start])
							{
							case 'A':
								{
									if (compareStrings(tmpTok, RSSL_OMMSTR_MPEA_ADD_ENTRY))
										mapEntry.action = RSSL_MPEA_ADD_ENTRY;
									else
									{
										unexpectedParameter(tmpTok, __LINE__, __FILE__, &JSON_ACTION);
										return false;
									}
									break;
								}
							case 'U':
								{
									if (compareStrings(tmpTok, RSSL_OMMSTR_MPEA_UPDATE_ENTRY))
										mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
									else
									{
										unexpectedParameter(tmpTok, __LINE__, __FILE__, &JSON_ACTION);
										return false;
									}
									break;
								}
							case 'D':
								{
									if (compareStrings(tmpTok, RSSL_OMMSTR_MPEA_DELETE_ENTRY))
										mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
									else
									{
										unexpectedParameter(tmpTok, __LINE__, __FILE__, &JSON_ACTION);
										return false;
									}
									break;
								}
							default:
								{
									unexpectedParameter(tmpTok, __LINE__, __FILE__, &JSON_ACTION);
									return false;
								}
							}
						}
						else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
						{
							unexpectedKey(tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
							return false;
						}
						else
						{
						tmpTok++;
						}
						tmpTok++;
						break;
					}
				case 'K':	// Key
					{
						if (compareStrings(tmpTok, JSON_KEY))
						{
							hasEntriesKey = true;
							foundValidToken = true;
							tmpTok++;
							if (processPrimitive(map.keyPrimitiveType, &tmpTok,
												 &bufPtr, &encKeyPtr) == false)
								return false;
							if (encKeyPtr == 0)
								encKeyPtr = bufPtr;
						}
						else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
						{
							unexpectedKey(tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
							return false;
						}
						else
						{
							tmpTok++;
							skipObject(&tmpTok);
						}
						break;
					}
				case 'P':	// Permission Data
					{
						if (compareStrings(tmpTok, JSON_PERMDATA))
						{
							foundValidToken = true;
							tmpTok++;
							if (tmpTok->type != JSMN_STRING)
							{
								unexpectedTokenType(JSMN_STRING, tmpTok, __LINE__, __FILE__, &JSON_PERMDATA);
								return false;
							}
							bufPtr = &mapEntry.permData;
							if (processBuffer(&tmpTok, &bufPtr, 0) == false)
								return false;
							rsslMapEntryApplyHasPermData(&mapEntry);
							break;
						}
						else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
						{
							unexpectedKey(tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
							return false;
					}
						else
						{
							tmpTok++;
							skipObject(&tmpTok);
						}
						break;
					}
				default:
					break;
				} // End Case - mapEntry

				if (foundValidToken)
					continue; // Go to the beginning of while

				// Look for containers.
				if (!getContainerType(tmpTok, &mapEntryContainerType))
				{
					if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
						return false;
					else
					{
						(*tokPtr)++;
						skipObject(tokPtr);
						continue;
					}
				}

				tmpTok++;
				entryDataTok = tmpTok;
				skipObject(&tmpTok);

				if (summaryTok)
				{
					if (mapEntryContainerType != summaryContainerType)
					{
						//map entry is not in the same data type as the summary data
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
				}
				else if (map.containerType == RSSL_DT_UNKNOWN)
				{
					//First time only if the summary data is not present
					map.containerType = mapEntryContainerType;
					if ((_rsslRet = rsslEncodeMapInit(&_iter, &map, 0, 0)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
				}
				else if (map.containerType != mapEntryContainerType)
				{
					//All map entries are not in the same data type
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}

			} // End While - mapEntry

			if (!summaryTok && map.containerType == RSSL_DT_UNKNOWN)
			{
				map.containerType = RSSL_DT_NO_DATA;
				if ((_rsslRet = rsslEncodeMapInit(&_iter, &map, 0, 0)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}

			if(!hasEntriesKey) {
				missingKey(JSON_KEY, __LINE__, __FILE__, &JSON_ENTRIES);
				return false;
			}
			if(!hasEntriesAction) {
				missingKey(JSON_ACTION, __LINE__, __FILE__, &JSON_ENTRIES);
				return false;
			}

			if (encKeyPtr == 0)
			{
				missingKey(JSON_KEY, __LINE__, __FILE__, &JSON_ENTRIES);
				return false;
			}

			// Encode each Map Entry
			if (entryDataTok && mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
			{
				if ((_rsslRet = rsslEncodeMapEntryInit(&_iter, &mapEntry, encKeyPtr, 0)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}

				if (processContainer(&entryDataTok, map.containerType, 0) == false)
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

	if (!summaryTok && map.containerType == RSSL_DT_UNKNOWN)
	{
		map.containerType = RSSL_DT_NO_DATA;
		if ((_rsslRet = rsslEncodeMapInit(&_iter, &map, 0, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}

	if ((_rsslRet = rsslEncodeMapComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}
bool jsonToRwfSimple::processSeries(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslSeries series = RSSL_INIT_SERIES;
	RsslSeriesEntry seriesEntry;
	RsslContainerType summaryContainerType = RSSL_DT_UNKNOWN;
	RsslContainerType seriesEntryContainerType = RSSL_DT_UNKNOWN;
	jsmntok_t *seriesTok = *tokPtr;
	jsmntok_t *dataTok = 0;
	jsmntok_t *summaryTok = 0;
	jsmntok_t *tmpTok = 0;
	jsmntok_t *entryTok = 0;
	jsmntok_t *entryDataTok = 0;
	int i = 0;

	rsslClearSeries(&series);

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_SERIES);
		return false;
	}

	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < seriesTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_SERIES);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
			case 'E':	// Entries
			{
				if (compareStrings(*tokPtr, JSON_ENTRIES)) // Series Entries
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_ARRAY)
					{
						unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__, &JSON_ENTRIES);
						return false;
					}
					dataTok = *tokPtr;
					skipObject(tokPtr);
					break;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey((*tokPtr), __LINE__, __FILE__, &JSON_SERIES);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
			case 'S':   // Summary
			{
				if (compareStrings(*tokPtr, JSON_SUMMARY)) // Summary
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_OBJECT)
					{
						unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_SUMMARY);
						return false;
					}

					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_STRING)
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_SUMMARY);
						return false;
					}

					if (!getContainerType(*tokPtr, &summaryContainerType))
					{
						if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
							return false;
						else
						{
							(*tokPtr)++;
							skipObject(tokPtr);
							continue;
						}
					}

					(*tokPtr)++;
					summaryTok = *tokPtr;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey((*tokPtr), __LINE__, __FILE__, &JSON_SERIES);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
			case 'C':
			{
				if (compareStrings((*tokPtr), JSON_COUNTHINT)) // Count Hint
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_COUNTHINT);
						return false;
					}

					series.totalCountHint = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
												   &_jsonMsg[(*tokPtr)->end]);
					rsslSeriesApplyHasTotalCountHint(&series);
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey((*tokPtr), __LINE__, __FILE__, &JSON_SERIES);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey((*tokPtr), __LINE__, __FILE__, &JSON_SERIES);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
			}
		} // End of case
	} // End of while

	if (summaryTok)
	{
		rsslSeriesApplyHasSummaryData(&series);
		series.containerType = summaryContainerType;

		if ((_rsslRet = rsslEncodeSeriesInit(&_iter, &series, 0, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
		if (processContainer(&summaryTok, summaryContainerType, 0) == false)
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
			{
				unexpectedTokenType(JSMN_OBJECT, tmpTok, __LINE__, __FILE__, &JSON_ENTRIES);
				return false;
			}

			entryDataTok = 0;
			rsslClearSeriesEntry(&seriesEntry);
			entryTok = tmpTok;
			tmpTok++;
			if (!getContainerType(tmpTok, &seriesEntryContainerType))
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
					return false;
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
					continue;
				}
			}

			if(summaryTok)
			{
				if (seriesEntryContainerType != summaryContainerType)
				{
					//series entries are not in the same data type
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			else if (series.containerType == RSSL_DT_UNKNOWN)
			{
				//First time only if the summary data is not present
				series.containerType = seriesEntryContainerType;
				if ((_rsslRet = rsslEncodeSeriesInit(&_iter, &series, 0, 0)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			else if (series.containerType != seriesEntryContainerType)
			{
				//All series entries are not in the same data type
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}

			tmpTok++;

			entryDataTok = tmpTok;
			skipObject(&tmpTok);

			if (entryDataTok)
			{
				if ((_rsslRet = rsslEncodeSeriesEntryInit(&_iter, &seriesEntry, 0)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
				if (processContainer(&entryDataTok, series.containerType, 0) == false)
					return false;
				if ((_rsslRet = rsslEncodeSeriesEntryComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			else
			{
				if ((_rsslRet = rsslEncodeSeriesEntry(&_iter, &seriesEntry)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
		} // End for
	} // End if(dataTok)

	if(!summaryTok && (!dataTok || dataTok->size == 0))
	{
		series.containerType = RSSL_DT_NO_DATA;
		if ((_rsslRet = rsslEncodeSeriesInit(&_iter, &series, 0, 0)) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}

	if ((_rsslRet = rsslEncodeSeriesComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool jsonToRwfSimple::processMsg(jsmntok_t ** const tokPtr, void* setDb)
{
	jsmntok_t *		prevViewTokPtr = _viewTokPtr;
	jsmntok_t *		prevBatchReqTokPtr = _batchReqTokPtr;
	jsmntok_t *		prevBatchCloseTokPtr = _batchCloseTokPtr;

	RsslJsonMsg jsonMsg;
	rsslClearJsonMsg(&jsonMsg);

	// MJD - need to locally store _viewTokPtr and _batchReqTokPtr ????

	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_MESSAGE);
		return false;
	}

	if (processMessage(tokPtr, &jsonMsg))
	{
		_viewTokPtr = prevViewTokPtr;
		_batchReqTokPtr = prevBatchReqTokPtr;
		_batchCloseTokPtr = prevBatchCloseTokPtr;
		return true;
	}
	return false;
}

bool jsonToRwfSimple::processJson(jsmntok_t ** const tokPtr, void* setDb)
{
	RsslBuffer buffer = RSSL_INIT_BUFFER;
	RsslBuffer jsonBuffer;
	jsonBuffer.length = (*tokPtr)->end - (*tokPtr)->start;
	jsonBuffer.data = &_jsonMsg[(*tokPtr)->start];

	jsmntok_t *jsonObjTok = *tokPtr;
	skipObject(tokPtr);

	_rsslRet = rsslEncodeNonRWFDataTypeInit(&_iter, &buffer);
	if (jsonBuffer.length > buffer.length)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	// MJD could optimize this by encoding directly to buffer returned by rsslEncodeNonRWFDataTypeInit()
	memcpy(buffer.data, jsonBuffer.data, jsonBuffer.length);
	buffer.length = jsonBuffer.length;

	if ((_rsslRet = rsslEncodeNonRWFDataTypeComplete(&_iter, &buffer, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Primitives
//////////////////////////////////////////////////////////////////////
bool jsonToRwfSimple::processReal(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{

	int i, expVal;
	int decimalLoc = 0;
	int eLoc = 0;
	bool foundE = false;
	bool foundDecimal = false;
	jsmntok_t *realTok;

	*ptrBufPtr = 0;
	*ptrVoidPtr = &_realVar;

	rsslClearReal(&_realVar);
	switch ((*tokPtr)->type)
	{
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
				/* the decimal always be prior to the exponent*/
				if (_jsonMsg[i] == '.')
				{
					foundDecimal = true;
					decimalLoc = i;
				}
				if (_jsonMsg[i] == 'e' || _jsonMsg[i] == 'E')
				{
					foundE = true;
					eLoc = i;
					break;
				}
			}
			if (foundE)
			{
				_realVar.value = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start], &_jsonMsg[i]);
				if (foundDecimal == true)
				{
					_realVar.value = _realVar.value*(int)pow(10, (i - decimalLoc - 1));

					_realVar.value += rtr_atoi_size(&_jsonMsg[decimalLoc], &_jsonMsg[i]);
				}

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
					if (foundDecimal)
					{
						if ((eLoc - decimalLoc - 1) > expVal)
						{
							/* find out how many digits are between the . and the E. There should be no whitespace in this element */
							expVal = expVal - (eLoc - decimalLoc - 1);

							if (expVal >= NEG_EXP_MIN && expVal <= NEG_EXP_MAX)
								_realVar.hint = _negExponentTable[expVal];
							else
							{
								unexpectedParameter(*tokPtr, __LINE__, __FILE__);
								return false;
							}
						}
					}
					if (expVal >= POS_EXP_MIN && expVal <= POS_EXP_MAX)
					{
						_realVar.hint = _posExponentTable[expVal];
					}
					else
					{
						unexpectedParameter(*tokPtr, __LINE__, __FILE__);
						return false;
					}
				}
			}

			else
			{
				RsslBuffer buf;
				buf.data = &_jsonMsg[(*tokPtr)->start];
				buf.length = &_jsonMsg[(*tokPtr)->end] - &_jsonMsg[(*tokPtr)->start];
				if ((_rsslRet = rsslNumericStringToReal(&_realVar, &buf)) < RSSL_RET_SUCCESS)
				{
					error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
					return false;
				}
			}
			(*tokPtr)++;
			break;
		}
	case JSMN_STRING:
		{
			switch(_jsonMsg[(*tokPtr)->start])
			{
				case '-':
				case 'I':
				case 'i':
				case 'N':
				case 'n':
				{
					RsslBuffer buf;
					buf.data = &_jsonMsg[(*tokPtr)->start];
					buf.length = &_jsonMsg[(*tokPtr)->end] - &_jsonMsg[(*tokPtr)->start];
					if ((_rsslRet = rsslNumericStringToReal(&_realVar, &buf)) < RSSL_RET_SUCCESS)
					{
						error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
						return false;
					}
					break;
				}
				default:
					unexpectedParameter(*tokPtr, __LINE__, __FILE__);
					return false;
			}
			(*tokPtr)++;
			break;
		}
	case JSMN_OBJECT:
		{
			// MJD ToDo
			return false;
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

bool jsonToRwfSimple::processArray(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	jsmntype_t typeCheck;
	RsslBuffer buf;
	RsslBuffer *bufPtr;
	jsmntok_t *dataTok = NULL;
	bool hasType = false;
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

		if (compareStrings(*tokPtr, JSON_LENGTH))
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_PRIMITIVE)
			{
				unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_LENGTH);
				return false;
			}
			array.itemLength = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
											 &_jsonMsg[(*tokPtr)->end]);
			(*tokPtr)++;
		}
		else if (compareStrings(*tokPtr, JSON_TYPE))
		{
			(*tokPtr)++;
			hasType = true;
			if (!getDataType(*tokPtr, &array.primitiveType))
				return false;

			(*tokPtr)++;
		}
		else if (compareStrings(*tokPtr, JSON_DATA))
		{
			(*tokPtr)++;
			if ((*tokPtr)->type != JSMN_ARRAY)
			{
				unexpectedTokenType(JSMN_ARRAY, *tokPtr, __LINE__, __FILE__, &JSON_DATA);
				return false;
			}
			dataTok = *tokPtr;
			skipObject(tokPtr);
		}
		else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
		{
			unexpectedKey(*tokPtr, __LINE__, __FILE__, &RSSL_OMMSTR_DT_ARRAY);
			return false;
		}
		else
		{
			(*tokPtr)++;
			skipObject(tokPtr);
		}

	}

	if (!dataTok)
	{
		missingKey(JSON_DATA, __LINE__, __FILE__, &RSSL_OMMSTR_DT_ARRAY);
		return false;
	}

	if (!hasType)
	{
		missingKey(JSON_TYPE, __LINE__, __FILE__, &RSSL_OMMSTR_DT_ARRAY);
		return false;
	}

	if ((_rsslRet = rsslEncodeArrayInit(&_iter, &array)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	int numEntries = dataTok->size;
	++dataTok; // Move to first entry in array
	for (i = 0; i < numEntries; i++)
	{
		voidPtr = 0;
		bufPtr = &buf;

		if(!i) {
			typeCheck = dataTok->type;
		}

		if(dataTok->type != typeCheck) {
			typeMismatch(dataTok, typeCheck, dataTok->type, __LINE__, __FILE__, &JSON_DATA);
			return false;
		}

		if (processPrimitive(array.primitiveType, &dataTok, &bufPtr, &voidPtr) == false)
			return false;
		if (_rsslRet = rsslEncodeArrayEntry(&_iter, bufPtr, voidPtr) < RSSL_RET_SUCCESS)
		{
			error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
			return false;
		}
	}

	if ((_rsslRet = rsslEncodeArrayComplete(&_iter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//
// Helper functions
//
//////////////////////////////////////////////////////////////////////
bool jsonToRwfSimple::processKey(jsmntok_t ** const tokPtr, RsslMsgKey *keyPtr, RsslUInt8 domain, jsmntok_t ** const attribTokPtr)
{

	jsmntok_t *keyTok = *tokPtr;

	// Default to 1 for nameType
	keyPtr->nameType = 1;
	RsslUInt16 flags = RSSL_MKF_HAS_NAME_TYPE;

	if ((*tokPtr)->type != JSMN_OBJECT)
		return false;

	(*tokPtr)++;

	while((*tokPtr) < _tokensEndPtr
			&& (*tokPtr)->end < keyTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_KEY);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'S':
			{
				if (compareStrings(*tokPtr, JSON_KEY_SERVICE)) //Service
				{
					(*tokPtr)++;
					switch ((*tokPtr)->type)
					{
						case JSMN_STRING: //Service Name
						{
							if (_rsslServiceNameToIdCallback)
							{
								RsslBuffer serviceName = { (*tokPtr)->end - (*tokPtr)->start, &_jsonMsg[(*tokPtr)->start] };
								if (_rsslServiceNameToIdCallback(&serviceName, _closure, &keyPtr->serviceId) != RSSL_RET_SUCCESS)
									keyPtr->serviceId = 0;
							}
							break;
						}
						case JSMN_PRIMITIVE: //Service Id
							keyPtr->serviceId = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start], &_jsonMsg[(*tokPtr)->end]);
							break;
						default:
							unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_KEY_SERVICE);
							return false;
					}

					if (!keyPtr->serviceId)
					{
						unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_SERVICE);
						return false;
					}

					flags |= RSSL_MKF_HAS_SERVICE_ID;
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_KEY);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'N':
			{
				if (compareStrings(*tokPtr, JSON_KEY_NAME))
				{
					(*tokPtr)++;
					switch ((*tokPtr)->type)
					{
					case JSMN_STRING:
						{
							keyPtr->name.length =  (*tokPtr)->end - (*tokPtr)->start;
							keyPtr->name.data = &_jsonMsg[(*tokPtr)->start];
							flags |=  RSSL_MKF_HAS_NAME;
							(*tokPtr)++;
							break;
						}
					case JSMN_ARRAY:
						{
							_batchReqTokPtr = *tokPtr;
							skipObject(tokPtr);
							break;
						}
					default:
						{
							unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME);
							return false;
						}
					}
				}
				else if (compareStrings(*tokPtr, JSON_KEY_NAME_TYPE))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type == JSMN_PRIMITIVE)
					{
						keyPtr->nameType =
							rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
										  &_jsonMsg[(*tokPtr)->end]);
						flags |= RSSL_MKF_HAS_NAME_TYPE;
						(*tokPtr)++;
					}
					else if ((*tokPtr)->type == JSMN_STRING)
					{
						switch (_jsonMsg[(*tokPtr)->start])
						{
						case 'R':
							if (_jsonMsg[(*tokPtr)->start] == RDM_OMMSTR_INSTRUMENT_NAME_TYPE_RIC.data[0])
							{
								if (compareStrings(*tokPtr, RDM_OMMSTR_INSTRUMENT_NAME_TYPE_RIC))
								{
									keyPtr->nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
								}
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
									return false;
								}
								break;
							}
						case 'N':
							{
								if (compareStrings(*tokPtr, RDM_OMMSTR_LOGIN_USER_NAME))
								{
									keyPtr->nameType = RDM_LOGIN_USER_NAME;
								}
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
									return false;
								}
								break;
							}
						case 'A':
							{
								if (compareStrings(*tokPtr, RDM_OMMSTR_LOGIN_USER_AUTHENTICATION_TOKEN))
								{
									keyPtr->nameType = RDM_LOGIN_USER_AUTHN_TOKEN;
								}
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
									return false;
								}
								break;
							}
						case 'E':
							{
								if (compareStrings(*tokPtr, RDM_OMMSTR_LOGIN_USER_EMAIL_ADDRESS))
								{
									keyPtr->nameType = RDM_LOGIN_USER_EMAIL_ADDRESS;
								}
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
									return false;
								}
								break;
							}
						case 'T':
							{
								if (compareStrings(*tokPtr, RDM_OMMSTR_LOGIN_USER_TOKEN))
								{
									keyPtr->nameType = RDM_LOGIN_USER_TOKEN;
								}
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
									return false;
								}
								break;
							}
						case 'C':
							{
								if (compareStrings(*tokPtr, RDM_OMMSTR_LOGIN_USER_COOKIE))
								{
									keyPtr->nameType = RDM_LOGIN_USER_COOKIE;
								}
								else if (compareStrings(*tokPtr, RDM_OMMSTR_INSTRUMENT_NAME_TYPE_CONTRIBUTOR))
								{
									keyPtr->nameType = RDM_INSTRUMENT_NAME_TYPE_CONTRIBUTOR;
								}
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
									return false;
								}
								break;
							}
						case 'U':
							{
								if (compareStrings(*tokPtr, RDM_OMMSTR_INSTRUMENT_NAME_TYPE_UNSPECIFIED))
								{
									keyPtr->nameType = RDM_INSTRUMENT_NAME_TYPE_UNSPECIFIED;
								}
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
									return false;
								}
								break;
							}
						default:
							{
								unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
								return false;
							}
						}
						(*tokPtr)++;
					}
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_KEY_NAME_TYPE);
						return false;
					}
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_KEY);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'F':
			{
				if (compareStrings(*tokPtr, JSON_KEY_FILTER))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_KEY_FILTER);
						return false;
					}
					keyPtr->filter =
						rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
									  &_jsonMsg[(*tokPtr)->end]);
					flags |= RSSL_MKF_HAS_FILTER;
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_KEY);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'E':
			{
				if (compareStrings(*tokPtr, JSON_ELEMENTS))
				{
					if (flags & RSSL_MKF_HAS_ATTRIB)
					{
						// MJD Error - already had data
					}
					flags |= RSSL_MKF_HAS_ATTRIB;
					keyPtr->attribContainerType = RSSL_DT_ELEMENT_LIST;
					(*tokPtr)++;
					*attribTokPtr = *tokPtr;
					skipObject(tokPtr);
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_KEY);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
        case 'I':
            {
                if (compareStrings(*tokPtr, JSON_KEY_IDENTIFIER))
                {
                    (*tokPtr)++;
                    if ((*tokPtr)->type != JSMN_PRIMITIVE)
                    {
                        unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__);
                        return false;
                    }

                    keyPtr->identifier = rtr_atoi_size(&_jsonMsg[(*tokPtr)->start],
                                                       &_jsonMsg[(*tokPtr)->end]);
                    flags |= RSSL_MKF_HAS_IDENTIFIER;
                    (*tokPtr)++;
                }
                else
                {
                    unexpectedParameter(*tokPtr, __LINE__, __FILE__);
                    return false;
                }
				break;
            }
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_KEY);
					return false;
				}
				else
				{
				(*tokPtr)++;
				skipObject(tokPtr);
			}
		}
	}
	}
	if ((keyPtr->nameType == RDM_LOGIN_USER_COOKIE || keyPtr->nameType == 5) &&
		!(flags & RSSL_MKF_HAS_NAME))
	{
		keyPtr->name.length =  1;
		keyPtr->name.data = &_nullUserName;
		flags |=  RSSL_MKF_HAS_NAME;
	}

	if ( !(flags & RSSL_MKF_HAS_SERVICE_ID) &&
		 domain != RSSL_DMT_SOURCE && domain != RSSL_DMT_LOGIN && _defaultServiceId != 0)
	{
		keyPtr->serviceId = _defaultServiceId;
		flags |= RSSL_MKF_HAS_SERVICE_ID;
	}

	keyPtr->flags = flags;
	return true;
}
// QOS
bool jsonToRwfSimple::populateQos(jsmntok_t ** const tokPtr, RsslQos *qosPtr)
{
	bool hasRate = false;
	bool hasTimeliness = false;

	int i = 0;
	if ((*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_QOS);
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
		case 'R':
			{
				if (compareStrings(*tokPtr, JSON_RATE))
				{
					hasRate = true;
					(*tokPtr)++;
					if ((*tokPtr)->type == JSMN_STRING)
					{
						switch (_jsonMsg[(*tokPtr)->start])
						{
						case 'U':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_QOS_RATE_UNSPECIFIED))
									qosPtr->rate = RSSL_QOS_RATE_UNSPECIFIED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_RATE);
									return false;
								}
								break;
							}
						case 'T':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_QOS_RATE_TICK_BY_TICK))
									qosPtr->rate = RSSL_QOS_RATE_TICK_BY_TICK;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_QOS_RATE_TIME_CONFLATED))
									qosPtr->rate = RSSL_QOS_RATE_TIME_CONFLATED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_RATE);
									return false;
								}
								break;
							}
						case 'J':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_QOS_RATE_JIT_CONFLATED))
									qosPtr->rate = RSSL_QOS_RATE_JIT_CONFLATED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_RATE);
									return false;
								}
								break;
							}
						default:
							{
								unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_RATE);
								return false;
							}
						}
						(*tokPtr)++;
					}
					else if ((*tokPtr)->type == JSMN_PRIMITIVE)
					{
						qosPtr->rate = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
													  &_jsonMsg[(*tokPtr)->end]);
						(*tokPtr)++;
					}
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_RATE);
						return false;
					}
				}
				else if (compareStrings(*tokPtr, JSON_RATEINFO))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type == JSMN_PRIMITIVE)
					{
						qosPtr->rateInfo = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
														  &_jsonMsg[(*tokPtr)->end]);
						(*tokPtr)++;
					}
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_RATEINFO);
						return false;
					}
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_QOS);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'T':
			{
				if (compareStrings(*tokPtr, JSON_TIMELINESS))
				{
					hasTimeliness = true;
					(*tokPtr)++;
					if ((*tokPtr)->type == JSMN_STRING)
					{
						switch (_jsonMsg[(*tokPtr)->start])
						{
						case 'U':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_QOS_TIME_UNSPECIFIED))
									qosPtr->timeliness = RSSL_QOS_TIME_UNSPECIFIED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_TIMELINESS);
									return false;
								}
								break;
							}
						case 'R':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_QOS_TIME_REALTIME))
									qosPtr->timeliness = RSSL_QOS_TIME_REALTIME;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_TIMELINESS);
									return false;
								}
								break;
							}

						case 'D':
						{
							if (compareStrings(*tokPtr, RSSL_OMMSTR_QOS_TIME_DELAYED_UNKNOWN))
								qosPtr->timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
							else if (compareStrings(*tokPtr, RSSL_OMMSTR_QOS_TIME_DELAYED))
								qosPtr->timeliness = RSSL_QOS_TIME_DELAYED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_TIMELINESS);
									return false;
								}
							break;
						}
						default:
							{
								unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_TIMELINESS);
								return false;
							}
						}
						(*tokPtr)++;
					}
					else if ((*tokPtr)->type == JSMN_PRIMITIVE)
					{
						qosPtr->timeliness = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
															&_jsonMsg[(*tokPtr)->end]);
						(*tokPtr)++;
					}
					else
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_TIMELINESS);
						return false;
					}
				}
				else if (compareStrings(*tokPtr, JSON_TIMEINFO))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type == JSMN_PRIMITIVE)
					{
						qosPtr->timeInfo = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
														  &_jsonMsg[(*tokPtr)->end]);
						(*tokPtr)++;
					}
					else
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_TIMEINFO);
						return false;
					}
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_QOS);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		case 'D':
			{
				if (compareStrings(*tokPtr, JSON_DYNAMIC))
				{
					(*tokPtr)++;
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_DYNAMIC);
						return false;
					}
					if (isTokenTrue(*tokPtr))
					{
						qosPtr->dynamic;
					}
					(*tokPtr)++;
				}
				else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_QOS);
					return false;
				}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
				}
				break;
			}
		default:
			{
				if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_QOS);
				return false;
			}
				else
				{
					(*tokPtr)++;
					skipObject(tokPtr);
		}
	}
		}
	}
	if(!hasRate) {
		missingKey(JSON_RATE, __LINE__, __FILE__, &JSON_QOS);
		return false;
	}
	if(!hasTimeliness) {
		missingKey(JSON_TIMELINESS, __LINE__, __FILE__, &JSON_QOS);
		return false;
	}
	return true;
}

bool jsonToRwfSimple::populateState(jsmntok_t ** const tokPtr, RsslState* statePtr)
{
	int len = 0;
	if ( (*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_STATE);
		return false;
	}

	jsmntok_t *stateTok = *tokPtr;
	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < stateTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_STATE);
			return false;
		}
		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'S':	// Stream State
		{
			if (!compareStrings(*tokPtr, JSON_STREAM) && _flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			{
				unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_STATE);
				return false;
			}
			(*tokPtr)++;
			switch ((*tokPtr)->type)
			{
			case JSMN_PRIMITIVE:
				{
					if ( (*tokPtr)->end - (*tokPtr)->start == 1 )
						statePtr->streamState = _jsonMsg[(*tokPtr)->start] - 0x30;
					else
					{
						unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_STREAM);
						return false;
					}
					break;
				}
			case JSMN_STRING:
				{
					switch (_jsonMsg[(*tokPtr)->start])
					{
					case 'O':
						{
							if (compareStrings(*tokPtr, RSSL_OMMSTR_STREAM_OPEN))
								statePtr->streamState = RSSL_STREAM_OPEN;
							else
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_STREAM);
								return false;
							}
							break;
						}
					case 'C':
						{
							if (compareStrings(*tokPtr, RSSL_OMMSTR_STREAM_CLOSED))
								statePtr->streamState = RSSL_STREAM_CLOSED;
							else if (compareStrings(*tokPtr, RSSL_OMMSTR_STREAM_CLOSED_RECOVER))
								statePtr->streamState = RSSL_STREAM_CLOSED_RECOVER;
							else
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_STREAM);
								return false;
							}
							break;
						}
					case 'U':
						{
							if (compareStrings(*tokPtr, RSSL_OMMSTR_STREAM_UNSPECIFIED))
								statePtr->streamState = RSSL_STREAM_UNSPECIFIED;
							else
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_STREAM);
								return false;
							}
							break;
						}
					case 'N':
						{
							if (compareStrings(*tokPtr, RSSL_OMMSTR_STREAM_NON_STREAMING))
								statePtr->streamState = RSSL_STREAM_NON_STREAMING;
							else
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_STREAM);
								return false;
							}
							break;
						}
					case 'R':
						{
							if (compareStrings(*tokPtr, RSSL_OMMSTR_STREAM_REDIRECTED))
								statePtr->streamState = RSSL_STREAM_REDIRECTED;
							else
							{
								unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_STREAM);
								return false;
							}
							break;
						}
					default:
						{
							unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_STREAM);
							return false;
						}
					}
					break;
				}
			default:
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_STREAM);
					return false;
				}
			}
			(*tokPtr)++;
			break;
		}

		case 'D':	// Data State
			{
				if (!compareStrings(*tokPtr, JSON_DATA) && _flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_STATE);
					return false;
				}
				(*tokPtr)++;
				switch ((*tokPtr)->type)
				{
				case JSMN_PRIMITIVE:
					{
						if ( (*tokPtr)->end - (*tokPtr)->start == 1 )
							statePtr->dataState = _jsonMsg[(*tokPtr)->start] - 0x30;
						else
						{
							unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_DATA);
							return false;
						}
						break;
					}
				case JSMN_STRING:
					{
						if (compareStrings(*tokPtr, RSSL_OMMSTR_DATA_OK))
								statePtr->dataState = RSSL_DATA_OK;
						else if (compareStrings(*tokPtr, RSSL_OMMSTR_DATA_SUSPECT))
								statePtr->dataState = RSSL_DATA_SUSPECT;
						else if (compareStrings(*tokPtr, RSSL_OMMSTR_DATA_NO_CHANGE))
								statePtr->dataState = RSSL_DATA_NO_CHANGE;
						else
						{
							unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_DATA);
							return false;
						}
						break;

					}
				default:
					if ((*tokPtr)->type != JSMN_PRIMITIVE)
					{
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_DATA);
						return false;
					}
				}
			}
			(*tokPtr)++;
			break;

		case 'C':	// State Code
			{
				if (!compareStrings(*tokPtr, JSON_CODE) && _flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_STATE);
					return false;
				}
				(*tokPtr)++;

				switch ((*tokPtr)->type)
				{
				case JSMN_PRIMITIVE:
					{
						len = (*tokPtr)->end - (*tokPtr)->start;
						statePtr->code = 0;
						switch (len)
						{
						case 2:
							statePtr->code = (((_jsonMsg[(*tokPtr)->start] - 0x30) * 10) +
											  (_jsonMsg[(*tokPtr)->start+1] - 0x30));
							break;
						case 1:
							statePtr->code = _jsonMsg[(*tokPtr)->start] - 0x30;
							break;
						default:
							unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
							return false;
						}
						break;
					}
				case JSMN_STRING:
					{
						switch (_jsonMsg[(*tokPtr)->start])
						{
						case 'N':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_NONE))
									statePtr->code = RSSL_SC_NONE;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_NOT_FOUND))
									statePtr->code = RSSL_SC_NOT_FOUND;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_NOT_ENTITLED))
									statePtr->code = RSSL_SC_NOT_ENTITLED;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_NO_RESOURCES))
									statePtr->code = RSSL_SC_NO_RESOURCES;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_NOT_OPEN))
									statePtr->code = RSSL_SC_NOT_OPEN;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_NON_UPDATING_ITEM))
									statePtr->code = RSSL_SC_NON_UPDATING_ITEM;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ))
									statePtr->code = RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'T':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_TIMEOUT))
									statePtr->code = RSSL_SC_TIMEOUT;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_TOO_MANY_ITEMS))
									statePtr->code = RSSL_SC_TOO_MANY_ITEMS;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'I':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_INVALID_ARGUMENT))
									statePtr->code = RSSL_SC_INVALID_ARGUMENT;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_INVALID_VIEW))
									statePtr->code = RSSL_SC_INVALID_VIEW;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'U':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_USAGE_ERROR))
									statePtr->code = RSSL_SC_USAGE_ERROR;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_UNSUPPORTED_VIEW_TYPE))
									statePtr->code = RSSL_SC_UNSUPPORTED_VIEW_TYPE;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_UNABLE_TO_REQUEST_AS_BATCH))
									statePtr->code = RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_USER_UNKNOWN_TO_PERM_SYS))
									statePtr->code = RSSL_SC_USER_UNKNOWN_TO_PERM_SYS;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_DACS_USER_ACCESS_TO_APP_DENIED))
									statePtr->code = RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'P':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_PREEMPTED))
									statePtr->code = RSSL_SC_PREEMPTED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'J':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_JIT_CONFLATION_STARTED))
									statePtr->code = RSSL_SC_JIT_CONFLATION_STARTED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'R':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_REALTIME_RESUMED))
									statePtr->code = RSSL_SC_REALTIME_RESUMED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'F':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_FAILOVER_STARTED))
									statePtr->code = RSSL_SC_FAILOVER_STARTED;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_FAILOVER_COMPLETED))
									statePtr->code = RSSL_SC_FAILOVER_COMPLETED;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_FULL_VIEW_PROVIDED))
									statePtr->code = RSSL_SC_FULL_VIEW_PROVIDED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'G':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_GAP_DETECTED))
									statePtr->code = RSSL_SC_GAP_DETECTED;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_GAP_FILL))
									statePtr->code = RSSL_SC_GAP_FILL;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'A':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_ALREADY_OPEN))
									statePtr->code = RSSL_SC_ALREADY_OPEN;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_APP_AUTHORIZATION_FAILED))
									statePtr->code = RSSL_SC_APP_AUTHORIZATION_FAILED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'S':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_SOURCE_UNKNOWN))
									statePtr->code = RSSL_SC_SOURCE_UNKNOWN;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'E':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_EXCEEDED_MAX_MOUNTS_PER_USER))
									statePtr->code = RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER;
								else if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_ERROR))
									statePtr->code = RSSL_SC_ERROR;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'D':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_DACS_DOWN))
									statePtr->code = RSSL_SC_DACS_DOWN;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						case 'M':
							{
								if (compareStrings(*tokPtr, RSSL_OMMSTR_SC_DACS_MAX_LOGINS_REACHED))
									statePtr->code = RSSL_SC_DACS_MAX_LOGINS_REACHED;
								else
								{
									unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
									return false;
								}
								break;
							}
						default:
							{
								unexpectedParameter(*tokPtr, __LINE__, __FILE__, &JSON_CODE);
								return false;
							}
						}
						break;
					}
				default:
					{
						unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_CODE);
						return false;
					}
				}
				(*tokPtr)++;
				break;
			}
		case 'T':	// Text
			{
				if (!compareStrings(*tokPtr, JSON_TEXT) && _flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
				{
					unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_STATE);
					return false;
				}
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
				unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_TEXT);
				return false;
			}
		default:
			if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			{
				unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_STATE);
			return false;
		}
			else
			{
				(*tokPtr)++;
				skipObject(tokPtr);
	}
		}
	}
	return true;
}

bool jsonToRwfSimple::processEnumeration(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	if ((*tokPtr)->type == JSMN_STRING && (_flags & JSON_FLAG_ALLOW_ENUM_DISPLAY_STRINGS))
	{
		/* If the type is string, the enum may be expanded. Converting back to RWF is not supported, but we may
		 * blank the entry for use in testing. */
		*ptrVoidPtr = 0;
  		_bufVar.length = 0;
		_bufVar.data = 0;
		*ptrBufPtr = &_bufVar;
		(*tokPtr)++;
		return true;
	}
	else
		return processInteger(tokPtr, ptrBufPtr, ptrVoidPtr);
}

bool jsonToRwfSimple::populatePostUserInfo(jsmntok_t ** const tokPtr, RsslPostUserInfo *userInfoPtr)
{
	bool hasAddress = false;
	bool hasUserId = false;
	int len = 0;
	if ( (*tokPtr)->type != JSMN_OBJECT)
	{
		unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
		return false;
	}

	jsmntok_t *infoTok = *tokPtr;
	(*tokPtr)++;
	while( (*tokPtr) < _tokensEndPtr &&
		   (*tokPtr)->end < infoTok->end)
	{
		if ((*tokPtr)->type != JSMN_STRING)
		{
			unexpectedTokenType(JSMN_STRING, *tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
			return false;
		}

		switch (_jsonMsg[(*tokPtr)->start])
		{
		case 'A':	// Post User Addr
		{
			if (compareStrings(*tokPtr, JSON_ADDRESS))
			{
				hasAddress = true;
				(*tokPtr)++;
				switch ((*tokPtr)->type)
				{
					case JSMN_STRING:
					{
						RsslBuffer buf;
						buf.data = &_jsonMsg[(*tokPtr)->start];
						buf.length = (*tokPtr)->end - (*tokPtr)->start;

						if (rsslIPAddrBufferToUInt(&userInfoPtr->postUserAddr, &buf) < RSSL_RET_SUCCESS)
						{
							error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
							return false;
						}
						break;
					}
					case JSMN_PRIMITIVE:
						userInfoPtr->postUserAddr = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
											   &_jsonMsg[(*tokPtr)->end]);
						break;
					default:
						unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_ADDRESS);
						return false;
				}
			}
			else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			{
				unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
				return false;
			}
			else
			{
				(*tokPtr)++;
			}
			(*tokPtr)++;
				break;
			}
		case 'U':	//Post UserId
		{
			if (compareStrings(*tokPtr, JSON_USERID))
			{
				hasUserId = true;
				(*tokPtr)++;
				if ((*tokPtr)->type != JSMN_PRIMITIVE)
				{
					unexpectedTokenType(JSMN_PRIMITIVE, *tokPtr, __LINE__, __FILE__, &JSON_USERID);
					return false;
				}
				userInfoPtr->postUserId = rtr_atoi_size( &_jsonMsg[(*tokPtr)->start],
									 &_jsonMsg[(*tokPtr)->end]);
			}
			else if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			{
				unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
				return false;
			}
			else
			{
				(*tokPtr)++;
			}
			(*tokPtr)++;
				break;
			}
		default:
			if (_flags & JSON_FLAG_CATCH_UNEXPECTED_KEYS)
			{
				unexpectedKey(*tokPtr, __LINE__, __FILE__, &JSON_POSTUSERINFO);
			return false;
		}
			else
			{
				(*tokPtr)++;
				skipObject(tokPtr);
			}
		}
	}
	if(!hasAddress) {
		missingKey(JSON_ADDRESS, __LINE__, __FILE__, &JSON_POSTUSERINFO);
			return false;
		}
	if(!hasUserId) {
		missingKey(JSON_USERID, __LINE__, __FILE__, &JSON_POSTUSERINFO);
		return false;
	}
	return true;
}

bool jsonToRwfSimple::processDate(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{

	*ptrBufPtr = 0;
	*ptrVoidPtr = 0;
	RsslBuffer buf;

	rsslClearDate(&_dateVar);

	switch ((*tokPtr)->type)
	{
	case JSMN_STRING:
		{
			buf.data = &_jsonMsg[(*tokPtr)->start];
			buf.length = (*tokPtr)->end - (*tokPtr)->start;
			if ((_rsslRet = rsslDateStringToDate(&_dateVar, &buf) < RSSL_RET_SUCCESS))
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
			*ptrVoidPtr = &_dateVar;
			(*tokPtr)++;
			return true;
		}
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
			// MJD Todo
			return false;
			break;
		}
	default:
		{
			unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
}

bool jsonToRwfSimple::processTime(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	*ptrBufPtr = 0;
	*ptrVoidPtr = 0;
	RsslBuffer buf;

	rsslClearTime(&_timeVar);

	switch ((*tokPtr)->type)
	{
	case JSMN_STRING:
		{
			buf.data = &_jsonMsg[(*tokPtr)->start];
			buf.length = (*tokPtr)->end - (*tokPtr)->start;
			if ((_rsslRet = rsslTimeStringToTime(&_timeVar, &buf) < RSSL_RET_SUCCESS))
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
			*ptrVoidPtr = &_timeVar;
			(*tokPtr)++;
			return true;
		}
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
			// MJD Todo
			return false;
			break;
		}
	default:
		{
			unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
}

bool jsonToRwfSimple::processDateTime(jsmntok_t ** const tokPtr, RsslBuffer ** const ptrBufPtr, void** const  ptrVoidPtr)
{
	*ptrBufPtr = 0;
	*ptrVoidPtr = 0;
	RsslBuffer buf;

	rsslClearDateTime(&_dateTimeVar);

	switch ((*tokPtr)->type)
	{
	case JSMN_STRING:
		{
			buf.data = &_jsonMsg[(*tokPtr)->start];
			buf.length = (*tokPtr)->end - (*tokPtr)->start;
			if ((_rsslRet = rsslDateTimeStringToDateTime(&_dateTimeVar, &buf) < RSSL_RET_SUCCESS))
			{
				error(RSSL_ENCODE_ERROR, __LINE__, __FILE__);
				return false;
			}
			*ptrVoidPtr = &_dateTimeVar;
			(*tokPtr)++;
			return true;
		}
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
			// MJD Todo
			return false;
		}
	default:
		{
			unexpectedTokenType(JSMN_OBJECT, *tokPtr, __LINE__, __FILE__);
			return false;
		}
	}
}

bool jsonToRwfSimple::getDataType(jsmntok_t * tok, RsslContainerType* formatPtr)
{
	switch (tok->type)
	{
	case JSMN_STRING:
		{
			switch (_jsonMsg[tok->start])
			{
			case 'D':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_DOUBLE))
						*formatPtr = RSSL_DT_DOUBLE;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_DATE))
						*formatPtr = RSSL_DT_DATE;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_DATETIME))
						*formatPtr = RSSL_DT_DATETIME;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'R':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_REAL))
						*formatPtr = RSSL_DT_REAL;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_RMTES_STRING))
						*formatPtr = RSSL_DT_RMTES_STRING;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'B':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_BUFFER))
						*formatPtr = RSSL_DT_BUFFER;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'Q':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_QOS))
						*formatPtr = RSSL_DT_QOS;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'T':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_TIME))
						*formatPtr = RSSL_DT_TIME;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'I':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_INT))
						*formatPtr = RSSL_DT_INT;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'N':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_NO_DATA))
						*formatPtr = RSSL_DT_NO_DATA;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'O':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_OPAQUE))
						*formatPtr = RSSL_DT_OPAQUE;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'X':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_XML))
						*formatPtr = RSSL_DT_XML;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'F':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_FLOAT))
						*formatPtr = RSSL_DT_FLOAT;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_FIELD_LIST))
						*formatPtr = RSSL_DT_FIELD_LIST;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_FILTER_LIST))
						*formatPtr = RSSL_DT_FILTER_LIST;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'E':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_ELEMENT_LIST))
						*formatPtr = RSSL_DT_ELEMENT_LIST;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_ENUM))
						*formatPtr = RSSL_DT_ENUM;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'A':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_ARRAY))
						*formatPtr = RSSL_DT_ARRAY;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_ASCII_STRING))
						*formatPtr = RSSL_DT_ASCII_STRING;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_ANSI_PAGE))
						*formatPtr = RSSL_DT_ANSI_PAGE;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'V':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_VECTOR))
						*formatPtr = RSSL_DT_VECTOR;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'M':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_MAP))
						*formatPtr = RSSL_DT_MAP;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_MSG))
						*formatPtr = RSSL_DT_MSG;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'S':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_STATE))
						*formatPtr = RSSL_DT_STATE;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_SERIES))
						*formatPtr = RSSL_DT_SERIES;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'J':
				{
					if (compareStrings(tok, RSSL_OMMSTR_DT_JSON))
						*formatPtr = RSSL_DT_JSON;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			case 'U':
						{
					if (compareStrings(tok, RSSL_OMMSTR_DT_UINT))
						*formatPtr = RSSL_DT_UINT;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_UTF8_STRING))
						*formatPtr = RSSL_DT_UTF8_STRING;
					else if (compareStrings(tok, RSSL_OMMSTR_DT_UNKNOWN))
						*formatPtr = RSSL_DT_UNKNOWN;
					else
					{
						unexpectedParameter(tok, __LINE__, __FILE__);
						return false;
					}
					break;
				}
			default:
				{
					unexpectedParameter(tok, __LINE__, __FILE__);
					return false;
				}
			}
			break;
		}
	case JSMN_PRIMITIVE:
		{
			*formatPtr =
				rtr_atoi_size( &_jsonMsg[tok->start],
							   &_jsonMsg[tok->end]) + RSSL_DT_CONTAINER_TYPE_MIN;
			break;
		}
	default:
		{
			unexpectedTokenType(JSMN_PRIMITIVE, tok, __LINE__, __FILE__);
			return false;
		}
	}
	return true;
}

bool jsonToRwfSimple::getContainerType(jsmntok_t * tok, RsslContainerType* formatPtr)
{
	if (tok->type != JSMN_STRING)
	{
		unexpectedTokenType(JSMN_STRING, tok, __LINE__, __FILE__);
		return false;
	}

	switch (_jsonMsg[tok->start])
	{
		case 'F':
			if (compareStrings(tok, JSON_FIELDS) || compareStrings(tok, RSSL_OMMSTR_DT_FIELD_LIST))
				*formatPtr = RSSL_DT_FIELD_LIST;
			else if (compareStrings(tok, RSSL_OMMSTR_DT_FILTER_LIST))
				*formatPtr = RSSL_DT_FILTER_LIST;
			else
			{
				unexpectedKey(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'E':
			if (compareStrings(tok, JSON_ELEMENTS) || compareStrings(tok, RSSL_OMMSTR_DT_ELEMENT_LIST))
				*formatPtr = RSSL_DT_ELEMENT_LIST;
			else
			{
				unexpectedKey(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'J':
			if (compareStrings(tok, RSSL_OMMSTR_DT_JSON))
				*formatPtr = RSSL_DT_JSON;
			else
			{
				unexpectedKey(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'M':
			if (compareStrings(tok, RSSL_OMMSTR_DT_MAP))
				*formatPtr = RSSL_DT_MAP;
			else if (compareStrings(tok, JSON_MESSAGE))
				*formatPtr = RSSL_DT_MSG;
			else
			{
				unexpectedKey(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'O':
			if (compareStrings(tok, RSSL_OMMSTR_DT_OPAQUE))
				*formatPtr = RSSL_DT_OPAQUE;
			else
			{
				unexpectedKey(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'V':
			if (compareStrings(tok, RSSL_OMMSTR_DT_VECTOR))
				*formatPtr = RSSL_DT_VECTOR;
			else
			{
				unexpectedKey(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'S':
			if (compareStrings(tok, RSSL_OMMSTR_DT_SERIES))
				*formatPtr = RSSL_DT_SERIES;
			else
			{
				unexpectedKey(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'X':
			if (compareStrings(tok, RSSL_OMMSTR_DT_XML))
				*formatPtr = RSSL_DT_XML;
			else
			{
				unexpectedKey(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		default:
			unexpectedKey(tok, __LINE__, __FILE__);
			return false;
	}

	return true;
}

bool jsonToRwfSimple::getVectorAction(jsmntok_t *tok, RsslUInt8* action)
{
	if (tok->type != JSMN_STRING)
	{
		unexpectedTokenType(JSMN_STRING, tok, __LINE__, __FILE__);
		return false;
	}

	switch (_jsonMsg[tok->start])
	{
		case 'U':
			if (compareStrings(tok, RSSL_OMMSTR_VTEA_UPDATE_ENTRY))
				*action = RSSL_VTEA_UPDATE_ENTRY;
			else
			{
				unexpectedParameter(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'S':
			if (compareStrings(tok, RSSL_OMMSTR_VTEA_SET_ENTRY))
				*action = RSSL_VTEA_SET_ENTRY;
			else
			{
				unexpectedParameter(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'C':
			if (compareStrings(tok, RSSL_OMMSTR_VTEA_CLEAR_ENTRY))
				*action = RSSL_VTEA_CLEAR_ENTRY;
			else
			{
				unexpectedParameter(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'I':
			if (compareStrings(tok, RSSL_OMMSTR_VTEA_INSERT_ENTRY))
				*action = RSSL_VTEA_INSERT_ENTRY;
			else
			{
				unexpectedParameter(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		case 'D':
			if (compareStrings(tok, RSSL_OMMSTR_VTEA_DELETE_ENTRY))
				*action = RSSL_VTEA_DELETE_ENTRY;
			else
			{
				unexpectedParameter(tok, __LINE__, __FILE__);
				return false;
			}
			break;
		default:
			unexpectedParameter(tok, __LINE__, __FILE__);
			return false;
	}

	return true;
}

void jsonToRwfSimple::reset()
{
	_viewTokPtr = 0;
	_batchReqTokPtr = 0;
	_batchCloseTokPtr = 0;
	jsonToRwfBase::reset();
}

RsslBuffer* jsonToRwfSimple::errorText()
{
	if (_error == false)
		return 0;

	//memset(_errorText.data,'0',_errorTextLen);
	_errorText.length = 0;

	switch (_errorCode)
	{
		case MEM_ALLOC_FAILURE:
			_errorText.length = snprintf(_errorText.data,
										ERROR_TEXT_MAX, "JSON Converter memory allocation");
			break;
		case JSMN_PARSE_ERROR:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON parser error: %d", _jsmnError);
			break;
		case INVALID_TOKEN_TYPE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Converter Token Type error: Expected ");
			switch ( _expectedTokenType)
			{
				case JSMN_PRIMITIVE :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'PRIMITIVE'");
					break;
				case JSMN_OBJECT :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'OBJECT'");
					break;
				case JSMN_ARRAY :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'ARRAY'");
					break;
				case JSMN_STRING :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'STRING'");
					break;
			}
			if (_errorParentKey.data)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " for key '%.*s'", _errorParentKey.length, _errorParentKey.data);
			}
			if (_errorToken)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received ");
				switch ( _errorToken->type)
				{
					case JSMN_PRIMITIVE :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'PRIMITIVE'");
						break;
					case JSMN_OBJECT :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'OBJECT'");
						break;
					case JSMN_ARRAY :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'ARRAY'");
						break;
					case JSMN_STRING :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'STRING'");
						break;
				}
			}
			break;
		case UNEXPECTED_VALUE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Unexpected Value.");
			if (_errorToken)
			{
				int tokenLength = _errorToken->end - _errorToken->start;
				char* singleToken = (char*)malloc((size_t)tokenLength*sizeof(char)+1);
				memset(singleToken, 0, tokenLength + 1);
				memcpy(singleToken, &_jsonMsg[_errorToken->start], tokenLength);
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received '%s'", singleToken);
				if (_errorParentKey.data)
				{
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " for key '%.*s'", _errorParentKey.length, _errorParentKey.data);
				}
				free(singleToken);
			}
			break;
		case UNEXPECTED_KEY:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Unexpected Key.");
			if (_errorToken)
			{
				int tokenLength = _errorToken->end - _errorToken->start;
				char* singleToken = (char*)malloc((size_t)tokenLength*sizeof(char)+1);
				memset(singleToken, 0, tokenLength + 1);
				memcpy(singleToken, &_jsonMsg[_errorToken->start], tokenLength);
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received '%s'", singleToken);
				if (_errorParentKey.data)
				{
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " for key '%.*s'", _errorParentKey.length, _errorParentKey.data);
				}
				//free(singleToken);
			}
			break;
		case UNEXPECTED_FID:
				_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Unexpected FID.");
				if (_errorToken)
				{
					int tokenLength = _errorToken->end - _errorToken->start;
					char* singleToken = (char*)malloc((size_t)tokenLength * sizeof(char)+1);
					memset(singleToken, 0, tokenLength + 1);
					memcpy(singleToken, &_jsonMsg[_errorToken->start], tokenLength);
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received '%s'", singleToken);
					if (_errorParentKey.data)
					{
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " for key '%.*s'", _errorParentKey.length, _errorParentKey.data);
					}
					free(singleToken);
				}
				break;
		case MISSING_KEY:
		  _errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Missing required key ");
			if (_errorMissingKey.data)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'%s'", _errorMissingKey.data);
			}
			if (_errorParentKey.data)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " for '%.*s'", _errorParentKey.length, _errorParentKey.data);
			}
			break;
		case TYPE_MISMATCH:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Mixed Types in OMM Array: Received ");
			switch (_errorFirstType)
			{
				case JSMN_PRIMITIVE :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'PRIMITIVE'");
					break;
				case JSMN_OBJECT :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'OBJECT'");
					break;
				case JSMN_ARRAY :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'ARRAY'");
					break;
				case JSMN_STRING :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'STRING'");
					break;
			}
			_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " and ");
			switch (_errorSecondType)
			{
				case JSMN_PRIMITIVE :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'PRIMITIVE'");
					break;
				case JSMN_OBJECT :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'OBJECT'");
					break;
				case JSMN_ARRAY :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'ARRAY'");
					break;
				case JSMN_STRING :
					_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'STRING'");
					break;
			}
			if (_errorParentKey.data)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " for key '%.*s'", _errorParentKey.length, _errorParentKey.data);
			}
			break;
		case INVALID_PRIMITIVE_TYPE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON invalid primitive type.");
			if (_errorToken)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received ");
				switch ( _errorToken->type)
				{
					case JSMN_PRIMITIVE :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'PRIMITIVE'");
						break;
					case JSMN_OBJECT :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'OBJECT'");
						break;
					case JSMN_ARRAY :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'ARRAY'");
						break;
					case JSMN_STRING :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'STRING'");
						break;
				}
			}
			break;
		case INVALID_CONTAINER_TYPE:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON invalid container type.");
			if (_errorToken)
			{
				_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, " Received ");
				switch ( _errorToken->type)
				{
					case JSMN_PRIMITIVE :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'PRIMITIVE'");
						break;
					case JSMN_OBJECT :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'OBJECT'");
						break;
					case JSMN_ARRAY :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'ARRAY'");
						break;
					case JSMN_STRING :
						_errorText.length += snprintf(_errorText.data + _errorText.length, ERROR_TEXT_MAX - _errorText.length, "'STRING'");
						break;
				}
			}
			break;
		case SET_DEFINITION_ERROR:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON Set Definition error.");
			break;
		case RSSL_ENCODE_ERROR:
			_errorText.length = snprintf(_errorText.data, ERROR_TEXT_MAX, "JSON RSSL Conversion Error. RSSL error code : %d",_rsslRet);
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
