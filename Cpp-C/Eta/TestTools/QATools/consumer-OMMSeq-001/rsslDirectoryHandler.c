
/*
 * This is the source directory handler for the rsslConsumer application.
 * It provides functions for sending the source directory request to a
 * provider and processing the response.  Functions for setting the service
 * name, getting the service id, and closing a source directory stream are
 * also provided.
 */

#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslMarketPriceHandler.h"
#include "rsslMarketByOrderHandler.h"
#include "rsslMarketByPriceHandler.h"
#include "rsslYieldCurveHandler.h"
#include "rsslSymbolListHandler.h"
#include "rsslSendMessage.h"
#include "rsslJsonSession.h"

/* source directory response information */
static RsslSourceDirectoryResponseInfo sourceDirectoryResponseInfo[MAX_SOURCE_DIRECTORY_SERVICES];
/* service name requested by application */
static char* serviceName;
/* service id associated with the service name requested by application */
static RsslUInt64 serviceId = 0;
/* service name found flag */
static RsslBool serviceNameFound = RSSL_FALSE;

//APIQA
static int _initialFilter = 0;
static int _reissueFilter = 0;
static RsslBool _sendGenericMsg = RSSL_FALSE;

void setDirectoryFilter(int initialFilter, int reissueFilter)
{
	_initialFilter = initialFilter;
	_reissueFilter = reissueFilter;
}
//END APIQA


/*
 * Sets the service name requested by the application.
 * servicename - The service name requested by the application
 */
void setServiceName(char* servicename)
{
	serviceName = servicename;
}

RsslRet serviceNameToIdCallback(RsslBuffer* name, RsslUInt16* Id)
{
	int i = 0;
	for (i = 0; i < MAX_SOURCE_DIRECTORY_SERVICES; i++)
	{
		if (strlen(sourceDirectoryResponseInfo[i].ServiceGeneralInfo.ServiceName) == name->length)
		{

			if (!strncmp(sourceDirectoryResponseInfo[i].ServiceGeneralInfo.ServiceName, name->data, name->length))
			{
				*Id = (RsslUInt16)sourceDirectoryResponseInfo[i].ServiceId;
			}
			return RSSL_RET_SUCCESS;
		}
	}

	return RSSL_RET_FAILURE;
}

/*
 * Returns the service id associated with service name
 * requested by the application
 */
RsslUInt64 getServiceId()
{
	return serviceId;
}

/*
*Returns whether or not the requested domain is supported by the provider
*domainId - the desired domain 
*/
RsslBool getSourceDirectoryCapabilities(RsslUInt64 domainId)
{
	int i, j;
	RsslBool domainMatch = RSSL_FALSE;

	for(i = 0; i < MAX_SOURCE_DIRECTORY_SERVICES; i++)
	{
		/*check to see if the provider is able to support the requested domain*/
		for(j = 0; j < 10; j++)
		{
			if(sourceDirectoryResponseInfo[i].ServiceGeneralInfo.Capabilities[j] == domainId)
			{
				domainMatch = RSSL_TRUE;	
			}
		}
	}

	return domainMatch;
}

RsslRet getSourceDirectoryResponseInfo(RsslUInt serviceId, RsslSourceDirectoryResponseInfo** rsslSourceDirectoryResponseInfo)
{
	int i;
	for(i = 0; i < MAX_SOURCE_DIRECTORY_SERVICES; i++)
	{
		if(sourceDirectoryResponseInfo[i].ServiceId == serviceId)
		{
			*rsslSourceDirectoryResponseInfo = &sourceDirectoryResponseInfo[i];
			return RSSL_RET_SUCCESS;
		}
	}
	
	printf("\nSource Directory response info does not contain service Id: " RTR_LLU ".\n", serviceId);
	*rsslSourceDirectoryResponseInfo = NULL;
	return RSSL_RET_FAILURE;
}
//APIQA
RsslRet encodeSourceDirectoryRequestCustom(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId, int filter)
{
	RsslRet ret = 0;
	RsslRequestMsg msg = RSSL_INIT_REQUEST_MSG;
	RsslEncodeIterator encodeIter;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_SOURCE;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;
	msg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY;
	msg.priorityClass = 1;
	msg.priorityCount = 1;


	/* set members in msgKey */
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER;
	msg.msgBase.msgKey.filter = filter;

	/* encode message */
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsg(&encodeIter, (RsslMsg*)&msg)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsg() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

//END APIQA

/*
 * Sends a source directory request to a channel.  This consists
 * of getting a message buffer, encoding the source directory request,
 * and sending the source directory request to the server.
 * chnl - The channel to send a source directory request to
 */
//APIQA
//RsslRet sendSourceDirectoryRequest(RsslChannel* chnl)
RsslRet sendSourceDirectoryRequest(RsslChannel* chnl, RsslBool sendGenericMsg)
//END APIQA
{
	RsslError error;
	RsslBuffer* msgBuf = 0;
	//APIQA
	_sendGenericMsg = sendGenericMsg;
	//END APIQA
	/* get a buffer for the source directory request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode source directory request */
		//APIQA
		//if (encodeSourceDirectoryRequest(chnl, msgBuf, SRCDIR_STREAM_ID) != RSSL_RET_SUCCESS)
		if (encodeSourceDirectoryRequestCustom(chnl, msgBuf, SRCDIR_STREAM_ID, _initialFilter) != RSSL_RET_SUCCESS)
		//END APIQA
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nencodeSourceDirectoryRequest() failed\n");
			return RSSL_RET_FAILURE;
		}


		/* send source directory request */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}
//APIQA
RsslRet sendGenericMsgOnDirectory(RsslChannel* chnl)
{
	RsslRet ret = 0;
	RsslGenericMsg msg = RSSL_INIT_GENERIC_MSG;
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mEntry = RSSL_INIT_MAP_ENTRY;
	RsslElementList rsslElementList = RSSL_INIT_ELEMENT_LIST;
	RsslBuffer serviceNameBuffer;
	RsslEncodeIterator encodeIter;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;
	RsslBuffer* msgBuf;
	RsslError error;
	RsslUInt sourceMirroringMode = 1;

	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);

	/* set-up message */
	msg.msgBase.msgClass = RSSL_MC_GENERIC;
	msg.msgBase.domainType = RSSL_DMT_SOURCE;
	msg.msgBase.containerType = RSSL_DT_MAP;
	msg.msgBase.msgKey.name.length = 14;
	msg.msgBase.msgKey.name.data = "ConsumerStatus";
	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	msg.flags = RSSL_GNMF_HAS_MSG_KEY | RSSL_GNMF_MESSAGE_COMPLETE;

	/* get a buffer for the login request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	/* StreamId */
	msg.msgBase.streamId = SRCDIR_STREAM_ID;

	/* encode message */
	if ((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslSetEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode map */
	map.keyPrimitiveType = RSSL_DT_UINT;
	map.containerType = RSSL_DT_ELEMENT_LIST;
	if ((ret = rsslEncodeMapInit(&encodeIter, &map, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode map entry */
	mEntry.action = RSSL_MPEA_ADD_ENTRY;
	if ((ret = rsslEncodeMapEntryInit(&encodeIter, &mEntry, &serviceId, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* encode the element list */
	rsslElementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeElementListInit(&encodeIter, &rsslElementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* WarmStandbyMode */
	element.dataType = RSSL_DT_UINT;
	element.name = RSSL_ENAME_SOURCE_MIRROR_MODE;
	if ((ret = rsslEncodeElementEntry(&encodeIter, &element, &sourceMirroringMode)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementEntry() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode element list */
	if ((ret = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeElementListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode map entry */
	if ((ret = rsslEncodeMapEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode map */
	if ((ret = rsslEncodeMapComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMapComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}
	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	if (ret != RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("\nsendGenericMsgOnLogin() failed with return code: %d\n", ret);
		return ret;
	}

	/* send login request */
	if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

//END APIQA
/*
 * Processes a source directory response.  This consists of calling
 * decodeSourceDirectoryResponse() to decode the response and calling
 * sendDictionaryRequest() to send the dictionary request or calling
 * sendItemRequest() to send the item request if the service name
 * requested by application is found.
 * chnl - The channel of the response
 * msg - The partially decoded message
 * dIter - The decode iterator
 */
RsslRet processSourceDirectoryResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslState *pState = 0;
	int i;
	char tempData[1024];
	RsslBuffer tempBuffer;

	tempBuffer.data = tempData;
	tempBuffer.length = 1024;

	switch(msg->msgBase.msgClass)
	{
	case RSSL_MC_REFRESH:
		/* first clear response info structure */
		clearSourceDirRespInfo(&sourceDirectoryResponseInfo[0]);
		/* decode source directory response */
		if (decodeSourceDirectoryResponse(&sourceDirectoryResponseInfo[0],
											dIter,
											MAX_SOURCE_DIRECTORY_SERVICES,
											MAX_CAPABILITIES,
											MAX_QOS,
											MAX_DICTIONARIES,
											MAX_LINKS) != RSSL_RET_SUCCESS)
		{
			printf("\ndecodeSourceDirectoryResponse() failed\n");
			return RSSL_RET_FAILURE;
		}

		printf("\nReceived Source Directory Response\n");

		pState = &msg->refreshMsg.state;
		rsslStateToString(&tempBuffer, pState);
		printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);

		/* check if service name received in response matches that entered by user */
		for (i = 0; i < MAX_SOURCE_DIRECTORY_SERVICES; i++)
		{
			if (strlen(sourceDirectoryResponseInfo[i].ServiceGeneralInfo.ServiceName))
			{
				printf("Received serviceName: %s\n", sourceDirectoryResponseInfo[i].ServiceGeneralInfo.ServiceName);
				/* check if name matches service name entered by user */
				/* if it does, store the service id */
				if (!strcmp(sourceDirectoryResponseInfo[i].ServiceGeneralInfo.ServiceName, serviceName))
				{
					serviceId = sourceDirectoryResponseInfo[i].ServiceId;

					/* Set the symbol list name from the source directory response */
					if(getSlNameGivenInfo() == RSSL_FALSE)
					{
						setSymbolListName(sourceDirectoryResponseInfo[i].ServiceGeneralInfo.ItemList);
					}
					serviceNameFound = RSSL_TRUE;
				}
				printf("\n");
			}
		}

		/* exit if service name entered by user cannot be found */
		if (!serviceNameFound)
		{
			printf("\nSource directory response does not contain service name: %s\n", serviceName);
			return RSSL_RET_FAILURE;
		}
		//APIQA
		if (_reissueFilter != -1)
		{
			printf("\n\n*********************************\n");
			printf("Reissue directory with filter %d*\n", _reissueFilter);
			printf("*********************************\n\n");

			RsslError error;
			RsslBuffer* msgBuf = 0;

			/* get a buffer for the source directory request */
			msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

			if (msgBuf != NULL)
			{
				/* encode source directory request */
				if (encodeSourceDirectoryRequestCustom(chnl, msgBuf, SRCDIR_STREAM_ID, _reissueFilter) != RSSL_RET_SUCCESS)
				{
					rsslReleaseBuffer(msgBuf, &error);
					printf("\nencodeSourceDirectoryRequest() failed\n");
					return RSSL_RET_FAILURE;
				}


				/* send source directory request */
				if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
			}
			else
			{
				printf("rsslGetBuffer(): Failed <%s>\n", error.text);
				return RSSL_RET_FAILURE;
			}

			_reissueFilter = -1;

			return RSSL_RET_SUCCESS;
		}

		if (_sendGenericMsg)
		{
			sendGenericMsgOnDirectory(chnl);
			return RSSL_RET_SUCCESS;
		}
		//END APIQA
		/* send item request or dictionary request if dictionary not yet loaded */
		if (!isFieldDictionaryLoaded() || !isEnumTypeDictionaryLoaded() )
		{
    		if (sendDictionaryRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			// APIQA
			/*if (sendSymbolListRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;*/
			//END APIQA
			if (sendMarketPriceItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
			//APIQA
			/*
			if (sendMarketByOrderItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendMarketByPriceItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendYieldCurveItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;*/
			//END APIQA
		}

		break;

	case RSSL_MC_UPDATE:
		printf("\nReceived Source Directory Update\n");
		break;

	case RSSL_MC_CLOSE:
		printf("\nReceived Source Directory Close\n");
		break;

	case RSSL_MC_STATUS:
		printf("\nReceived Source Directory StatusMsg\n");
		if (msg->statusMsg.flags & RSSL_STMF_HAS_STATE)
    	{
    		pState = &msg->statusMsg.state;
			rsslStateToString(&tempBuffer, pState);
			printf("	%.*s\n\n", tempBuffer.length, tempBuffer.data);
    	}
		break;

	default:
		printf("\nReceived Unhandled Source Directory Msg Class: %d\n", msg->msgBase.msgClass);
    	break;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Close the source directory stream.
 * chnl - The channel to send a source directory close to
 */
RsslRet closeSourceDirectoryStream(RsslChannel* chnl)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the source directory close */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode source directory close */
		if (encodeSourceDirectoryClose(chnl, msgBuf, SRCDIR_STREAM_ID) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nencodeSourceDirectoryClose() failed\n");
			return RSSL_RET_FAILURE;
		}

		/* send close */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

