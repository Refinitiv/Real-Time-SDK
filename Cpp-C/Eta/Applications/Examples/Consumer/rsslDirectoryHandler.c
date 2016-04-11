
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

/* source directory response information */
static RsslSourceDirectoryResponseInfo sourceDirectoryResponseInfo[MAX_SOURCE_DIRECTORY_SERVICES];
/* service name requested by application */
static char* serviceName;
/* service id associated with the service name requested by application */
static RsslUInt64 serviceId = 0;
/* service name found flag */
static RsslBool serviceNameFound = RSSL_FALSE;

/*
 * Sets the service name requested by the application.
 * servicename - The service name requested by the application
 */
void setServiceName(char* servicename)
{
	serviceName = servicename;
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
	
	printf("\nSource Directory response info does not contain service Id: "RTR_LLU".\n", serviceId);
	*rsslSourceDirectoryResponseInfo = NULL;
	return RSSL_RET_FAILURE;
}

/*
 * Sends a source directory request to a channel.  This consists
 * of getting a message buffer, encoding the source directory request,
 * and sending the source directory request to the server.
 * chnl - The channel to send a source directory request to
 */
RsslRet sendSourceDirectoryRequest(RsslChannel* chnl)
{
	RsslError error;
	RsslBuffer* msgBuf = 0;

	/* get a buffer for the source directory request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* encode source directory request */
		if (encodeSourceDirectoryRequest(chnl, msgBuf, SRCDIR_STREAM_ID) != RSSL_RET_SUCCESS)
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

		/* send item request or dictionary request if dictionary not yet loaded */
		if (!isFieldDictionaryLoaded() || !isEnumTypeDictionaryLoaded() )
		{
    		if (sendDictionaryRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		else
		{
			if (sendSymbolListRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendMarketPriceItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendMarketByOrderItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendMarketByPriceItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			if (sendYieldCurveItemRequests(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
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

