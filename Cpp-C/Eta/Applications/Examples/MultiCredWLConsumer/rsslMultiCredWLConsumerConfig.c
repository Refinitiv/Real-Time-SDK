/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020-2025 LSEG. All rights reserved.
*/

/*
 * This file handles configuration of the rsslMultiCredWLConsumer application.
 */

#include "rsslMultiCredWLConsumerConfig.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include "cjson/cJSON.h"

#ifdef WIN32
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

WatchlistConsumerConfig watchlistConsumerConfig;

/* Defaults */
const char *defaultHostName		= "localhost";
const char *defaultPort 		= "14002";
RsslBuffer defaultServiceName	= { 11, (char*)"DIRECT_FEED" };
RsslBool xmlTrace = RSSL_FALSE;

WarmStandbyGroupConfig* warmStandbyGroupList = NULL;
int warmStandbyGroupCount = 0;

OAuthRequestCredential* oAuthCredentialList = NULL;
int oAuthCredentialCount = 0;

LoginRequestCredential* loginList = NULL;
int loginCount = 0;

ConnectionInfoConfig* connectionList = NULL;
int connectionCount = 0;

RsslBool credentialFound = RSSL_FALSE;
RsslBool loginFound = RSSL_FALSE;

PreferredHostConfig preferredHostConfig;


void clearLoginRequestCredential(LoginRequestCredential* credentials)
{
	memset((void*)credentials, 0, sizeof(LoginRequestCredential));
	rsslClearReactorLoginRequestMsgCredential(&credentials->requestMsgCredential);
}

void clearOAuthRequestCredential(OAuthRequestCredential* credentials)
{
	memset((void*)credentials, 0, sizeof(OAuthRequestCredential));
	rsslClearReactorOAuthCredential(&credentials->oAuthCredential);
}


void clearConnectionInfoConfig(ConnectionInfoConfig* config)
{
	memset((void*)config, 0, sizeof(ConnectionInfoConfig));
	rsslClearReactorConnectInfo(&(config->connectionInfo));
}

void clearWarmStandbyGroupConfig(WarmStandbyGroupConfig* config)
{
	memset((void*)config, 0, sizeof(WarmStandbyGroupConfig));
	rsslClearReactorWarmStandbyGroup(&config->warmStandbyGroup);
	clearConnectionInfoConfig(&config->startingActiveConnectionInfo);
}

void clearAllocatedMemory()
{
	int i, j, k;

	if(connectionList != NULL)
		free((void*)connectionList);

	if(oAuthCredentialList != NULL)
		free((void*)oAuthCredentialList);
	
	if (loginList != NULL)
	{
		for (i = 0; i < loginCount; i++)
		{
			free((void*)loginList[i].requestMsgCredential.loginRequestMsg);
		}
		free((void*)loginList);
	}

	if (warmStandbyGroupList != NULL)
	{
		for (i = 0; i < warmStandbyGroupCount; i++)
		{
			if (warmStandbyGroupList[i].startingActiveConnectionInfo.serviceList != NULL)
			{
				for (j = 0; j < warmStandbyGroupList[i].startingActiveConnectionInfo.serviceCount; j++)
				{
					free(warmStandbyGroupList[i].startingActiveConnectionInfo.serviceList[j].data);
				}

				free(warmStandbyGroupList[i].startingActiveConnectionInfo.serviceList);
			}

			if (warmStandbyGroupList[i].standbyConnectionInfoList != NULL)
			{
				for (j = 0; j < (RsslInt32)warmStandbyGroupList[i].warmStandbyGroup.standbyServerCount; j++)
				{
					if (warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceList != NULL)
					{
						for (k = 0; k < warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceCount; k++)
						{
							free(warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceList[k].data);
						}
						free(warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceList);
					}
				}
				free((void*)warmStandbyGroupList[i].standbyConnectionInfoList);
				free((void*)warmStandbyGroupList[i].warmStandbyGroup.standbyServerList);
			}
			
		}

		free((void*)warmStandbyGroupList);
	}

	if (watchlistConsumerConfig.connectionOpts.reactorConnectionList != NULL)
		free((void*)watchlistConsumerConfig.connectionOpts.reactorConnectionList);

	if (watchlistConsumerConfig.connectionOpts.reactorWarmStandbyGroupList != NULL)
		free((void*)watchlistConsumerConfig.connectionOpts.reactorWarmStandbyGroupList);

	if (watchlistConsumerConfig.consumerRole.pLoginRequestList != NULL)
		free((void*)watchlistConsumerConfig.consumerRole.pLoginRequestList);

	if (watchlistConsumerConfig.consumerRole.pOAuthCredentialList != NULL)
		free((void*)watchlistConsumerConfig.consumerRole.pOAuthCredentialList);


}


/* Adds an item to the consumer's configured list of items to request.
 * Assumes itemName is a permanent string(usually should be taken directly from argv). */
static void addItem(char *itemName, RsslUInt8 domainType, RsslBool symbolListData)
{
	ItemInfo *pItem;

	if (watchlistConsumerConfig.itemCount == MAX_ITEMS)
	{
		printf("Config Error: Example only supports up to %d items.\n", MAX_ITEMS);
		exit(-1);
	}

	pItem = &watchlistConsumerConfig.itemList[watchlistConsumerConfig.itemCount];

	/* Copy item name. */
	pItem->name.length = (RsslUInt32)strlen(itemName);
	pItem->name.data = pItem->_nameMem;
	snprintf(pItem->name.data, MAX_ITEM_NAME_LEN, "%s", itemName);

	pItem->domainType = domainType;
	pItem->streamId = ITEMS_MIN_STREAM_ID + watchlistConsumerConfig.itemCount;

	++watchlistConsumerConfig.itemCount;
}

void printUsageAndExit(int argc, char **argv)
{
	printf("Usage: %s"
			" -mp          For each occurance, requests item using Market Price domain.\n"
			" -mbp          For each occurance, requests item using Market By Price domain.\n"
			" -mbo          For each occurance, requests item using Market By Order domain.\n"
			" -x           Enables tracing of messages sent to and received from the channel.\n"
			" -runTime     Adjusts the running time of the application.\n"
			" -reconnectAttemptLimit Specifies the number of reconnections per channel, -1 is the default\n"
		    " -configJson Specifies the json config for this application.  Please see the readme for information about the JSON format.\n"
			"\n"
			"-restEnableLog enable REST logging message"
			"\n"
			"-restLogFileName set REST logging output stream"
			"\n"
			"-restEnableLogCallback enable receiving REST logging messages via callback"
			"-tokenURLV1 URL of token service V1\n"
			"-tokenURLV2 URL of token service V2\n"
			"-serviceDiscoveryURL URL the service discovery\n"
			"-restProxyHost <proxy host> Proxy host name. Used for Rest requests only: service discovery, auth\n"
			"-restProxyPort <proxy port> Proxy port. Used for Rest requests only: service discovery, auth\n"
			"-restProxyUserName <proxy username> Proxy user name. Used for Rest requests only: service discovery, auth\n"
			"-restProxyPasswd <proxy password> Proxy password. Used for Rest requests only: service discovery, auth\n"
			"-restProxyDomain <proxy domain> Proxy domain of the user. Used for Rest requests only: service discovery, auth"
			"\n"
			"-libcurlName specifies the name of the libcurl library\n"
			"-libsslName specifies the name of libssl\n"
			"-libcryptoName specifies the name of libcrypto\n"
			"-castore specifies the castore location for encrypted connections"
			"\n"
			"\n The options for Preferred host"
			"\n -fallBackInterval <value in seconds> specifies time interval in the application before call Ad Hoc Fallback Function call is invoked"
			"\n -ioctlInterval <value in seconds> specifies time interval in the application before call IOCTL is invoked"
			"\n"
			"\n Additional set of options for Preferred host. These options will be set by the IOCTL call"
			"\n -ioctlEnablePH <true/false> enable Preferred host feature"
			"\n -ioctlConnectListIndex <integer value> specifies the preferred host as an index in the connection list, starting at 0"
			"\n -ioctlDetectionTimeInterval <value in seconds> specifies time interval to switch over to a preferred host or WSB group. 0 indicates that the detection time interval is disabled"
			"\n -ioctlDetectionTimeSchedule <Cron time> specifies Cron time format to switch over to a preferred host or WSB group. detectionTimeInterval is used instead if this member is set to empty"
			"\n -ioctlWarmstandbyGroupListIndex <integer value> specifies an index to set as a preferred WarmStandby group list"
			"\n -ioctlFallBackWithinWSBGroup <true/false> specifies whether to fallback within a WSB group instead of moving into a preferred WSB group"
			"\n"
			, argv[0]);
	exit(-1);
}

/* Prerequsites for this function: Both node and connectInfo are NOT null */
RsslRet parseJsonChannelInfo(cJSON* node, ConnectionInfoConfig* connectInfo)
{
	cJSON* value;
	int i = 0;

	connectInfo->connectionInfo.rsslConnectOptions.tcp_nodelay = RSSL_TRUE;
	connectInfo->connectionInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectInfo->connectionInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	value = cJSON_GetObjectItem(node, "Name");

	if (value != NULL)
	{
		memcpy(connectInfo->_nameBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionName.data = connectInfo->_nameBuffer;
		connectInfo->connectionName.length = (RsslUInt32)strlen(value->valuestring);
	}

	value = cJSON_GetObjectItem(node, "Host");

	if (value != NULL)
	{
		memcpy(connectInfo->_hostBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.connectionInfo.unified.address = connectInfo->_hostBuffer;
	}

	value = cJSON_GetObjectItem(node, "Port");

	if (value != NULL)
	{
		memcpy(connectInfo->_portBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.connectionInfo.unified.serviceName = connectInfo->_portBuffer;
	}

	value = cJSON_GetObjectItem(node, "Interface");

	if (value != NULL)
	{
		memcpy(connectInfo->_interfaceBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.connectionInfo.unified.interfaceName = connectInfo->_interfaceBuffer;
	}

	value = cJSON_GetObjectItem(node, "connType");

	if (value != NULL)
	{
		connectInfo->connectionInfo.rsslConnectOptions.connectionType = (RsslUInt)cJSON_GetNumberValue(value);;
	}

	value = cJSON_GetObjectItem(node, "encryptedConnType");

	if (value != NULL)
	{
		connectInfo->connectionInfo.rsslConnectOptions.encryptionOpts.encryptedProtocol = (RsslUInt)cJSON_GetNumberValue(value);;
	}

	value = cJSON_GetObjectItem(node, "encryptionProtocolFlags");

	if (value != NULL)
	{
		connectInfo->connectionInfo.rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = (RsslUInt32)cJSON_GetNumberValue(value);
	}

	value = cJSON_GetObjectItem(node, "SessionMgnt");

	if (value != NULL)
	{
		if (cJSON_IsBool(value) == 0)
		{
			printf("SessionMgnt value is not a boolean.");
			return RSSL_RET_FAILURE;
		}

		if(value->type == cJSON_True)
			connectInfo->connectionInfo.enableSessionManagement = RSSL_TRUE;
		else
			connectInfo->connectionInfo.enableSessionManagement = RSSL_FALSE;
	}

	value = cJSON_GetObjectItem(node, "ProxyHost");

	if (value != NULL)
	{
		memcpy(connectInfo->_proxyHostBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.proxyOpts.proxyHostName = connectInfo->_proxyHostBuffer;
	}

	value = cJSON_GetObjectItem(node, "ProxyPort");

	if (value != NULL)
	{
		memcpy(connectInfo->_proxyPortBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.proxyOpts.proxyPort = connectInfo->_proxyPortBuffer;
	}

	value = cJSON_GetObjectItem(node, "ProxyUserName");

	if (value != NULL)
	{
		memcpy(connectInfo->_proxyUsernameBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.proxyOpts.proxyUserName = connectInfo->_proxyUsernameBuffer;
	}

	value = cJSON_GetObjectItem(node, "ProxyPassword");

	if (value != NULL)
	{
		memcpy(connectInfo->_proxyPasswordBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.proxyOpts.proxyPasswd = connectInfo->_proxyPasswordBuffer;
	}

	value = cJSON_GetObjectItem(node, "ProxyDomain");

	if (value != NULL)
	{
		memcpy(connectInfo->_proxyDomainBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.proxyOpts.proxyDomain = connectInfo->_proxyDomainBuffer;
	}

	value = cJSON_GetObjectItem(node, "CAStore");

	if (value != NULL)
	{
		memcpy(connectInfo->_caStoreBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.rsslConnectOptions.proxyOpts.proxyDomain = connectInfo->_caStoreBuffer;
	}

	value = cJSON_GetObjectItem(node, "Location");

	if (value != NULL)
	{
		memcpy(connectInfo->_locationBuffer, value->valuestring, strlen(value->valuestring));
		connectInfo->connectionInfo.location.data = connectInfo->_locationBuffer;
		connectInfo->connectionInfo.location.length = (RsslUInt32)strlen(value->valuestring);
	}

	value = cJSON_GetObjectItem(node, "oAuthCredentialName");

	if (value != NULL)
	{
		for (i = 0; i < oAuthCredentialCount; ++i)
		{
			if (strncmp((const char*)oAuthCredentialList[i].credentialName.data, (const char*)value->valuestring, (size_t)oAuthCredentialList[i].credentialName.length) == 0)
			{
				credentialFound = RSSL_TRUE;
				connectInfo->connectionInfo.oAuthCredentialIndex = i;
				break;
			}
		}
	}

	value = cJSON_GetObjectItem(node, "LoginMsgName");

	if (value != NULL)
	{
		for (i = 0; i < loginCount; ++i)
		{
			if (strncmp((const char*)loginList[i].loginName.data, (const char*)value->valuestring, (size_t)loginList[i].loginName.length) == 0)
			{
				loginFound = RSSL_TRUE;
				connectInfo->connectionInfo.loginReqIndex = i;
				break;
			}
		}
	}

	if (credentialFound == RSSL_FALSE && loginFound == RSSL_FALSE)
	{
		printf("Unable to match login or oauth credentials for connection %s.\n", connectInfo->connectionName.data);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/* Prerequsites for this function: Both node and pPreferredHostConfig are NOT null */
RsslRet parsePreferredHostOptions(cJSON* node, PreferredHostConfig* pPreferredHostConfig)
{
	cJSON* value;
	RsslPreferredHostOptions* pPreferredHostOptions = &pPreferredHostConfig->preferredHostOptions;

	value = cJSON_GetObjectItem(node, "enablePH"); // "EnablePreferredHostOptions");

	if (value != NULL)
	{
		if (cJSON_IsBool(value) == 0)
		{
			printf("EnablePreferredHostOptions value is not a boolean.");
			return RSSL_RET_FAILURE;
		}

		if (value->type == cJSON_True)
			pPreferredHostOptions->enablePreferredHostOptions = RSSL_TRUE;
		else
			pPreferredHostOptions->enablePreferredHostOptions = RSSL_FALSE;
	}

	value = cJSON_GetObjectItem(node, "detectionTimeSchedule");

	if (value != NULL)
	{
		RsslUInt32 len = (RsslUInt32)(strlen(value->valuestring) < sizeof(pPreferredHostConfig->_detectionTimeSchedule) ?
			strlen(value->valuestring) : sizeof(pPreferredHostConfig->_detectionTimeSchedule));
		memcpy(pPreferredHostConfig->_detectionTimeSchedule, value->valuestring, len);
		pPreferredHostOptions->detectionTimeSchedule.data = pPreferredHostConfig->_detectionTimeSchedule;
		pPreferredHostOptions->detectionTimeSchedule.length = (RsslUInt32)strlen(value->valuestring);
	}

	value = cJSON_GetObjectItem(node, "detectionTimeInterval");

	if (value != NULL)
	{
		pPreferredHostOptions->detectionTimeInterval = (RsslUInt32)cJSON_GetNumberValue(value);
	}

	value = cJSON_GetObjectItem(node, "connectListIndex");

	if (value != NULL)
	{
		pPreferredHostOptions->connectionListIndex = (RsslUInt32)cJSON_GetNumberValue(value);
	}

	value = cJSON_GetObjectItem(node, "warmstandbyGroupListIndex");

	if (value != NULL)
	{
		pPreferredHostOptions->warmStandbyGroupListIndex = (RsslUInt32)cJSON_GetNumberValue(value);
	}

	value = cJSON_GetObjectItem(node, "fallbackWithinWSBGroup");

	if (value != NULL)
	{
		if (cJSON_IsBool(value) == 0)
		{
			printf("FallBackWithInWSBGroup value is not a boolean.");
			return RSSL_RET_FAILURE;
		}

		if (value->type == cJSON_True)
			pPreferredHostOptions->fallBackWithInWSBGroup = RSSL_TRUE;
		else
			pPreferredHostOptions->fallBackWithInWSBGroup = RSSL_FALSE;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet parseJsonInput(char* fileName)
{
	int statResult;
	cJSON* root;
	cJSON* node;
	cJSON* subNode;
	cJSON* item;
	cJSON* standbyNode;
	cJSON* itemValue;
	cJSON* serviceNode;
	char* inputBuffer = NULL;
	size_t bytesRead;
	FILE* fp = NULL;
	FILE* jwkFile = NULL;
	int readSize;
	int i = 0;
	int j = 0;
	int k = 0;
#ifdef WIN32
	struct _stat statBuffer;
	statResult = _stat(fileName, &statBuffer);
#else
	struct stat statBuffer;
	statResult = stat(fileName, &statBuffer);
#endif
	if (statResult == -1 || !statBuffer.st_size)
	{
		printf("Unable to find config file %s\n", fileName);
		exit(-1);
	}

	fp = fopen(fileName, "r");
	if (!fp)
	{
		printf("Unable to open config file %s\n", fileName);
		exit(-1);
	}

	inputBuffer = malloc(statBuffer.st_size + 1);

	memset((void*)inputBuffer, 0, statBuffer.st_size + 1);

	bytesRead = fread((void*)(inputBuffer), sizeof(char), statBuffer.st_size, fp);
	if (!bytesRead)
	{
		printf("Unable to read config file %s\n", fileName);
		exit(-1);
	}
	fclose(fp);

	root  = cJSON_ParseWithLength(inputBuffer, bytesRead);

	if (root == NULL)
	{
		printf("Bad JSON Parse.\n");
		exit(-1);
	}

	/* First parse the login information */

	node = cJSON_GetObjectItem(root, "LoginMsgs");
	
	if (node != NULL)
	{
		loginList = (LoginRequestCredential*)malloc((size_t)(sizeof(LoginRequestCredential)* cJSON_GetArraySize(node)));
		if (loginList == NULL)
		{
			printf("Unable to allocated login request structures.\n");
			cJSON_Delete(root);

			return RSSL_RET_FAILURE;
		}
		loginCount = cJSON_GetArraySize(node);
		for (i = 0; i < cJSON_GetArraySize(node); ++i)
		{
			clearLoginRequestCredential(&loginList[i]);
			
			/* Allocate and initialize the login request message*/
			loginList[i].requestMsgCredential.loginRequestMsg = (RsslRDMLoginRequest*)malloc(sizeof(RsslRDMLoginRequest));

			if (loginList[i].requestMsgCredential.loginRequestMsg == NULL)
			{
				printf("Failed to allocated login request message.\n");
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}

			rsslInitDefaultRDMLoginRequest(loginList[i].requestMsgCredential.loginRequestMsg, LOGIN_STREAM_ID);
			loginList[i].loginArrayIndex = i;

			
			item = cJSON_GetArrayItem(node, i);

			if (item == NULL)
			{
				printf("Failed to get Login message array.\n");
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}

			itemValue = cJSON_GetObjectItem(item, "UserName");

			if(itemValue != NULL)
			{
				memcpy(loginList[i]._userNameBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				loginList[i].requestMsgCredential.loginRequestMsg->userName.data = loginList[i]._userNameBuffer;
				loginList[i].requestMsgCredential.loginRequestMsg->userName.length = (RsslUInt32)strlen(itemValue->valuestring);
			}

			itemValue = cJSON_GetObjectItem(item, "AuthToken");

			if (itemValue != NULL)
			{
				memcpy(loginList[i]._userNameBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				loginList[i].requestMsgCredential.loginRequestMsg->userName.data = loginList[i]._userNameBuffer;
				loginList[i].requestMsgCredential.loginRequestMsg->userName.length = (RsslUInt32)strlen(itemValue->valuestring);
				loginList[i].requestMsgCredential.loginRequestMsg->flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
				loginList[i].requestMsgCredential.loginRequestMsg->userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;

				itemValue = cJSON_GetObjectItem(item, "AuthTokenExtended");

				if (itemValue != NULL)
				{
					memcpy(loginList[i]._authTokenExtendedBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
					loginList[i].requestMsgCredential.loginRequestMsg->userName.data = loginList[i]._authTokenExtendedBuffer;
					loginList[i].requestMsgCredential.loginRequestMsg->userName.length = (RsslUInt32)strlen(itemValue->valuestring);
					loginList[i].requestMsgCredential.loginRequestMsg->flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
				}
			}

			itemValue = cJSON_GetObjectItem(item, "Name");
			if (itemValue != NULL)
			{
				memcpy(loginList[i]._nameBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				loginList[i].loginName.data = loginList[i]._nameBuffer;
				loginList[i].loginName.length = (RsslUInt32)strlen(itemValue->valuestring);
			}
			else
			{
				printf("All login array members need to have a Name\n");
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}

			loginList[i].requestMsgCredential.pLoginRenewalEventCallback = loginMsgEventCallback;
			loginList[i].requestMsgCredential.userSpecPtr = &loginList[i];
		}
	}

	node = cJSON_GetObjectItem(root, "oAuthCredentials");

	if (node != NULL)
	{
		oAuthCredentialList = (OAuthRequestCredential*)malloc((size_t)(sizeof(OAuthRequestCredential) * cJSON_GetArraySize(node)));
		if (oAuthCredentialList == NULL)
		{
			printf("Unable to allocated login request structures.\n");
			cJSON_Delete(root);

			return RSSL_RET_FAILURE;
		}
		oAuthCredentialCount = cJSON_GetArraySize(node);

		for (i = 0; i < cJSON_GetArraySize(node); ++i)
		{
			clearOAuthRequestCredential(&oAuthCredentialList[i]);

			item = cJSON_GetArrayItem(node, i);

			if (item == NULL)
			{
				printf("Unable to find oauth credential array index\n");
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}

			itemValue = cJSON_GetObjectItem(item, "Name");
			if (itemValue != NULL)
			{
				memcpy(oAuthCredentialList[i]._nameBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				oAuthCredentialList[i].credentialName.data = oAuthCredentialList[i]._nameBuffer;
				oAuthCredentialList[i].credentialName.length = (RsslUInt32)strlen(itemValue->valuestring);
			}
			else
			{
				printf("All oAuth array members need to have a Name\n");
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}

			itemValue = cJSON_GetObjectItem(item, "UserName");
			if (itemValue != NULL)
			{
				memcpy(oAuthCredentialList[i]._usernameBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				oAuthCredentialList[i].oAuthCredential.userName.data = oAuthCredentialList[i]._usernameBuffer;
				oAuthCredentialList[i].oAuthCredential.userName.length = (RsslUInt32)strlen(itemValue->valuestring);
			}

			itemValue = cJSON_GetObjectItem(item, "Password");
			if (itemValue != NULL)
			{
				memcpy(oAuthCredentialList[i]._passwordBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				oAuthCredentialList[i].oAuthCredential.password.data = oAuthCredentialList[i]._passwordBuffer;
				oAuthCredentialList[i].oAuthCredential.password.length = (RsslUInt32)strlen(itemValue->valuestring);
			}

			itemValue = cJSON_GetObjectItem(item, "clientId");
			if (itemValue != NULL)
			{
				memcpy(oAuthCredentialList[i]._clientIdBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				oAuthCredentialList[i].oAuthCredential.clientId.data = oAuthCredentialList[i]._clientIdBuffer;
				oAuthCredentialList[i].oAuthCredential.clientId.length = (RsslUInt32)strlen(itemValue->valuestring);
			}

			itemValue = cJSON_GetObjectItem(item, "clientSecret");
			if (itemValue != NULL)
			{
				memcpy(oAuthCredentialList[i]._clientSecretBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				oAuthCredentialList[i].oAuthCredential.clientSecret.data = oAuthCredentialList[i]._clientSecretBuffer;
				oAuthCredentialList[i].oAuthCredential.clientSecret.length = (RsslUInt32)strlen(itemValue->valuestring);
			}

			itemValue = cJSON_GetObjectItem(item, "jwkFile");
			if (itemValue != NULL)
			{
				jwkFile = NULL;
				readSize = 0;
				/* As this is an example program showing API, this handling of the JWK is not secure. */
				jwkFile = fopen(itemValue->valuestring, "rb");
				if (jwkFile == NULL)
				{
					printf("Cannot open jwk file: %s\n", itemValue->valuestring);
					cJSON_Delete(root);

					return RSSL_RET_FAILURE;
				}
				/* Read the JWK contents into a pre-allocated buffer*/
				readSize = (int)fread(oAuthCredentialList[i]._clientJWKBuffer, sizeof(char), 2047, jwkFile);
				if (readSize == 0)
				{
					printf("Cannot read jwk file: %s\n", itemValue->valuestring);
					cJSON_Delete(root);

					return RSSL_RET_FAILURE;
				}

				fclose(jwkFile);

				oAuthCredentialList[i].oAuthCredential.clientJWK.data = oAuthCredentialList[i]._clientJWKBuffer;
				oAuthCredentialList[i].oAuthCredential.clientJWK.length = readSize;
			}

			itemValue = cJSON_GetObjectItem(item, "audience");
			if (itemValue != NULL)
			{
				memcpy(oAuthCredentialList[i]._audienceBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				oAuthCredentialList[i].oAuthCredential.audience.data = oAuthCredentialList[i]._audienceBuffer;
				oAuthCredentialList[i].oAuthCredential.audience.length = (RsslUInt32)strlen(itemValue->valuestring);
			}

			itemValue = cJSON_GetObjectItem(item, "tokenScope");
			if (itemValue != NULL)
			{
				memcpy(oAuthCredentialList[i]._tokenScopeBuffer, itemValue->valuestring, strlen(itemValue->valuestring));
				oAuthCredentialList[i].oAuthCredential.tokenScope.data = oAuthCredentialList[i]._tokenScopeBuffer;
				oAuthCredentialList[i].oAuthCredential.tokenScope.length = (RsslUInt32)strlen(itemValue->valuestring);
			}

			itemValue = cJSON_GetObjectItem(item, "takeExclusiveSignOn");
			if (itemValue != NULL)
			{
				if (cJSON_IsBool(itemValue) == 0)
				{
					printf("takeExclusiveSignOn value is not a boolean.");
					cJSON_Delete(root);

					return RSSL_RET_FAILURE;
				}

				if (itemValue->type == cJSON_True)
					oAuthCredentialList[i].oAuthCredential.takeExclusiveSignOnControl = RSSL_TRUE;
				else
					oAuthCredentialList[i].oAuthCredential.takeExclusiveSignOnControl = RSSL_FALSE;
			}

			oAuthCredentialList[i].oAuthArrayIndex = i;
			oAuthCredentialList[i].oAuthCredential.pOAuthCredentialEventCallback = oAuthCredentialEventCallback;
			oAuthCredentialList[i].oAuthCredential.userSpecPtr = &oAuthCredentialList[i];

		}
	}

	node = cJSON_GetObjectItem(root, "WSBGroups");

	if (node != NULL)
	{
		warmStandbyGroupList = (WarmStandbyGroupConfig*)malloc(sizeof(WarmStandbyGroupConfig) * cJSON_GetArraySize(node));
		warmStandbyGroupCount = cJSON_GetArraySize(node);

		if (warmStandbyGroupList == NULL)
		{
			printf("Unable to allocate warmStandby group list.");
			cJSON_Delete(root);

			return RSSL_RET_FAILURE;
		}
		
		for (i = 0; i < warmStandbyGroupCount; ++i)
		{
			clearWarmStandbyGroupConfig(&warmStandbyGroupList[i]);

			subNode = cJSON_GetArrayItem(node, i);
			if (subNode == NULL)
			{
				printf("No array for WSB group\n");
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}


			item = cJSON_GetObjectItem(subNode, "WSBMode");
			if (item != NULL)
			{
				if (0 == strcmp(item->valuestring, "login"))
					warmStandbyGroupList[i].warmStandbyGroup.warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;
				else if (0 == strcmp(item->valuestring, "service"))
					warmStandbyGroupList[i].warmStandbyGroup.warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;
				else
				{
					printf("Invalid WSB Mode specified\n");
					cJSON_Delete(root);

					return RSSL_RET_FAILURE;
				}
			}

			item = cJSON_GetObjectItem(subNode, "WSBActive");

			if (item == NULL)
			{
				printf("No active config for WSB group\n");
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}

			if (parseJsonChannelInfo(item, &warmStandbyGroupList[i].startingActiveConnectionInfo) != RSSL_RET_SUCCESS)
			{
				return RSSL_RET_FAILURE;
			}

			itemValue = cJSON_GetObjectItem(item, "ServiceList");

			if (itemValue != NULL)
			{
				warmStandbyGroupList[i].startingActiveConnectionInfo.serviceList = (RsslBuffer*)malloc(sizeof(RsslBuffer) * cJSON_GetArraySize(itemValue));
				warmStandbyGroupList[i].startingActiveConnectionInfo.serviceCount = cJSON_GetArraySize(itemValue);

				for (k = 0; k < cJSON_GetArraySize(itemValue); k++)
				{
					serviceNode = cJSON_GetArrayItem(itemValue, k);

					if (serviceNode != NULL)
					{
						warmStandbyGroupList[i].startingActiveConnectionInfo.serviceList[k].data = (char*)malloc(sizeof(char) * strlen(serviceNode->valuestring));
						memcpy(warmStandbyGroupList[i].startingActiveConnectionInfo.serviceList[k].data, serviceNode->valuestring, strlen(serviceNode->valuestring));
						warmStandbyGroupList[i].startingActiveConnectionInfo.serviceList[k].length = (RsslUInt32)strlen(serviceNode->valuestring);
					}
				}
			}

			/* Set the active here */
			warmStandbyGroupList[i].warmStandbyGroup.startingActiveServer.reactorConnectInfo = warmStandbyGroupList[i].startingActiveConnectionInfo.connectionInfo;
			warmStandbyGroupList[i].warmStandbyGroup.startingActiveServer.perServiceBasedOptions.serviceNameList = warmStandbyGroupList[i].startingActiveConnectionInfo.serviceList;
			warmStandbyGroupList[i].warmStandbyGroup.startingActiveServer.perServiceBasedOptions.serviceNameCount = warmStandbyGroupList[i].startingActiveConnectionInfo.serviceCount;


			item = cJSON_GetObjectItem(subNode, "WSBStandby");

			if (item != NULL)
			{
				warmStandbyGroupList[i].standbyConnectionInfoList = (ConnectionInfoConfig*)malloc(sizeof(ConnectionInfoConfig) * cJSON_GetArraySize(item));
				warmStandbyGroupList[i].warmStandbyGroup.standbyServerCount = cJSON_GetArraySize(item);

				if (warmStandbyGroupList[i].standbyConnectionInfoList == NULL)
				{
					printf("Unable to allocated memory for standby connection info list\n");
					cJSON_Delete(root);

					return RSSL_RET_FAILURE;
				}

				warmStandbyGroupList[i].warmStandbyGroup.standbyServerList = (RsslReactorWarmStandbyServerInfo*)malloc(sizeof(RsslReactorWarmStandbyServerInfo) * cJSON_GetArraySize(item));

				if (warmStandbyGroupList[i].warmStandbyGroup.standbyServerList == NULL)
				{
					printf("Unable to allocated memory for standby server info list.\n");
					cJSON_Delete(root);

					return RSSL_RET_FAILURE;
				}

				for (j = 0; j < cJSON_GetArraySize(item); j++)
				{
					clearConnectionInfoConfig(&warmStandbyGroupList[i].standbyConnectionInfoList[j]);
					rsslClearReactorWarmStandbyServerInfo(&warmStandbyGroupList[i].warmStandbyGroup.standbyServerList[j]);
					standbyNode = cJSON_GetArrayItem(item, j);

					if (standbyNode == NULL)
					{
						printf("Unable to get standby connection.\n");
						cJSON_Delete(root);

						return RSSL_RET_FAILURE;
					}

					if (parseJsonChannelInfo(standbyNode, &warmStandbyGroupList[i].standbyConnectionInfoList[j]) != RSSL_RET_SUCCESS)
					{
						cJSON_Delete(root);

						return RSSL_RET_FAILURE;
					}

					itemValue = cJSON_GetObjectItem(standbyNode, "ServiceList");

					if (itemValue != NULL)
					{
						warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceList = (RsslBuffer*)malloc(sizeof(RsslBuffer) * cJSON_GetArraySize(itemValue));
						warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceCount = cJSON_GetArraySize(itemValue);

						for (k = 0; k < cJSON_GetArraySize(itemValue); k++)
						{
							serviceNode = cJSON_GetArrayItem(itemValue, k);

							if (serviceNode != NULL)
							{
								warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceList[k].data = (char*)malloc(sizeof(char) * strlen(serviceNode->valuestring));
								memcpy(warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceList[k].data, serviceNode->valuestring, strlen(serviceNode->valuestring));
								warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceList[k].length = (RsslUInt32)strlen(serviceNode->valuestring);
							}
						}
					}

					warmStandbyGroupList[i].warmStandbyGroup.standbyServerList[j].reactorConnectInfo = warmStandbyGroupList[i].standbyConnectionInfoList[j].connectionInfo;
					warmStandbyGroupList[i].warmStandbyGroup.standbyServerList[j].perServiceBasedOptions.serviceNameList = warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceList;
					warmStandbyGroupList[i].warmStandbyGroup.standbyServerList[j].perServiceBasedOptions.serviceNameCount = warmStandbyGroupList[i].standbyConnectionInfoList[j].serviceCount;
				}
			}
		}
	}

	node = cJSON_GetObjectItem(root, "ConnectionList");

	if (node != NULL)
	{
		connectionList = (ConnectionInfoConfig*)malloc(sizeof(ConnectionInfoConfig) * cJSON_GetArraySize(node));
		connectionCount = cJSON_GetArraySize(node);

		if (connectionList == NULL)
		{
			printf("Unable to allocate connection list.\n");
			cJSON_Delete(root);

			return RSSL_RET_FAILURE;
		}

		for (i = 0; i < cJSON_GetArraySize(node); i++)
		{
			item = cJSON_GetArrayItem(node, i);
			if (item == NULL)
			{
				printf("Unable to get connection list array item\n");
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}
			clearConnectionInfoConfig(&connectionList[i]);
			if (parseJsonChannelInfo(item, &connectionList[i]) != RSSL_RET_SUCCESS)
			{
				cJSON_Delete(root);

				return RSSL_RET_FAILURE;
			}
		}
	}

	node = cJSON_GetObjectItem(root, "PreferredHost");

	if (node != NULL)
	{
		rsslClearRsslPreferredHostOptions(&preferredHostConfig.preferredHostOptions);
		preferredHostConfig._detectionTimeSchedule[0] = 0;

		if (parsePreferredHostOptions(node, &preferredHostConfig) != RSSL_RET_SUCCESS)
		{
			cJSON_Delete(root);

			return RSSL_RET_FAILURE;
		}
	}

	cJSON_Delete(root);

	return RSSL_RET_SUCCESS;
}


void watchlistConsumerConfigInit(int argc, char** argv)
{
	int i;
	int configFlags = 0;
	int readSize = 0;
	int wsConfigFlags = 0;
	char warmStandbyMode[255];
	RsslErrorInfo						rsslErrorInfo;
	RsslBool setIoctlPreferredHostOptValue = RSSL_FALSE;

	RsslInitializeExOpts		initOpts = RSSL_INIT_INITIALIZE_EX_OPTS;

	memset(&watchlistConsumerConfig, 0, sizeof(WatchlistConsumerConfig));

	rsslClearReactorConnectOptions(&watchlistConsumerConfig.connectionOpts);
	rsslClearOMMConsumerRole(&watchlistConsumerConfig.consumerRole);

	/* Set defaults. */
	watchlistConsumerConfig.serviceName = defaultServiceName;
	watchlistConsumerConfig.itemCount = 0;
	watchlistConsumerConfig.runTime = 300;

	snprintf(watchlistConsumerConfig.libsslName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.libcryptoName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.libcurlName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.sslCAStore, 255, "%s", "");

	snprintf(watchlistConsumerConfig._tokenUrlV1, 255, "%s", "");
	snprintf(watchlistConsumerConfig._tokenUrlV2, 255, "%s", "");
	snprintf(watchlistConsumerConfig._serviceDiscoveryUrl, 255, "%s", "");

	snprintf(watchlistConsumerConfig.restProxyHost, 255, "%s", "");
	snprintf(watchlistConsumerConfig.restProxyPort, 255, "%s", "");
	snprintf(watchlistConsumerConfig.restProxyUserName, 255, "%s", "");
	snprintf(watchlistConsumerConfig.restProxyPasswd, 255, "%s", "");
	snprintf(watchlistConsumerConfig.restProxyDomain, 255, "%s", "");

	snprintf(watchlistConsumerConfig.configJsonFileName, 255, "WSBConfig.json");

	watchlistConsumerConfig.restEnableLog = RSSL_FALSE;
	watchlistConsumerConfig.restOutputStreamName = NULL;
	watchlistConsumerConfig.restEnableLogCallback = RSSL_FALSE;

	watchlistConsumerConfig.reconnectAttemptLimit = -1;
	watchlistConsumerConfig.reconnectMinDelay = 500;
	watchlistConsumerConfig.reconnectMaxDelay = 6000;

	snprintf(warmStandbyMode, 255, "login");

	rsslClearRsslPreferredHostOptions(&preferredHostConfig.preferredHostOptions);
	preferredHostConfig._detectionTimeSchedule[0] = 0;

	preferredHostConfig.directFallbackTimeInterval = 0U;
	preferredHostConfig.ioctlCallTimeInterval = 0U;

	/* New RsslPreferredHostOptions values that will be updated by Ioctl call */
	preferredHostConfig.directFallbackTime = 0U;
	preferredHostConfig.ioctlCallTime = 0U;

	preferredHostConfig.setIoctlEnablePH = RSSL_FALSE;
	preferredHostConfig.setIoctlConnectListIndex = RSSL_FALSE;
	preferredHostConfig.setIoctlDetectionTimeInterval = RSSL_FALSE;
	preferredHostConfig.setIoctlDetectionTimeSchedule = RSSL_FALSE;
	preferredHostConfig.setIoctlWarmstandbyGroupListIndex = RSSL_FALSE;
	preferredHostConfig.setIoctlFallBackWithinWSBGroup = RSSL_FALSE;

	rsslClearRsslPreferredHostOptions(&preferredHostConfig.rsslIoctlPreferredHostOpts);

	for (i = 1; i < argc; ++i)
	{
		if (strcmp("-libsslName", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.libsslName, 255, "%s", argv[i]);
		}
		else if (strcmp("-libcryptoName", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.libcryptoName, 255, "%s", argv[i]);
		}
		else if (strcmp("-libcurlName", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.libcurlName, 255, "%s", argv[i]);
		}
		else if (strcmp("-castore", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.sslCAStore, 255, "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-s"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.serviceName.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._serviceNameMem, 255, "%s", argv[i]);
			watchlistConsumerConfig.serviceName.data = watchlistConsumerConfig._serviceNameMem;
		}
		else if (0 == strcmp(argv[i], "-mp"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_MARKET_PRICE, RSSL_FALSE);
		}
		else if (0 == strcmp(argv[i], "-mbo"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_MARKET_BY_ORDER, RSSL_FALSE);
		}
		else if (0 == strcmp(argv[i], "-mbp"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			addItem(argv[i], RSSL_DMT_MARKET_BY_PRICE, RSSL_FALSE);
		}
		else if (0 == strcmp(argv[i], "-runTime"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.runTime = atoi(argv[i]);
		}
		else if (strcmp("-x", argv[i]) == 0)
		{
			xmlTrace = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-tokenURLV1"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.tokenURLV1.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._tokenUrlV1, 255, "%s", argv[i]);
			watchlistConsumerConfig.tokenURLV1.data = watchlistConsumerConfig._tokenUrlV1;
		}
		else if (0 == strcmp(argv[i], "-tokenURLV2"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.tokenURLV2.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._tokenUrlV2, 255, "%s", argv[i]);
			watchlistConsumerConfig.tokenURLV2.data = watchlistConsumerConfig._tokenUrlV2;
		}
		else if (0 == strcmp(argv[i], "-serviceDiscoveryURL"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.serviceDiscoveryURL.length =
				(RsslUInt32)snprintf(watchlistConsumerConfig._serviceDiscoveryUrl, 255, "%s", argv[i]);
			watchlistConsumerConfig.serviceDiscoveryURL.data = watchlistConsumerConfig._serviceDiscoveryUrl;
		}
		else if (strcmp("-restEnableLog", argv[i]) == 0)
		{
			watchlistConsumerConfig.restEnableLog = RSSL_TRUE;
		}
		else if (0 == strcmp(argv[i], "-restLogFileName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.restOutputStreamName = fopen(argv[i], "w");
			if (!watchlistConsumerConfig.restOutputStreamName)
			{
				printf("Error: Unable to open the specified file name %s\n", argv[i]);
				printUsageAndExit(argc, argv);
			}
		}
		else if (strcmp("-restEnableLogCallback", argv[i]) == 0)
		{
			watchlistConsumerConfig.restEnableLogCallback = RSSL_TRUE;
		}
		else if (strcmp("-reconnectAttemptLimit", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.reconnectAttemptLimit = atoi(argv[i]);
		}
		else if (strcmp("-configJson", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.configJsonFileName, 255, "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyHost"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyHost, sizeof(watchlistConsumerConfig.restProxyHost), "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyPort"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyPort, sizeof(watchlistConsumerConfig.restProxyPort), "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyUserName"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyUserName, sizeof(watchlistConsumerConfig.restProxyUserName), "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyPasswd"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyPasswd, sizeof(watchlistConsumerConfig.restProxyPasswd), "%s", argv[i]);
		}
		else if (0 == strcmp(argv[i], "-restProxyDomain"))
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(watchlistConsumerConfig.restProxyDomain, sizeof(watchlistConsumerConfig.restProxyDomain), "%s", argv[i]);
		}
		else if (strcmp("-fallBackInterval", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			preferredHostConfig.directFallbackTimeInterval = atoi(argv[i]);
		}
		else if (strcmp("-ioctlInterval", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			preferredHostConfig.ioctlCallTimeInterval = atoi(argv[i]);
		}
		else if (strcmp("-ioctlEnablePH", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			if (RTR_STRNICMP(argv[i], "true", 4) == 0)
			{
				preferredHostConfig.rsslIoctlPreferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
			}
			else if (RTR_STRNICMP(argv[i], "false", 5) == 0)
			{
				preferredHostConfig.rsslIoctlPreferredHostOpts.enablePreferredHostOptions = RSSL_FALSE;
			}
			setIoctlPreferredHostOptValue = RSSL_TRUE;
			preferredHostConfig.setIoctlEnablePH = RSSL_TRUE;
		}
		else if (strcmp("-ioctlConnectListIndex", argv[i]) == 0)
		{
			int tempIndex = -1;
			if (++i == argc) printUsageAndExit(argc, argv);
			tempIndex = atoi(argv[i]);
			if (tempIndex >= 0)
			{
				preferredHostConfig.rsslIoctlPreferredHostOpts.connectionListIndex = tempIndex;
			}
			setIoctlPreferredHostOptValue = RSSL_TRUE;
			preferredHostConfig.setIoctlConnectListIndex = RSSL_TRUE;
		}
		else if (strcmp("-ioctlDetectionTimeInterval", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			preferredHostConfig.rsslIoctlPreferredHostOpts.detectionTimeInterval = atoi(argv[i]);
			setIoctlPreferredHostOptValue = RSSL_TRUE;
			preferredHostConfig.setIoctlDetectionTimeInterval = RSSL_TRUE;
		}
		else if (strcmp("-ioctlDetectionTimeSchedule", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			snprintf(preferredHostConfig.ioctlDetectionTimeCron, sizeof(preferredHostConfig.ioctlDetectionTimeCron), "%s", argv[i]);
			preferredHostConfig.rsslIoctlPreferredHostOpts.detectionTimeSchedule.data = preferredHostConfig.ioctlDetectionTimeCron;
			preferredHostConfig.rsslIoctlPreferredHostOpts.detectionTimeSchedule.length = (RsslUInt32)strlen(preferredHostConfig.ioctlDetectionTimeCron);
			setIoctlPreferredHostOptValue = RSSL_TRUE;
			preferredHostConfig.setIoctlDetectionTimeSchedule = RSSL_TRUE;
		}
		else if (strcmp("-ioctlWarmstandbyGroupListIndex", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			preferredHostConfig.rsslIoctlPreferredHostOpts.warmStandbyGroupListIndex = atoi(argv[i]);
			setIoctlPreferredHostOptValue = RSSL_TRUE;
			preferredHostConfig.setIoctlWarmstandbyGroupListIndex = RSSL_TRUE;
		}
		else if (strcmp("-ioctlFallBackWithinWSBGroup", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			if (RTR_STRNICMP(argv[i], "true", 4) == 0)
			{
				preferredHostConfig.rsslIoctlPreferredHostOpts.fallBackWithInWSBGroup = RSSL_TRUE;
			}
			else if (RTR_STRNICMP(argv[i], "false", 5) == 0)
			{
				preferredHostConfig.rsslIoctlPreferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
			}
			setIoctlPreferredHostOptValue = RSSL_TRUE;
			preferredHostConfig.setIoctlFallBackWithinWSBGroup = RSSL_TRUE;
		}
		else if (strcmp("-reconnectMinDelay", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.reconnectMinDelay = atoi(argv[i]);
		}
		else if (strcmp("-reconnectMaxDelay", argv[i]) == 0)
		{
			if (++i == argc) printUsageAndExit(argc, argv);
			watchlistConsumerConfig.reconnectMaxDelay = atoi(argv[i]);
		}
		else
		{
			printf("Error: Unrecognized option: %s\n", argv[i]);
			printUsageAndExit(argc, argv);
		}
	}


	initOpts.jitOpts.libsslName = watchlistConsumerConfig.libsslName;
	initOpts.jitOpts.libcryptoName = watchlistConsumerConfig.libcryptoName;
	initOpts.jitOpts.libcurlName = watchlistConsumerConfig.libcurlName;
	initOpts.rsslLocking = RSSL_LOCK_GLOBAL_AND_CHANNEL;

	/* Initialize RSSL. The locking mode RSSL_LOCK_GLOBAL_AND_CHANNEL is required to use the RsslReactor. */
	if (rsslInitializeEx(&initOpts, &rsslErrorInfo.rsslError) != RSSL_RET_SUCCESS)
	{
		printf("rsslInitialize(): failed <%s>\n", rsslErrorInfo.rsslError.text);
		clearAllocatedMemory();
		exit(-1);
	}

	if (parseJsonInput(watchlistConsumerConfig.configJsonFileName) == RSSL_RET_FAILURE)
	{
		clearAllocatedMemory();
		printUsageAndExit(argc, argv);
	}


	watchlistConsumerConfig.connectionOpts.connectionCount = connectionCount;
	watchlistConsumerConfig.connectionOpts.reactorConnectionList = (RsslReactorConnectInfo*)malloc(sizeof(RsslReactorConnectInfo) * connectionCount);

	if (watchlistConsumerConfig.connectionOpts.reactorConnectionList == NULL)
	{
		printf("Unable to allocate reactorConnectionList\n");
		clearAllocatedMemory();
		printUsageAndExit(argc, argv);
	}

	for (i = 0; i < connectionCount; i++)
	{
		watchlistConsumerConfig.connectionOpts.reactorConnectionList[i] = connectionList[i].connectionInfo;
	}

	watchlistConsumerConfig.connectionOpts.warmStandbyGroupCount = warmStandbyGroupCount;
	watchlistConsumerConfig.connectionOpts.reactorWarmStandbyGroupList = (RsslReactorWarmStandbyGroup*)malloc(sizeof(RsslReactorWarmStandbyGroup) * warmStandbyGroupCount);

	if (watchlistConsumerConfig.connectionOpts.reactorWarmStandbyGroupList == NULL)
	{
		printf("Unable to allocate reactorWarmStandbyGroupList\n");
		clearAllocatedMemory();
		printUsageAndExit(argc, argv);
	}

	for (i = 0; i < warmStandbyGroupCount; i++)
	{
		watchlistConsumerConfig.connectionOpts.reactorWarmStandbyGroupList[i] = warmStandbyGroupList[i].warmStandbyGroup;
	}

	if (loginFound == RSSL_TRUE && loginCount != 0)
	{
		watchlistConsumerConfig.consumerRole.loginRequestMsgCredentialCount = loginCount;

		watchlistConsumerConfig.consumerRole.pLoginRequestList = (RsslReactorLoginRequestMsgCredential**)malloc(sizeof(RsslReactorLoginRequestMsgCredential*) * loginCount);

		if (watchlistConsumerConfig.consumerRole.pLoginRequestList == NULL)
		{
			printf("Unable to allocate login request list on consumerRole\n");
			clearAllocatedMemory();
			printUsageAndExit(argc, argv);
		}

		for (i = 0; i < loginCount; i++)
		{
			watchlistConsumerConfig.consumerRole.pLoginRequestList[i] = &loginList[i].requestMsgCredential;
		}
	}

	if (credentialFound == RSSL_TRUE && oAuthCredentialCount != 0)
	{
		watchlistConsumerConfig.consumerRole.oAuthCredentialCount = oAuthCredentialCount;

		watchlistConsumerConfig.consumerRole.pOAuthCredentialList = (RsslReactorOAuthCredential**)malloc(sizeof(RsslReactorOAuthCredential*) * oAuthCredentialCount);

		if (watchlistConsumerConfig.consumerRole.pOAuthCredentialList == NULL)
		{
			printf("Unable to allocate login request list on consumerRole\n");
			clearAllocatedMemory();
			printUsageAndExit(argc, argv);
		}

		for (i = 0; i < oAuthCredentialCount; i++)
		{
			watchlistConsumerConfig.consumerRole.pOAuthCredentialList[i] = &oAuthCredentialList[i].oAuthCredential;
		}
	}

	{
		RsslPreferredHostOptions* pPreferredHostOpts = &watchlistConsumerConfig.connectionOpts.preferredHostOptions;

		rsslClearRsslPreferredHostOptions(pPreferredHostOpts);

		pPreferredHostOpts->enablePreferredHostOptions = preferredHostConfig.preferredHostOptions.enablePreferredHostOptions;
		pPreferredHostOpts->detectionTimeSchedule.data = preferredHostConfig.preferredHostOptions.detectionTimeSchedule.data;
		pPreferredHostOpts->detectionTimeSchedule.length = preferredHostConfig.preferredHostOptions.detectionTimeSchedule.length;
		pPreferredHostOpts->detectionTimeInterval = preferredHostConfig.preferredHostOptions.detectionTimeInterval;
		pPreferredHostOpts->connectionListIndex = preferredHostConfig.preferredHostOptions.connectionListIndex;
		pPreferredHostOpts->warmStandbyGroupListIndex = preferredHostConfig.preferredHostOptions.warmStandbyGroupListIndex;
		pPreferredHostOpts->fallBackWithInWSBGroup = preferredHostConfig.preferredHostOptions.fallBackWithInWSBGroup;

		printf("Preferred Host enabled: %s\n\n", (pPreferredHostOpts->enablePreferredHostOptions ? "Yes" : "No"));
	}

	if (preferredHostConfig.ioctlCallTimeInterval == 0 && setIoctlPreferredHostOptValue == RSSL_TRUE)
	{
		printf("Error: -ioctlInterval should be specified and have a non-zero positive value if any ioctl Preferred host parameters are specified.\n\n");
		clearAllocatedMemory();
		printUsageAndExit(argc, argv);
	}

	printf(	"  ServiceName: %s\n"
			"  Run Time: %us\n",
			watchlistConsumerConfig.serviceName.data,
			watchlistConsumerConfig.runTime);

	printf("\n");

	if (watchlistConsumerConfig.itemCount == 0)
	{
		addItem((char*)"TRI.N", RSSL_DMT_MARKET_PRICE, RSSL_FALSE);
	}
}

ItemInfo *getItemInfo(RsslInt32 streamId)
{
	if (streamId > 0)
	{
		int itemLocation = streamId - ITEMS_MIN_STREAM_ID;
		if (streamId >= ITEMS_MIN_STREAM_ID && streamId < (RsslInt32)watchlistConsumerConfig.itemCount
				+ ITEMS_MIN_STREAM_ID)
			return &watchlistConsumerConfig.itemList[streamId - ITEMS_MIN_STREAM_ID];
		else
			return NULL;
	}
	else
		return NULL;
}

RsslBool isXmlTracingEnabled()
{
	return xmlTrace;
}
