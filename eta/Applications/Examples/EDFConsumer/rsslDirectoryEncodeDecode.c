/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/


/*
 * This file contains all source directory decode
 * functions required by the rsslSimpleEDFExample
 * application.
 */

#include "rsslDirectoryEncodeDecode.h"

#define RDM_DIRECTORY_SERVICE_EDF_ID 64

/*
 * Decodes the source directory response into the RsslSourceDirectoryResponseInfo
 * structure.  Returns success if decoding succeeds or failure if decoding fails.
 * srcDirRespInfo - The source directory response information structure
 * dIter - The decode iterator
 * maxServices - The maximum number of services that the structure holds
 * maxCapabilities - The maximum number of capabilities that the structure holds
 * maxQOS - The maximum number of QOS entries that the structure holds
 * maxDictionaries - The maximum number of dictionaries that the structure holds
 * maxLinks - The maximum number of link entries that the structure holds
 */
RsslRet decodeSourceDirectoryResponse(RsslSourceDirectoryResponseInfo* srcDirRespInfo,
										  RsslDecodeIterator* dIter,
										  RsslUInt32 maxServices,
										  RsslUInt32 maxCapabilities,
										  RsslUInt32 maxQOS,
										  RsslUInt32 maxDictionaries,
										  RsslUInt32 maxLinks)
{
	RsslRet ret = 0;
	RsslMap map;
	RsslMapEntry mEntry;
	RsslFilterList rsslFilterList;
	RsslFilterEntry filterListItem;
	int serviceCount = 0;
	RsslUInt64 serviceIdTemp;

	/* decode source directory response */

	/* decode map */
	if ((ret = rsslDecodeMap(dIter, &map)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMap() failed with return code: %d\n", ret);
		return ret;
	}

	/* source directory response key data type must be RSSL_DT_UINT */
	if (map.keyPrimitiveType != RSSL_DT_UINT)
	{
		printf("Map has incorrect keyPrimitiveType: %s", rsslDataTypeToString(map.keyPrimitiveType));
		return RSSL_RET_FAILURE;
	}

	/* decode map entries */
	/* service id is contained in map entry encKey */
	/* store service id in source directory response information */
	while ((ret = rsslDecodeMapEntry(dIter, &mEntry, &serviceIdTemp)) != RSSL_RET_END_OF_CONTAINER)
	{
		/* break out of decoding when max services reached */
		if (serviceCount == maxServices)
		{
			rsslFinishDecodeEntries(dIter);
			printf("decodeSourceDirectoryResponse() maxServices limit reached - more services in message than memory can support\n");
			break;
		}

		if (ret == RSSL_RET_SUCCESS)
		{
			if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeUInt() failed with return code: %d\n", ret);
				return ret;
			}

			srcDirRespInfo[serviceCount].ServiceId = serviceIdTemp;

			/* decode filter list */
			if ((ret = rsslDecodeFilterList(dIter, &rsslFilterList)) < RSSL_RET_SUCCESS)
			{
				printf("rsslDecodeFilterList() failed with return code: %d\n", ret);
				return ret;
			}

			/* decode filter list items */
			while ((ret = rsslDecodeFilterEntry(dIter, &filterListItem)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (ret == RSSL_RET_SUCCESS)
				{
					/* decode source directory response information */
					switch (filterListItem.id)
					{
					case RDM_DIRECTORY_SERVICE_INFO_ID:
						if ((ret = decodeServiceGeneralInfo(&srcDirRespInfo[serviceCount].ServiceGeneralInfo, dIter, maxCapabilities, maxQOS, maxDictionaries)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceGeneralInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_STATE_ID:
						if ((ret = decodeServiceStateInfo(&srcDirRespInfo[serviceCount].ServiceStateInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceStateInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_GROUP_ID:
						if ((ret = decodeServiceGroupInfo(&srcDirRespInfo[serviceCount].ServiceGroupInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceGroupInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_LOAD_ID:
						if ((ret = decodeServiceLoadInfo(&srcDirRespInfo[serviceCount].ServiceLoadInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceLoadInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_DATA_ID:
						if ((ret = decodeServiceDataInfo(&srcDirRespInfo[serviceCount].ServiceDataInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceDataInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_LINK_ID:
						if ((ret = decodeServiceLinkInfo(&srcDirRespInfo[serviceCount].ServiceLinkInfo[0], dIter, maxLinks)) != RSSL_RET_SUCCESS)
						{
							printf("decodeServiceLinkInfo() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					case RDM_DIRECTORY_SERVICE_EDF_ID:
						if ((ret = decodeEDF(&srcDirRespInfo[serviceCount].EDFInfo, dIter)) != RSSL_RET_SUCCESS)
						{
							printf("decodeEDF() failed with return code: %d\n", ret);
							return ret;
						}
						break;
					default:
						printf("Unkonwn FilterListEntry filterID: %d\n", filterListItem.id);
						return RSSL_RET_FAILURE;
					}
				}
			}
		}
		serviceCount++;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's general information into the RsslServiceGeneralInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceGeneralInfo - The service's general information structure
 * dIter - The decode iterator
 * maxCapabilities - The maximum number of capabilities that the structure holds
 * maxQOS - The maximum number of QOS entries that the structure holds
 * maxDictionaries - The maximum number of dictionaries that the structure holds
 */
RsslRet decodeServiceGeneralInfo(RsslServiceGeneralInfo* serviceGeneralInfo,
										RsslDecodeIterator* dIter,
										RsslUInt32 maxCapabilities,
										RsslUInt32 maxQOS,
										RsslUInt32 maxDictionaries)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslBuffer arrayBuffer;
	RsslUInt32 arrayCount = 0;
	RsslBool foundQoS = RSSL_FALSE;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service general information */
			/* Name */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_NAME))
			{
				/* store service name in source directory response information */
				if (element.encData.length < MAX_SRCDIR_INFO_STRLEN)
				{
					strncpy(serviceGeneralInfo->ServiceName, element.encData.data, element.encData.length);
					serviceGeneralInfo->ServiceName[element.encData.length] = '\0';
				}
				else
				{
					strncpy(serviceGeneralInfo->ServiceName, element.encData.data, MAX_SRCDIR_INFO_STRLEN - 1);
					serviceGeneralInfo->ServiceName[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
				}
			}
			/* Vendor */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_VENDOR))
			{
				if (element.encData.length < MAX_SRCDIR_INFO_STRLEN)
				{
					strncpy(serviceGeneralInfo->Vendor, element.encData.data, element.encData.length);
					serviceGeneralInfo->Vendor[element.encData.length] = '\0';
				}
				else
				{
					strncpy(serviceGeneralInfo->Vendor, element.encData.data, MAX_SRCDIR_INFO_STRLEN - 1);
					serviceGeneralInfo->Vendor[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
				}
			}
			/* IsSource */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_IS_SOURCE))
			{
				ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->IsSource);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* Capabilities */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_CAPABILITIES))
			{
				if ((ret = rsslDecodeArray(dIter, &rsslArray)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeArray() failed with return code: %d\n", ret);
					return ret;
				}
				while ((ret = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* break out of decoding array items when max capabilities reached */
					if (arrayCount == maxCapabilities)
					{
						rsslFinishDecodeEntries(dIter);
						break;
					}

					if (ret == RSSL_RET_SUCCESS)
					{
						ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->Capabilities[arrayCount]);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", ret);
							return ret;
						}
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeArrayEntry() failed with return code: %d\n", ret);
						return ret;
					}
					arrayCount++;
				}
				arrayCount = 0;
			}
			/* DictionariesProvided */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DICTIONARYS_PROVIDED))
			{
				if ((ret = rsslDecodeArray(dIter, &rsslArray)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeArray() failed with return code: %d\n", ret);
					return ret;
				}
				while ((ret = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* break out of decoding array items when max dictionaries reached */
					if (arrayCount == maxDictionaries)
					{
						rsslFinishDecodeEntries(dIter);
						break;
					}

					if (ret == RSSL_RET_SUCCESS)
					{
						if (arrayBuffer.length < MAX_SRCDIR_INFO_STRLEN)
						{
							strncpy(serviceGeneralInfo->DictionariesProvided[arrayCount],
									arrayBuffer.data,
									arrayBuffer.length);
							serviceGeneralInfo->DictionariesProvided[arrayCount][arrayBuffer.length] = '\0';
						}
						else
						{
							strncpy(serviceGeneralInfo->DictionariesProvided[arrayCount],
									arrayBuffer.data,
									MAX_SRCDIR_INFO_STRLEN - 1);
							serviceGeneralInfo->DictionariesProvided[arrayCount][MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
						}
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeArrayEntry() failed with return code: %d\n", ret);
						return ret;
					}
					arrayCount++;
				}
				arrayCount = 0;
			}
			/* DictionariesUsed */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DICTIONARYS_USED))
			{
				if ((ret = rsslDecodeArray(dIter, &rsslArray)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeArray() failed with return code: %d\n", ret);
					return ret;
				}
				while ((ret = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* break out of decoding array items when max dictionaries reached */
					if (arrayCount == maxDictionaries)
					{
						rsslFinishDecodeEntries(dIter);
						break;
					}

					if (ret == RSSL_RET_SUCCESS)
					{
						if (arrayBuffer.length < MAX_SRCDIR_INFO_STRLEN)
						{
							strncpy(serviceGeneralInfo->DictionariesUsed[arrayCount],
									arrayBuffer.data,
									arrayBuffer.length);
							serviceGeneralInfo->DictionariesUsed[arrayCount][arrayBuffer.length] = '\0';
						}
						else
						{
							strncpy(serviceGeneralInfo->DictionariesUsed[arrayCount],
									arrayBuffer.data,
									MAX_SRCDIR_INFO_STRLEN - 1);
							serviceGeneralInfo->DictionariesUsed[arrayCount][MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
						}
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeArrayEntry() failed with return code: %d\n", ret);
						return ret;
					}
					arrayCount++;
				}
				arrayCount = 0;
			}
			/* QoS */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_QOS))
			{
				foundQoS = RSSL_TRUE;
				if ((ret = rsslDecodeArray(dIter, &rsslArray)) < RSSL_RET_SUCCESS)
				{
					printf("rsslDecodeArray() failed with return code: %d\n", ret);
					return ret;
				}
				while ((ret = rsslDecodeArrayEntry(dIter, &arrayBuffer)) != RSSL_RET_END_OF_CONTAINER)
				{
					/* break out of decoding array items when max QOS reached */
					if (arrayCount == maxQOS)
					{
						rsslFinishDecodeEntries(dIter);
						break;
					}

					if (ret == RSSL_RET_SUCCESS)
					{
						ret = rsslDecodeQos(dIter, &serviceGeneralInfo->QoS[arrayCount]);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeQos() failed with return code: %d\n", ret);
							return ret;
						}
					}
					else if (ret != RSSL_RET_BLANK_DATA)
					{
						printf("rsslDecodeArrayEntry() failed with return code: %d\n", ret);
						return ret;
					}
					arrayCount++;
				}
				arrayCount = 0;
			}
			/* SupportsQosRange */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPS_QOS_RANGE))
			{
				ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->SupportsQosRange);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* ItemList */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ITEM_LIST))
			{
				if (element.encData.length < MAX_SRCDIR_INFO_STRLEN)
				{
					strncpy(serviceGeneralInfo->ItemList, element.encData.data, element.encData.length);
					serviceGeneralInfo->ItemList[element.encData.length] = '\0';
				}
				else
				{
					strncpy(serviceGeneralInfo->ItemList, element.encData.data, MAX_SRCDIR_INFO_STRLEN - 1);
					serviceGeneralInfo->ItemList[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
				}
			}
			/* SupportsOutOfBandSnapshots */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SUPPS_OOB_SNAPSHOTS))
			{
				ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->SupportsOutOfBandSnapshots);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* AcceptingConsumerStatus */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ACCEPTING_CONS_STATUS))
			{
				ret = rsslDecodeUInt(dIter, &serviceGeneralInfo->AcceptingConsumerStatus);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* if QoS was not send in the directory refresh message set it to the default values */
	if (!foundQoS)
		for (arrayCount = 0; arrayCount < maxQOS; arrayCount++)
		{
			serviceGeneralInfo->QoS[arrayCount].timeliness = RSSL_QOS_TIME_REALTIME;
			serviceGeneralInfo->QoS[arrayCount].rate = RSSL_QOS_RATE_TICK_BY_TICK;
		}
	
	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's state information into the RsslServiceStateInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceStateInfo - The service's state information structure
 * dIter - The decode iterator
 */
RsslRet decodeServiceStateInfo(RsslServiceStateInfo* serviceStateInfo,
									  RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service state information */
			/* ServiceState */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SVC_STATE))
			{
				ret = rsslDecodeUInt(dIter, &serviceStateInfo->ServiceState);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* AcceptingRequests */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ACCEPTING_REQS))
			{
				ret = rsslDecodeUInt(dIter, &serviceStateInfo->AcceptingRequests);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* Status */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_STATUS))
			{
				ret = rsslDecodeState(dIter, &serviceStateInfo->Status);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's group information into the RsslServiceGroupInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceGroupInfo - The service's group information structure
 * dIter - The decode iterator
 */
RsslRet decodeServiceGroupInfo(RsslServiceGroupInfo* serviceGroupInfo,
									  RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service group information */
			/* Group */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_GROUP))
			{
				if (element.encData.length < MAX_GROUP_INFO_LEN)
				{
					memcpy(serviceGroupInfo->Group, element.encData.data, element.encData.length);
				}
				else
				{
					memcpy(serviceGroupInfo->Group, element.encData.data, MAX_GROUP_INFO_LEN);
				}
			}
			/* MergedToGroup */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MERG_TO_GRP))
			{
				if (element.encData.length < MAX_GROUP_INFO_LEN)
				{
					memcpy(serviceGroupInfo->MergedToGroup, element.encData.data, element.encData.length);
				}
				else
				{
					memcpy(serviceGroupInfo->MergedToGroup, element.encData.data, MAX_GROUP_INFO_LEN);
				}
			}
			/* Status */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_STATUS))
			{
				ret = rsslDecodeState(dIter, &serviceGroupInfo->Status);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's load information into the RsslServiceLoadInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceLoadInfo - The service's load information structure
 * dIter - The decode iterator
 */
RsslRet decodeServiceLoadInfo(RsslServiceLoadInfo* serviceLoadInfo,
									 RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service load information */
			/* OpenLimit */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_OPEN_LIMIT))
			{
				ret = rsslDecodeUInt(dIter, &serviceLoadInfo->OpenLimit);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* OpenWindow */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_OPEN_WINDOW))
			{
				ret = rsslDecodeUInt(dIter, &serviceLoadInfo->OpenWindow);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* LoadFactor */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_LOAD_FACT))
			{
				ret = rsslDecodeUInt(dIter, &serviceLoadInfo->LoadFactor);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's data information into the RsslServiceDataInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceDataInfo - The service's data information structure
 * dIter - The decode iterator
 */
RsslRet decodeServiceDataInfo(RsslServiceDataInfo* serviceDataInfo,
									 RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get service data information */
			/* Type */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TYPE))
			{
				ret = rsslDecodeUInt(dIter, &serviceDataInfo->Type);
				if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			/* Data */
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DATA))
			{
				if (element.encData.length < MAX_DATA_INFO_LEN)
				{
					memcpy(serviceDataInfo->Data, element.encData.data, element.encData.length);
				}
				else
				{
					memcpy(serviceDataInfo->Data, element.encData.data, MAX_DATA_INFO_LEN);
				}
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Decodes the service's link information into the RsslServiceLinkInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceLinkInfo - The service's link information structure
 * dIter - The decode iterator
 * maxLinks - The maximum number of link entries that the structure holds
 */
RsslRet decodeServiceLinkInfo(RsslServiceLinkInfo* serviceLinkInfo,
									 RsslDecodeIterator* dIter,
									 RsslUInt32 maxLinks)
{
	RsslRet ret = 0;
	RsslMap map;
	RsslMapEntry mEntry;
	RsslBuffer linkNameBuf;
	RsslElementList	elementList;
	RsslElementEntry element;
	int linkCount = 0;

	/* decode map */
	if ((ret = rsslDecodeMap(dIter, &map)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMap() failed with return code: %d\n", ret);
		return ret;
	}

	/* service link key data type must be RSSL_DT_ASCII_STRING */
	if (map.keyPrimitiveType != RSSL_DT_ASCII_STRING)
	{
		printf("Map has incorrect keyPrimitiveType: %s", rsslDataTypeToString(map.keyPrimitiveType));
		return RSSL_RET_FAILURE;
	}

	/* decode map entries */
	/* link name is contained in map entry encKey */
	while ((ret = rsslDecodeMapEntry(dIter, &mEntry, &linkNameBuf)) != RSSL_RET_END_OF_CONTAINER)
	{
		/* break out of decoding when max services reached */
		if (linkCount == maxLinks)
		{
			rsslFinishDecodeEntries(dIter);
			break;
		}

		/* store link name in service link information */
		if (linkNameBuf.length < MAX_SRCDIR_INFO_STRLEN)
		{
			strncpy(serviceLinkInfo[linkCount].LinkName, linkNameBuf.data, linkNameBuf.length);
			serviceLinkInfo[linkCount].LinkName[linkNameBuf.length] = '\0';
		}
		else
		{
			strncpy(serviceLinkInfo[linkCount].LinkName, linkNameBuf.data, MAX_SRCDIR_INFO_STRLEN - 1);
			serviceLinkInfo[linkCount].LinkName[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
		}

		if (ret == RSSL_RET_SUCCESS)
		{
			if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeUInt() failed with return code: %d\n", ret);
				return ret;
			}

			/* decode element list */
			if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
			{
				printf("rsslDecodeElementList() failed with return code: %d\n", ret);
				return ret;
			}

			/* decode element list elements */
			while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
			{
				if (ret == RSSL_RET_SUCCESS)
				{
					/* get service link information */
					/* Type */
					if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TYPE))
					{
						ret = rsslDecodeUInt(dIter, &serviceLinkInfo[linkCount].Type);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", ret);
							return ret;
						}
					}
					/* LinkState */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_LINK_STATE))
					{
						ret = rsslDecodeUInt(dIter, &serviceLinkInfo[linkCount].LinkState);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", ret);
							return ret;
						}
					}
					/* LinkCode */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_LINK_CODE))
					{
						ret = rsslDecodeUInt(dIter, &serviceLinkInfo[linkCount].LinkCode);
						if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
						{
							printf("rsslDecodeUInt() failed with return code: %d\n", ret);
							return ret;
						}
					}
					/* Text */
					else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TEXT))
					{
						if (element.encData.length < MAX_SRCDIR_INFO_STRLEN)
						{
							strncpy(serviceLinkInfo[linkCount].Text, element.encData.data, element.encData.length);
							serviceLinkInfo[linkCount].Text[element.encData.length] = '\0';
						}
						else
						{
							strncpy(serviceLinkInfo[linkCount].Text, element.encData.data, MAX_SRCDIR_INFO_STRLEN - 1);
							serviceLinkInfo[linkCount].Text[MAX_SRCDIR_INFO_STRLEN - 1] = '\0';
						}
					}
				}
				else
				{
					printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}
		linkCount++;
	}

	return RSSL_RET_SUCCESS;
}


/*
 * Decodes the service's link information into the RsslServiceLinkInfo structure.
 * Returns success if decoding succeeds or failure if decoding fails.
 * serviceLinkInfo - The service's link information structure
 * dIter - The decode iterator
 * maxLinks - The maximum number of link entries that the structure holds
 */
RsslRet decodeEDF(RsslEDFInfo* edfInfo,
									 RsslDecodeIterator* dIter)
{
	RsslRet ret = 0;
	RsslElementList	elementList;
	RsslElementEntry element;
	RsslVector vector;
	RsslVectorEntry vectorEntry;
	RsslBuffer fidBufferValue;
	RsslLocalElementSetDefDb elementSetDefDb;

	/* decode element list */
	if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeElementList() failed with return code: %d\n", ret);
		return ret;
	}

	/* decode element list elements */
	while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret == RSSL_RET_SUCCESS)
		{
			/* get snapshot server host information */
			/* SnapshotServerHost */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SNAPSHOT_SERVER_HOST))
			{
				if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
				{
					sprintf(&edfInfo->SnapshotServer.Address[0], "%.*s", fidBufferValue.length, fidBufferValue.data);
					printf("SnapshotServerHost: %s\n", edfInfo->SnapshotServer.Address);
				}
				else if (ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
					return ret;
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_SNAPSHOT_SERVER_PORT))
			{
				RsslUInt64	u64;
				
				if ((ret = rsslDecodeUInt(dIter, &u64)) == RSSL_RET_SUCCESS)
				{
					sprintf(&edfInfo->SnapshotServer.Port[0], "%d", u64);
					printf("SnapshotServerPort: %s\n", edfInfo->SnapshotServer.Port);
				}
				else if (ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_GAPFILL_SERVER_HOST))
			{
				if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
				{
					sprintf(&edfInfo->GapRequestServer.Address[0], "%.*s", fidBufferValue.length, fidBufferValue.data);
					printf("GapfillServerHost: %s\n", edfInfo->GapRequestServer.Address);
				}
				else if (ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
					return ret;
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_GAPFILL_SERVER_PORT))
			{
				RsslUInt64	u64;
				
				if ((ret = rsslDecodeUInt(dIter, &u64)) == RSSL_RET_SUCCESS)
				{
					sprintf(&edfInfo->GapRequestServer.Port[0], "%d", u64);
					printf("GapfillServerPort: %s\n", edfInfo->GapRequestServer.Port);
				}
				else if (ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_REFERENCE_DATA_SERVER_HOST))
			{
				if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
				{
					sprintf(&edfInfo->RefDataServer.Address[0], "%.*s", fidBufferValue.length, fidBufferValue.data);
					printf("RefDataServerHost: %s\n", edfInfo->RefDataServer.Address);
				}
				else if (ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
					return ret;
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_REFERENCE_DATA_SERVER_PORT))
			{
				RsslUInt64	u64;
				
				if ((ret = rsslDecodeUInt(dIter, &u64)) == RSSL_RET_SUCCESS)
				{
					sprintf(&edfInfo->RefDataServer.Port[0], "%d", u64);
					printf("RefDataServerPort: %s\n", edfInfo->RefDataServer.Port);
				}
				else if (ret != RSSL_RET_BLANK_DATA)
				{
					printf("rsslDecodeUInt() failed with return code: %d\n", ret);
					return ret;
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MBO_GAP_MULTICAST_CHANNELS))
			{
				rsslClearVector(&vector);
				if ((ret = rsslDecodeVector(dIter, &vector)) < RSSL_RET_SUCCESS)
				{
					/* decoding failures tend to be unrecoverable */
					printf("Error %s (%d) encountered with rsslDecodeVector().  Error Text: %s\n",
							rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
					return ret;
				}

				/* If the vector flags indicate that set definition content is present, decode the set def db */
				if (vector.flags & RSSL_VTF_HAS_SET_DEFS)
				{
					/* must ensure it is the correct type - if map contents are element list, this is a field set definition db */
					if (vector.containerType == RSSL_DT_ELEMENT_LIST)
					{
						rsslClearLocalElementSetDefDb(&elementSetDefDb);
						if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &elementSetDefDb)) < RSSL_RET_SUCCESS)
						{
							/* decoding failures tend to be unrecoverable */
							printf("Error %s (%d) encountered with rsslDecodeLocalElementSetDefDb().  Error Text: %s\n",
									rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
							return ret;
						}
					}
				}

				rsslClearVectorEntry(&vectorEntry);
				while ((ret = rsslDecodeVectorEntry(dIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret < RSSL_RET_SUCCESS)
					{
						printf("Error %s (%d) encountered with rsslDecodeVectorEntry().  Error Text: %s\n",
								rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
						return ret;
					}

					switch (vector.containerType)
					{
						case RSSL_DT_ELEMENT_LIST:
						{
							RsslElementList vectorElementList;
							/* decode element list */
							if ((ret = rsslDecodeElementList(dIter, &vectorElementList, &elementSetDefDb)) < RSSL_RET_SUCCESS)
							{
								printf("rsslDecodeElementList() failed with return code: %d\n", ret);
								return ret;
							}
							/* decode element list elements */
							while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (ret == RSSL_RET_SUCCESS)
								{
									/* get snapshot server host information */
									/* SnapshotServerHost */
									if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MULTICAST_GROUP))
									{
										if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
										{
											sprintf(&edfInfo->MBOGapFillServer[vectorEntry.index].Address[0], "%.*s", fidBufferValue.length, fidBufferValue.data);
											printf("MBOGapFillServerAddress: %s\n", edfInfo->MBOGapFillServer[vectorEntry.index].Address);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
											return ret;
										}
									}
									else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PORT))
									{
										RsslUInt64	u64;

										if ((ret = rsslDecodeUInt(dIter, &u64)) == RSSL_RET_SUCCESS)
										{
											sprintf(&edfInfo->MBOGapFillServer[vectorEntry.index].Port[0], "%d", u64);
											printf("MBOGapFillServerPort: %s\n", edfInfo->MBOGapFillServer[vectorEntry.index].Port);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeUInt() failed with return code: %d\n", ret);
											return ret;
										}
									}
								}
							}
						}
							break;
						default:
							printf("Error: Vector contained unhandled containerType %d.\n",
									vector.containerType);
							return RSSL_RET_FAILURE;
					}
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MBO_STREAMING_MULTICAST_CHANNELS))
			{
				rsslClearVector(&vector);
				if ((ret = rsslDecodeVector(dIter, &vector)) < RSSL_RET_SUCCESS)
				{
					/* decoding failures tend to be unrecoverable */
					printf("Error %s (%d) encountered with rsslDecodeVector().  Error Text: %s\n",
							rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
					return ret;
				}

				/* If the vector flags indicate that set definition content is present, decode the set def db */
				if (vector.flags & RSSL_VTF_HAS_SET_DEFS)
				{
					/* must ensure it is the correct type - if map contents are element list, this is a field set definition db */
					if (vector.containerType == RSSL_DT_ELEMENT_LIST)
					{
						rsslClearLocalElementSetDefDb(&elementSetDefDb);
						if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &elementSetDefDb)) < RSSL_RET_SUCCESS)
						{
							/* decoding failures tend to be unrecoverable */
							printf("Error %s (%d) encountered with rsslDecodeLocalElementSetDefDb().  Error Text: %s\n",
									rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
							return ret;
						}
					}
				}

				rsslClearVectorEntry(&vectorEntry);
				while ((ret = rsslDecodeVectorEntry(dIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret < RSSL_RET_SUCCESS)
					{
						printf("Error %s (%d) encountered with rsslDecodeVectorEntry().  Error Text: %s\n",
								rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
						return ret;
					}

					switch (vector.containerType)
					{
						case RSSL_DT_ELEMENT_LIST:
						{
							RsslElementList vectorElementList;
							/* decode element list */
							if ((ret = rsslDecodeElementList(dIter, &vectorElementList, &elementSetDefDb)) < RSSL_RET_SUCCESS)
							{
								printf("rsslDecodeElementList() failed with return code: %d\n", ret);
								return ret;
							}
							/* decode element list elements */
							while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (ret == RSSL_RET_SUCCESS)
								{
									/* get snapshot server host information */
									/* SnapshotServerHost */
									if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MULTICAST_GROUP))
									{
										if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
										{
											sprintf(&edfInfo->MBORealTimeDataServer[vectorEntry.index].Address[0], "%.*s", fidBufferValue.length, fidBufferValue.data);
											printf("MBORealTimeDataServerAddress: %s\n", edfInfo->MBORealTimeDataServer[vectorEntry.index].Address);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
											return ret;
										}
									}
									else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PORT))
									{
										RsslUInt64	u64;

										if ((ret = rsslDecodeUInt(dIter, &u64)) == RSSL_RET_SUCCESS)
										{
											sprintf(&edfInfo->MBORealTimeDataServer[vectorEntry.index].Port[0], "%d", u64);
											printf("MBORealTimeDataServerPort: %s\n", edfInfo->MBORealTimeDataServer[vectorEntry.index].Port);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeUInt() failed with return code: %d\n", ret);
											return ret;
										}
									}
									else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DOMAIN))
									{
										if ((ret = rsslDecodeUInt(dIter, &edfInfo->MBORealTimeDataServer[vectorEntry.index].Domain)) == RSSL_RET_SUCCESS)
										{
											printf("MBORealTimeDataServerDomain: %d\n", edfInfo->MBORealTimeDataServer[vectorEntry.index].Domain);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeUInt() failed with return code: %d\n", ret);
											return ret;
										}
									}
								}
								else
								{
									printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
									return ret;
								}
							}
						}
						break;
						default:
							printf("Error: Vector contained unhandled containerType %d.\n",
									vector.containerType);
							return RSSL_RET_FAILURE;
					}
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MP_GAP_MULTICAST_CHANNELS))
			{
				rsslClearVector(&vector);
				if ((ret = rsslDecodeVector(dIter, &vector)) < RSSL_RET_SUCCESS)
				{
					/* decoding failures tend to be unrecoverable */
					printf("Error %s (%d) encountered with rsslDecodeVector().  Error Text: %s\n",
							rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
					return ret;
				}

				/* If the vector flags indicate that set definition content is present, decode the set def db */
				if (vector.flags & RSSL_VTF_HAS_SET_DEFS)
				{
					/* must ensure it is the correct type - if map contents are element list, this is a field set definition db */
					if (vector.containerType == RSSL_DT_ELEMENT_LIST)
					{
						rsslClearLocalElementSetDefDb(&elementSetDefDb);
						if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &elementSetDefDb)) < RSSL_RET_SUCCESS)
						{
							/* decoding failures tend to be unrecoverable */
							printf("Error %s (%d) encountered with rsslDecodeLocalElementSetDefDb().  Error Text: %s\n",
									rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
							return ret;
						}
					}
				}

				rsslClearVectorEntry(&vectorEntry);
				while ((ret = rsslDecodeVectorEntry(dIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret < RSSL_RET_SUCCESS)
					{
						printf("Error %s (%d) encountered with rsslDecodeVectorEntry().  Error Text: %s\n",
								rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
						return ret;
					}

					switch (vector.containerType)
					{
						case RSSL_DT_ELEMENT_LIST:
						{
							RsslElementList vectorElementList;
							/* decode element list */
							if ((ret = rsslDecodeElementList(dIter, &vectorElementList, &elementSetDefDb)) < RSSL_RET_SUCCESS)
							{
								printf("rsslDecodeElementList() failed with return code: %d\n", ret);
								return ret;
							}
							/* decode element list elements */
							while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (ret == RSSL_RET_SUCCESS)
								{
									/* get snapshot server host information */
									/* SnapshotServerHost */
									if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MULTICAST_GROUP))
									{
										if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
										{
											sprintf(&edfInfo->MPGapFillServer[vectorEntry.index].Address[0], "%.*s", fidBufferValue.length, fidBufferValue.data);
											printf("MPGapFillServerAddress: %s\n", edfInfo->MPGapFillServer[vectorEntry.index].Address);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
											return ret;
										}
									}
									else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PORT))
									{
										RsslUInt64	u64;

										if ((ret = rsslDecodeUInt(dIter, &u64)) == RSSL_RET_SUCCESS)
										{
											sprintf(&edfInfo->MPGapFillServer[vectorEntry.index].Port[0], "%d", u64);
											printf("MPGapFillServerPort: %s\n", edfInfo->MPGapFillServer[vectorEntry.index].Port);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeUInt() failed with return code: %d\n", ret);
											return ret;
										}
									}
								}
							}
						}
							break;
						default:
							printf("Error: Vector contained unhandled containerType %d.\n",
									vector.containerType);
							return RSSL_RET_FAILURE;
					}
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MP_STREAMING_MULTICAST_CHANNELS))
			{
				rsslClearVector(&vector);
				if ((ret = rsslDecodeVector(dIter, &vector)) < RSSL_RET_SUCCESS)
				{
					/* decoding failures tend to be unrecoverable */
					printf("Error %s (%d) encountered with rsslDecodeVector().  Error Text: %s\n",
							rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
					return ret;
				}

				/* If the vector flags indicate that set definition content is present, decode the set def db */
				if (vector.flags & RSSL_VTF_HAS_SET_DEFS)
				{
					/* must ensure it is the correct type - if map contents are element list, this is a field set definition db */
					if (vector.containerType == RSSL_DT_ELEMENT_LIST)
					{
						rsslClearLocalElementSetDefDb(&elementSetDefDb);
						if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &elementSetDefDb)) < RSSL_RET_SUCCESS)
						{
							/* decoding failures tend to be unrecoverable */
							printf("Error %s (%d) encountered with rsslDecodeLocalElementSetDefDb().  Error Text: %s\n",
									rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
							return ret;
						}
					}
				}

				rsslClearVectorEntry(&vectorEntry);
				while ((ret = rsslDecodeVectorEntry(dIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret < RSSL_RET_SUCCESS)
					{
						printf("Error %s (%d) encountered with rsslDecodeVectorEntry().  Error Text: %s\n",
								rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
						return ret;
					}

					switch (vector.containerType)
					{
						case RSSL_DT_ELEMENT_LIST:
						{
							RsslElementList vectorElementList;
							/* decode element list */
							if ((ret = rsslDecodeElementList(dIter, &vectorElementList, &elementSetDefDb)) < RSSL_RET_SUCCESS)
							{
								printf("rsslDecodeElementList() failed with return code: %d\n", ret);
								return ret;
							}
							/* decode element list elements */
							while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
							{
								if (ret == RSSL_RET_SUCCESS)
								{
									/* get snapshot server host information */
									/* SnapshotServerHost */
									if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MULTICAST_GROUP))
									{
										if ((ret = rsslDecodeBuffer(dIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
										{
											sprintf(&edfInfo->MPRealTimeDataServer[vectorEntry.index].Address[0], "%.*s", fidBufferValue.length, fidBufferValue.data);
											printf("MPRealTimeDataServerAddress: %s\n", edfInfo->MPRealTimeDataServer[vectorEntry.index].Address);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeBuffer() failed with return code: %d\n", ret);
											return ret;
										}
									}
									else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_PORT))
									{
										RsslUInt64	u64;

										if ((ret = rsslDecodeUInt(dIter, &u64)) == RSSL_RET_SUCCESS)
										{
											sprintf(&edfInfo->MPRealTimeDataServer[vectorEntry.index].Port[0], "%d", u64);
											printf("MPRealTimeDataServerPort: %s\n", edfInfo->MPRealTimeDataServer[vectorEntry.index].Port);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeUInt() failed with return code: %d\n", ret);
											return ret;
										}
									}
									else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_DOMAIN))
									{
										if ((ret = rsslDecodeUInt(dIter, &edfInfo->MPRealTimeDataServer[vectorEntry.index].Domain)) == RSSL_RET_SUCCESS)
										{
											printf("MPRealTimeDataServerDomain: %d", edfInfo->MPRealTimeDataServer[vectorEntry.index].Domain);
										}
										else if (ret != RSSL_RET_BLANK_DATA)
										{
											printf("rsslDecodeUInt() failed with return code: %d\n", ret);
											return ret;
										}
									}
								}
								else
								{
									printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
									return ret;
								}
							}
						}
						break;
						default:
							printf("Error: Vector contained unhandled containerType %d.\n",
									vector.containerType);
							return RSSL_RET_FAILURE;
					}
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TOP_5_STREAMING_MC))
			{
				printf("%.*s\n", element.name.length, element.name.data);
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TOP_5_GAP_MC))
			{
				printf("%.*s\n", element.name.length, element.name.data);
			}
			else
			{
				printf ("Error: UNHANDELED ELEMENT ENTRY: %.*s\n", element.name.length, element.name.data);
			}
		}
		else
		{
			printf("rsslDecodeElementEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	return RSSL_RET_SUCCESS;
}
