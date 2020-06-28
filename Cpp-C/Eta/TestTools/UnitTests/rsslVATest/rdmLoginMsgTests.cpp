/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "testFramework.h"
#include "rsslVATestUtil.h"
#include "gtest/gtest.h"
#include "rtr/rsslVAUtils.h"

static TypedMessageStats stats;
static TestWriteAction testWriteAction;

void loginRequestMsgTests();
void loginRequestBlankTests();
void loginConnStatusMsgTests();
void loginCloseMsgTests();
void loginRefreshMsgTests();
void loginRefreshBlankTests();
void loginStatusMsgTests();
void loginStatusBlankTests();
void loginPostAndAckTests();
void loginRTTMsgTests();

TEST(LoginMsgTest, RequestMsgTests)
{
	loginRequestMsgTests();
}

TEST(LoginMsgTest, RequestBlankTests)
{
	loginRequestBlankTests();
}

TEST(LoginMsgTest, CloseMsgTests)
{
	loginCloseMsgTests();
}

TEST(LoginMsgTest, ConnStatusMsgTests)
{
	loginConnStatusMsgTests();
}

TEST(LoginMsgTest, RefreshMsgTests)
{
	loginRefreshMsgTests();
}

TEST(LoginMsgTest, RefreshBlankTests)
{
	loginRefreshBlankTests();
}

TEST(LoginMsgTest, StatusMsgTests)
{
	loginStatusMsgTests();
}

TEST(LoginMsgTest, StatusBlankTests)
{
	loginStatusBlankTests();
}

TEST(LoginMsgTest, PostAndAckTests)
{
	loginPostAndAckTests();
}

TEST(LoginMsgTest, RTTMsgTests)
{
	loginRTTMsgTests();
}

void loginRequestMsgTests()
{
	RsslRDMLoginRequest encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMLoginRequest *pDecRDMMsg;
	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA,
		RDM_LG_RQF_HAS_APPLICATION_ID,
		RDM_LG_RQF_HAS_APPLICATION_NAME,
		RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN,
		RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG,
		RDM_LG_RQF_HAS_INSTANCE_ID,
		RDM_LG_RQF_HAS_PASSWORD,
		RDM_LG_RQF_HAS_POSITION,
		RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR,
		RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE,
		RDM_LG_RQF_HAS_ROLE,
		RDM_LG_RQF_HAS_SINGLE_OPEN,
		RDM_LG_RQF_HAS_USERNAME_TYPE,
		RDM_LG_RQF_PAUSE_ALL,
		RDM_LG_RQF_NO_REFRESH,
		RDM_LG_RQF_HAS_SUPPORT_PROV_DIC_DOWNLOAD
	};
	RsslUInt32 *flagsList, flagsListCount;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt allowSuspectData = 2;
	RsslBuffer userName = rssl_init_buffer_from_string(const_cast<char*>("userName"));
	RsslBuffer applicationId = rssl_init_buffer_from_string(const_cast<char*>("applicationId"));
	RsslBuffer applicationName = rssl_init_buffer_from_string(const_cast<char*>("applicationName"));
	RsslBuffer appAuthToken = rssl_init_buffer_from_string(const_cast<char*>("appauthtoken"));
	RsslUInt downloadConnectionConfig = 2;
	RsslBuffer instanceId = rssl_init_buffer_from_string(const_cast<char*>("instanceId"));
	RsslBuffer password = rssl_init_buffer_from_string(const_cast<char*>("password"));
	RsslBuffer position = rssl_init_buffer_from_string(const_cast<char*>("position"));
	RsslBuffer authnToken = rssl_init_buffer_from_string(const_cast<char*>("AuthenticationToken"));
	RsslBuffer authnExtended = rssl_init_buffer_from_string(const_cast<char*>("ExtendAuth"));
	RsslUInt providePermissionProfile = 2;
	RsslUInt providePermissionExpressions = 2;
	RsslUInt role = RDM_LOGIN_ROLE_PROV;
	RsslUInt singleOpen = 2;
	RsslUInt supportDictionaryDownload = 3;
	RsslUInt8 userNameType = RDM_LOGIN_USER_TOKEN;

	clearTypedMessageStats(&stats);

	/* Request */
	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	for (i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;
		RsslUInt32 reqFlags = flagsList[i];
		int authnFlag, authExtFlag;
		userNameType = RDM_LOGIN_USER_TOKEN;

		for (authnFlag = 0; authnFlag <= 1; ++authnFlag)
		{
			if (authnFlag == 1)
			{
				userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;
				reqFlags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
				authExtFlag = 1;
			}
			else
				authExtFlag = 0;

			for (authExtFlag; authExtFlag >= 0; --authExtFlag)
			{
				if (authExtFlag == 1)
					reqFlags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;

				for (j = 0; j < testWriteActionsCount; ++j)
				{
					RsslRet ret;
					testWriteAction = testWriteActions[j];

					/*** Encode ***/
					rsslClearRDMLoginRequest(&encRDMMsg);
					ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN);
					ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

					encRDMMsg.flags = reqFlags;

					encRDMMsg.rdmMsgBase.streamId = streamId;

					if (authnFlag == 1)
						encRDMMsg.userName = authnToken;
					else
						encRDMMsg.userName = userName;

					/* Set parameters based on flags */
					if (encRDMMsg.flags & RDM_LG_RQF_HAS_USERNAME_TYPE)
						encRDMMsg.userNameType = userNameType;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA)
						encRDMMsg.allowSuspectData = allowSuspectData;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_APPLICATION_ID)
						encRDMMsg.applicationId = applicationId;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN)
						encRDMMsg.applicationAuthorizationToken = appAuthToken;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_APPLICATION_NAME)
						encRDMMsg.applicationName = applicationName;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG)
						encRDMMsg.downloadConnectionConfig = downloadConnectionConfig;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_INSTANCE_ID)
						encRDMMsg.instanceId = instanceId;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_PASSWORD)
						encRDMMsg.password = password;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_POSITION)
						encRDMMsg.position = position;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR)
						encRDMMsg.providePermissionExpressions = providePermissionExpressions;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE)
						encRDMMsg.providePermissionProfile = providePermissionProfile;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_ROLE)
						encRDMMsg.role = role;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_SINGLE_OPEN)
						encRDMMsg.singleOpen = singleOpen;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_SUPPORT_PROV_DIC_DOWNLOAD)
						encRDMMsg.supportProviderDictionaryDownload = supportDictionaryDownload;

					if (encRDMMsg.flags & RDM_LG_RQF_HAS_AUTHN_EXTENDED)
						encRDMMsg.authenticationExtended = authnExtended;

					if (testWriteAction != TEST_EACTION_CREATE_COPY)
					{
						writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
							&rsslMsg, &decRDMMsg,
							&stats);
						pDecRDMMsg = &decRDMMsg.loginMsg.request;
					}
					else
						ASSERT_TRUE((pDecRDMMsg = (RsslRDMLoginRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

					ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
					ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
					ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

					if (pDecRDMMsg->flags != reqFlags)
						printf("flags: 0x%X req: 0x%X", pDecRDMMsg->flags, reqFlags);

					ASSERT_TRUE(pDecRDMMsg->flags == reqFlags);

					/* Check parameters */
					if (userNameType != RDM_LOGIN_USER_AUTHN_TOKEN)
					{
						ASSERT_TRUE(pDecRDMMsg->userName.data != userName.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->userName, &userName));
					}
					else
					{
						ASSERT_TRUE(pDecRDMMsg->userName.data != userName.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->userName, &authnToken));
					}

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_USERNAME_TYPE)
						ASSERT_TRUE(pDecRDMMsg->userNameType == userNameType);

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA)
						ASSERT_TRUE(pDecRDMMsg->allowSuspectData == allowSuspectData);

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_APPLICATION_ID)
					{
						ASSERT_TRUE(pDecRDMMsg->applicationId.data != applicationId.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->applicationId, &applicationId));
					}

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_APPLICATION_NAME)
					{
						ASSERT_TRUE(pDecRDMMsg->applicationName.data != applicationName.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->applicationName, &applicationName));
					}

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN)
					{
						ASSERT_TRUE(pDecRDMMsg->applicationAuthorizationToken.data != appAuthToken.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->applicationAuthorizationToken, &appAuthToken));
					}


					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG)
						ASSERT_TRUE(pDecRDMMsg->downloadConnectionConfig == downloadConnectionConfig);

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_INSTANCE_ID)
					{
						ASSERT_TRUE(pDecRDMMsg->instanceId.data != instanceId.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->instanceId, &instanceId));
					}

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_PASSWORD)
					{
						ASSERT_TRUE(pDecRDMMsg->password.data != password.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->password, &password));
					}

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_POSITION)
					{
						ASSERT_TRUE(pDecRDMMsg->position.data != position.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->position, &position));
					}

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR)
						ASSERT_TRUE(pDecRDMMsg->providePermissionExpressions == providePermissionExpressions);

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE)
						ASSERT_TRUE(pDecRDMMsg->providePermissionProfile == providePermissionProfile);

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_ROLE)
						ASSERT_TRUE(pDecRDMMsg->role == role);

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_SINGLE_OPEN)
						ASSERT_TRUE(pDecRDMMsg->singleOpen == singleOpen);

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_SUPPORT_PROV_DIC_DOWNLOAD)
						ASSERT_TRUE(pDecRDMMsg->supportProviderDictionaryDownload == supportDictionaryDownload);

					if (pDecRDMMsg->flags & RDM_LG_RQF_HAS_AUTHN_EXTENDED)
					{
						ASSERT_TRUE(pDecRDMMsg->authenticationExtended.data != authnExtended.data); /* deep copy check */
						ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->authenticationExtended, &authnExtended));
					}

					if (testWriteAction == TEST_EACTION_CREATE_COPY)
						free(pDecRDMMsg);

				}
			}
		}
	}

	free(flagsList);

	////printTypedMessageStats(&stats);
}

void loginRequestBlankTests()
{
	RsslRDMLoginRequest encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMLoginRequest *pDecRDMMsg;
	RsslMsg encMsg;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslEncodeIterator eIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslDecodeIterator dIter = RSSL_INIT_DECODE_ITERATOR;
	RsslBuffer msgBuffer, memoryBuffer;
	RsslErrorInfo error;
	RsslRet ret;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt allowSuspectData = 2;
	RsslBuffer userName = rssl_init_buffer_from_string(const_cast<char*>("userName"));
	RsslBuffer blankBuffer = { 0,0 };

	/*** Encode test to make sure no 0 length buffers go on the wire ***/
	rsslClearRDMLoginRequest(&encRDMMsg);

	encRDMMsg.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE | RDM_LG_RQF_HAS_AUTHN_EXTENDED | RDM_LG_RQF_HAS_APPLICATION_ID | RDM_LG_RQF_HAS_APPLICATION_NAME
		| RDM_LG_RQF_HAS_INSTANCE_ID | RDM_LG_RQF_HAS_PASSWORD  | RDM_LG_RQF_HAS_POSITION;

	encRDMMsg.rdmMsgBase.streamId = streamId;

	encRDMMsg.userName = userName;
	
	encRDMMsg.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;

	encRDMMsg.applicationId = blankBuffer;
	encRDMMsg.applicationName = blankBuffer;
	encRDMMsg.instanceId = blankBuffer;
	encRDMMsg.password = blankBuffer;
	encRDMMsg.position = blankBuffer;
	encRDMMsg.authenticationExtended = blankBuffer;

	setupEncodeIterator(&eIter, &msgBuffer);
	ret = rsslEncodeRDMMsg(&eIter, (RsslRDMMsg*)&encRDMMsg, &msgBuffer.length, &error);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);


	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	pDecRDMMsg = &decRDMMsg.loginMsg.request;

	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	ASSERT_TRUE(pDecRDMMsg->flags == RDM_LG_RQF_HAS_USERNAME_TYPE);


	/* Test set 2: Ensure that Decode handles blank types */
	/* AppID */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRequestMsg((RsslRequestMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.requestMsg.flags = RSSL_RQMF_STREAMING;

	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	/* ApplicationId */
	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_APPID;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.request.flags |= RDM_LG_RQF_HAS_APPLICATION_ID && decRDMMsg.loginMsg.request.applicationId.length == 0);

	/* AppName */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRequestMsg((RsslRequestMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.requestMsg.flags = RSSL_RQMF_STREAMING;

	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_APPNAME;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.request.flags |= RDM_LG_RQF_HAS_APPLICATION_NAME && decRDMMsg.loginMsg.request.applicationName.length == 0);

	/* AppAuthToken */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRequestMsg((RsslRequestMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.requestMsg.flags = RSSL_RQMF_STREAMING;

	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_APPAUTH_TOKEN;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.request.flags |= RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN && decRDMMsg.loginMsg.request.applicationAuthorizationToken.length == 0);

	/* Position */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRequestMsg((RsslRequestMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.requestMsg.flags = RSSL_RQMF_STREAMING;

	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_POSITION;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.request.flags |= RDM_LG_RQF_HAS_POSITION && decRDMMsg.loginMsg.request.position.length == 0);

	/* Password */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRequestMsg((RsslRequestMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.requestMsg.flags = RSSL_RQMF_STREAMING;

	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_PASSWORD;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.request.flags |= RDM_LG_RQF_HAS_PASSWORD && decRDMMsg.loginMsg.request.password.length == 0);

	/* Instance ID */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRequestMsg((RsslRequestMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.requestMsg.flags = RSSL_RQMF_STREAMING;

	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_INST_ID;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.request.flags |= RDM_LG_RQF_HAS_INSTANCE_ID && decRDMMsg.loginMsg.request.instanceId.length == 0);
}


void loginConnStatusMsgTests()
{
	RsslRDMLoginConsumerConnectionStatus encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMLoginConsumerConnectionStatus *pDecRDMMsg;
	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_LG_CCSF_HAS_WARM_STANDBY_INFO
	};
	RsslUInt32 *flagsList, flagsListCount;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt warmStandbyMode = 2;

	clearTypedMessageStats(&stats);

	/* ConnStatus */
	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	for (i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslRet ret;
			testWriteAction = testWriteActions[j];
			rsslClearRDMLoginConsumerConnectionStatus(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);

			encRDMMsg.flags = flagsList[i];

			encRDMMsg.rdmMsgBase.streamId = streamId;


			/* Set parameters based on flags */
			if (encRDMMsg.flags & RDM_LG_CCSF_HAS_WARM_STANDBY_INFO)
			{
				encRDMMsg.warmStandbyInfo.action = RSSL_MPEA_ADD_ENTRY;
				encRDMMsg.warmStandbyInfo.warmStandbyMode = warmStandbyMode;
			}

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.loginMsg.consumerConnectionStatus;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMLoginConsumerConnectionStatus*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret))!= NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);

			ASSERT_TRUE(pDecRDMMsg->flags == flagsList[i]);

			/* Check parameters */
			if (pDecRDMMsg->flags & RDM_LG_CCSF_HAS_WARM_STANDBY_INFO)
			{
				ASSERT_TRUE(pDecRDMMsg->warmStandbyInfo.action == RSSL_MPEA_ADD_ENTRY);
				ASSERT_TRUE(pDecRDMMsg->warmStandbyInfo.warmStandbyMode == warmStandbyMode);
			}

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);
		}
	}

	/* Test delete action */
	for (i = 0; i < testWriteActionsCount; ++i)
	{
		RsslRet ret;
		testWriteAction = testWriteActions[i];
		rsslClearRDMLoginConsumerConnectionStatus(&encRDMMsg);

		encRDMMsg.rdmMsgBase.streamId = streamId;
		encRDMMsg.flags = RDM_LG_CCSF_HAS_WARM_STANDBY_INFO;
		encRDMMsg.warmStandbyInfo.action = RSSL_MPEA_DELETE_ENTRY;

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.loginMsg.consumerConnectionStatus;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMLoginConsumerConnectionStatus*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);

		ASSERT_TRUE(pDecRDMMsg->flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);

		/* Check parameters */
		ASSERT_TRUE(pDecRDMMsg->warmStandbyInfo.action == RSSL_MPEA_DELETE_ENTRY);

		if (testWriteAction == TEST_EACTION_CREATE_COPY)
			free(pDecRDMMsg);
	}

	free(flagsList);

	//printTypedMessageStats(&stats);
}

void loginCloseMsgTests()
{
	RsslRDMLoginClose encRDMMsg, *pDecRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;

	/* Parameters to test with */
	RsslInt32 streamId = -5;

	RsslUInt32 j;

	clearTypedMessageStats(&stats);

	for (j = 0; j < testWriteActionsCount; ++j)
	{
		RsslRet ret;
		testWriteAction = testWriteActions[j];

		/*** Encode ***/
		rsslClearRDMLoginClose(&encRDMMsg);
		ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_CLOSE);

		encRDMMsg.rdmMsgBase.streamId = streamId;

		if (testWriteAction != TEST_EACTION_CREATE_COPY)
		{
			writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
					&rsslMsg, &decRDMMsg,
					&stats);
			pDecRDMMsg = &decRDMMsg.loginMsg.close;
		}
		else
			ASSERT_TRUE((pDecRDMMsg = (RsslRDMLoginClose*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CLOSE);

		if (testWriteAction == TEST_EACTION_CREATE_COPY)
			free(pDecRDMMsg);
	}

	//printTypedMessageStats(&stats);
}

void loginPostAndAckTests()
{
	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;

	RsslBuffer msgBuffer;
	RsslBuffer memoryBuffer;
	RsslEncodeIterator eIter;
	RsslDecodeIterator dIter;
	RsslErrorInfo rsslErrorInfo;
	
	/* Post & Ack Msgs have no structure but rsslDecodeRDMMsg should be able to pass them through */

	/* Parameters to test with */
	RsslInt32 streamId = -5;

	/* Post Msg */

	/*** Encode ***/
	rsslClearPostMsg((RsslPostMsg*)&rsslMsg);
	rsslMsg.msgBase.msgClass = RSSL_MC_POST;
	rsslMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	rsslMsg.msgBase.streamId = streamId;
	rsslMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	setupEncodeIterator(&eIter, &msgBuffer);

	ASSERT_TRUE(rsslEncodeMsg(&eIter, &rsslMsg) == RSSL_RET_SUCCESS);

	/*** Decode ***/

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);

	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslMsg.msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(decRDMMsg.rdmMsgBase.streamId == streamId);
	ASSERT_TRUE(decRDMMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(decRDMMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_POST);

	/* Ack Msg */

	/*** Encode ***/
	rsslClearAckMsg((RsslAckMsg*)&rsslMsg);
	rsslMsg.msgBase.msgClass = RSSL_MC_ACK;
	rsslMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	rsslMsg.msgBase.streamId = streamId;
	rsslMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	setupEncodeIterator(&eIter, &msgBuffer);

	ASSERT_TRUE(rsslEncodeMsg(&eIter, &rsslMsg) == RSSL_RET_SUCCESS);

	/*** Decode ***/

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);

	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &rsslErrorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslMsg.msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(decRDMMsg.rdmMsgBase.streamId == streamId);
	ASSERT_TRUE(decRDMMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(decRDMMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_ACK);
}

void loginRefreshMsgTests()
{
	RsslRDMLoginRefresh encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMLoginRefresh *pDecRDMMsg;
	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA,
		RDM_LG_RFF_HAS_APPLICATION_ID,
		RDM_LG_RFF_HAS_APPLICATION_NAME,
		RDM_LG_RFF_HAS_POSITION,
		RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR,
		RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE,
		RDM_LG_RFF_HAS_SINGLE_OPEN,
		RDM_LG_RFF_HAS_SUPPORT_BATCH,
		RDM_LG_RFF_HAS_SUPPORT_POST,
		RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE,
		RDM_LG_RFF_HAS_SUPPORT_VIEW,
		RDM_LG_RFF_HAS_SUPPORT_STANDBY,
		RDM_LG_RFF_SOLICITED,
		RDM_LG_RFF_HAS_SEQ_NUM,
		RDM_LG_RFF_CLEAR_CACHE,
		RDM_LG_RFF_HAS_SUPPORT_ENH_SL,
		RDM_LG_RFF_HAS_SUPPORT_PROV_DIC_DOWNLOAD,
		RDM_LG_RFF_HAS_SEQ_RETRY_INTERVAL,
		RDM_LG_RFF_HAS_UPDATE_BUF_LIMIT,
		RDM_LG_RFF_HAS_SEQ_NUM_RECOVERY,
		RDM_LG_RFF_HAS_AUTHN_TT_REISSUE,
		RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP,
		RDM_LG_RFF_HAS_AUTHN_ERROR_CODE,
		RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT
		/* RDM_LG_RFF_HAS_CONN_CONFIG */ /* Handled by loop that sends serverList */
	};
	RsslUInt32 *flagsList, flagsListCount;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslBuffer userName = rssl_init_buffer_from_string(const_cast<char*>("userName"));
	RsslUInt8 userNameType = 2;
	RsslBuffer applicationId = rssl_init_buffer_from_string(const_cast<char*>("applicationId"));
	RsslBuffer applicationName = rssl_init_buffer_from_string(const_cast<char*>("applicationName"));
	RsslBuffer position = rssl_init_buffer_from_string(const_cast<char*>("position"));
	RsslState state = { RSSL_STREAM_OPEN, RSSL_DATA_SUSPECT, RSSL_SC_FAILOVER_COMPLETED, rssl_init_buffer_from_string(const_cast<char*>("In Soviet Russia, state tests you!")) };
	RsslUInt providePermissionProfile = 2;
	RsslUInt providePermissionExpressions = 2;
	RsslUInt singleOpen = 3;
	RsslUInt allowSuspectData = 4;
	RsslUInt supportOMMPost = 5;
	RsslUInt supportStandby = 6;
	RsslUInt supportBatchRequests = 7;
	RsslUInt supportViewRequests = 8;
	RsslUInt supportOptimizedPauseResume = 9;
	RsslUInt32 seqNum = 10;
	RsslUInt numStandbyServers = 11;
	RsslUInt supportEnhancedSymbolList = 12;
	RsslUInt supportDictionaryDownload = 13;
	RsslUInt sequenceRetryInterval = 14;
	RsslUInt updateBufferLimit = 101;
	RsslUInt sequenceNumberRecovery = 0;
	RsslUInt authnTTReissue = 12345678;
	RsslBuffer authnExtResponse = rssl_init_buffer_from_string(const_cast<char*>("extendedResp"));
	RsslUInt authnErrorCode = 404;
	RsslBuffer authnErrorText = rssl_init_buffer_from_string(const_cast<char*>("errorText"));


	RsslRDMServerInfo serverList[3];
	char hostnameString[3][32];
	RsslUInt32 serverListSize = 3;
	
	for(i = 0; i < serverListSize; ++i)
	{
		serverList[i].flags = RDM_LG_SIF_HAS_LOAD_FACTOR | RDM_LG_SIF_HAS_TYPE;
		serverList[i].serverIndex = i;
		serverList[i].hostname.data = hostnameString[i];
		snprintf(serverList[i].hostname.data, sizeof(hostnameString[i]), "host%d", i+1);
		serverList[i].hostname.length = (RsslUInt32)strlen(serverList[i].hostname.data);
		serverList[i].port = i+2;
		serverList[i].loadFactor = i+3;
		serverList[i].serverType = i+4;
	}

	clearTypedMessageStats(&stats);

	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	for (i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;
		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslUInt32 k;

			testWriteAction = testWriteActions[j];

			for(k = -1; k <= serverListSize; ++k)
			{
				RsslRet ret;

				/*** Encode ***/
				rsslClearRDMLoginRefresh(&encRDMMsg);
				ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN);
				ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);

				encRDMMsg.flags = flagsList[i];

				encRDMMsg.rdmMsgBase.streamId = streamId;

				encRDMMsg.state = state;

				/* Set parameters based on flags */
				if (encRDMMsg.flags & RDM_LG_RFF_HAS_USERNAME)
					encRDMMsg.userName = userName;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_USERNAME_TYPE)
					encRDMMsg.userNameType = userNameType;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA)
					encRDMMsg.allowSuspectData = allowSuspectData;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_APPLICATION_ID)
					encRDMMsg.applicationId = applicationId;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_APPLICATION_NAME)
					encRDMMsg.applicationName = applicationName;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE)
					encRDMMsg.providePermissionProfile = providePermissionProfile;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR)
					encRDMMsg.providePermissionExpressions = providePermissionExpressions;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SINGLE_OPEN)
					encRDMMsg.singleOpen = singleOpen;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SUPPORT_BATCH)
					encRDMMsg.supportBatchRequests = supportBatchRequests;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SUPPORT_PROV_DIC_DOWNLOAD)
					encRDMMsg.supportProviderDictionaryDownload = supportDictionaryDownload;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SEQ_RETRY_INTERVAL)
					encRDMMsg.sequenceRetryInterval = sequenceRetryInterval;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_UPDATE_BUF_LIMIT)
					encRDMMsg.updateBufferLimit = updateBufferLimit;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SEQ_NUM_RECOVERY)
					encRDMMsg.sequenceNumberRecovery = sequenceNumberRecovery;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SUPPORT_ENH_SL)
					encRDMMsg.supportEnhancedSymbolList = supportEnhancedSymbolList;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SUPPORT_VIEW)
					encRDMMsg.supportViewRequests = supportViewRequests;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SUPPORT_STANDBY)
					encRDMMsg.supportStandby = supportStandby;

				if (encRDMMsg.flags & RDM_LG_RFF_HAS_SEQ_NUM)
					encRDMMsg.sequenceNumber = seqNum;
				
				if(encRDMMsg.flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
					encRDMMsg.authenticationTTReissue = authnTTReissue;
				
				if(encRDMMsg.flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP)
					encRDMMsg.authenticationExtendedResp = authnExtResponse;
				
				if(encRDMMsg.flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
					encRDMMsg.authenticationErrorCode = authnErrorCode;
				
				if(encRDMMsg.flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT)
					encRDMMsg.authenticationErrorText = authnErrorText;

				if (k >= 0)
				{
					encRDMMsg.flags |= RDM_LG_RFF_HAS_CONN_CONFIG;
					encRDMMsg.numStandbyServers = numStandbyServers;
					encRDMMsg.serverCount = k;
					encRDMMsg.serverList = serverList;
				}

				if (testWriteAction != TEST_EACTION_CREATE_COPY)
				{
					writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
							&rsslMsg, &decRDMMsg,
							&stats);
					pDecRDMMsg = &decRDMMsg.loginMsg.refresh;
				}
				else
					ASSERT_TRUE((pDecRDMMsg = (RsslRDMLoginRefresh*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);

				ASSERT_TRUE(pDecRDMMsg->flags == (flagsList[i] | ((k >= 0) ? RDM_LG_RFF_HAS_CONN_CONFIG : 0)));

				/* Check parameters */
				ASSERT_TRUE(pDecRDMMsg->state.streamState == state.streamState);
				ASSERT_TRUE(pDecRDMMsg->state.dataState == state.dataState);
				ASSERT_TRUE(pDecRDMMsg->state.code == state.code);
				ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->state.text, &state.text));
				ASSERT_TRUE(pDecRDMMsg->state.text.data != state.text.data); /* deep-copy check */

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_USERNAME)
				{
					ASSERT_TRUE(pDecRDMMsg->userName.data != userName.data); /* deep copy check */
					ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->userName, &userName));
				}

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_USERNAME_TYPE)
					ASSERT_TRUE(pDecRDMMsg->userNameType == userNameType);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA)
					ASSERT_TRUE(pDecRDMMsg->allowSuspectData == allowSuspectData);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_APPLICATION_ID)
				{
					ASSERT_TRUE(pDecRDMMsg->applicationId.data != applicationId.data); /* deep copy check */
					ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->applicationId, &applicationId));
				}

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_APPLICATION_NAME)
				{
					ASSERT_TRUE(pDecRDMMsg->applicationName.data != applicationName.data); /* deep copy check */
					ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->applicationName, &applicationName));
				}

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE)
					ASSERT_TRUE(pDecRDMMsg->providePermissionProfile == providePermissionProfile);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR)
					ASSERT_TRUE(pDecRDMMsg->providePermissionExpressions == providePermissionExpressions);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SINGLE_OPEN)
					ASSERT_TRUE(pDecRDMMsg->singleOpen == singleOpen);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SUPPORT_BATCH)
					ASSERT_TRUE(pDecRDMMsg->supportBatchRequests == supportBatchRequests);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SUPPORT_PROV_DIC_DOWNLOAD)
					ASSERT_TRUE(pDecRDMMsg->supportProviderDictionaryDownload == supportDictionaryDownload);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SEQ_RETRY_INTERVAL)
					ASSERT_TRUE(pDecRDMMsg->sequenceRetryInterval == sequenceRetryInterval);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_UPDATE_BUF_LIMIT)
					ASSERT_TRUE(pDecRDMMsg->updateBufferLimit == updateBufferLimit);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SEQ_NUM_RECOVERY)
					ASSERT_TRUE(pDecRDMMsg->sequenceNumberRecovery == sequenceNumberRecovery);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SUPPORT_ENH_SL)
					ASSERT_TRUE(pDecRDMMsg->supportEnhancedSymbolList == supportEnhancedSymbolList);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SUPPORT_VIEW)
					ASSERT_TRUE(pDecRDMMsg->supportViewRequests == supportViewRequests);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SUPPORT_STANDBY)
					ASSERT_TRUE(pDecRDMMsg->supportStandby == supportStandby);

				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_SEQ_NUM)
					ASSERT_TRUE(pDecRDMMsg->sequenceNumber == seqNum);
				
				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
					ASSERT_TRUE(pDecRDMMsg->authenticationTTReissue == authnTTReissue);
				
				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP)
				{
					ASSERT_TRUE(pDecRDMMsg->authenticationExtendedResp.data != authnExtResponse.data); /* deep copy check */
					ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->authenticationExtendedResp, &authnExtResponse));
				}
				
				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
					ASSERT_TRUE(pDecRDMMsg->authenticationErrorCode == authnErrorCode);
				
				if (pDecRDMMsg->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT)
				{
					ASSERT_TRUE(pDecRDMMsg->authenticationErrorText.data != authnErrorText.data); /* deep copy check */
					ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->authenticationErrorText, &authnErrorText));
				}


				if (k >= 0)
				{
					RsslUInt32 l;

					ASSERT_TRUE(encRDMMsg.flags & RDM_LG_RFF_HAS_CONN_CONFIG);
					ASSERT_TRUE(pDecRDMMsg->serverCount == k);
					ASSERT_TRUE(pDecRDMMsg->numStandbyServers == numStandbyServers);
					for(l = 0; l < pDecRDMMsg->serverCount; ++l)
					{
						ASSERT_TRUE(pDecRDMMsg->serverList[l].flags == (RDM_LG_SIF_HAS_LOAD_FACTOR | RDM_LG_SIF_HAS_TYPE));
						ASSERT_TRUE(pDecRDMMsg->serverList[l].serverIndex == l);
						ASSERT_TRUE(rsslBufferIsEqual(&serverList[l].hostname, &pDecRDMMsg->serverList[l].hostname));
						ASSERT_TRUE(pDecRDMMsg->serverList[l].hostname.data != serverList[l].hostname.data); /* deep-copy check */
						ASSERT_TRUE(pDecRDMMsg->serverList[l].port == l+2);
						ASSERT_TRUE(pDecRDMMsg->serverList[l].loadFactor == l+3);
						ASSERT_TRUE(pDecRDMMsg->serverList[l].serverType == l+4);
					}

				}

				if (testWriteAction == TEST_EACTION_CREATE_COPY)
					free(pDecRDMMsg);

			}
		}
	}

	free(flagsList);

	//printTypedMessageStats(&stats);
}

void loginRefreshBlankTests()
{
	RsslRDMLoginRefresh encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMLoginRefresh *pDecRDMMsg;
	RsslMsg encMsg;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslEncodeIterator eIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslDecodeIterator dIter = RSSL_INIT_DECODE_ITERATOR;
	RsslBuffer msgBuffer, memoryBuffer;
	RsslErrorInfo error;
	RsslRet ret;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt allowSuspectData = 2;
	RsslBuffer userName = rssl_init_buffer_from_string(const_cast<char*>("userName"));
	RsslBuffer blankBuffer = { 0,0 };

	/*** Encode test to make sure no 0 length buffers go on the wire ***/
	rsslClearRDMLoginRefresh(&encRDMMsg);

	encRDMMsg.flags |= RDM_LG_RFF_HAS_APPLICATION_ID | RDM_LG_RFF_HAS_APPLICATION_NAME | RDM_LG_RFF_HAS_POSITION | RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP | RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT;

	encRDMMsg.rdmMsgBase.streamId = streamId;

	encRDMMsg.userName = userName;

	encRDMMsg.userNameType = RDM_LOGIN_USER_AUTHN_TOKEN;

	encRDMMsg.applicationId = blankBuffer;
	encRDMMsg.applicationName = blankBuffer;
	encRDMMsg.position = blankBuffer;
	encRDMMsg.authenticationExtendedResp = blankBuffer;
	encRDMMsg.authenticationErrorText = blankBuffer;

	setupEncodeIterator(&eIter, &msgBuffer);
	ret = rsslEncodeRDMMsg(&eIter, (RsslRDMMsg*)&encRDMMsg, &msgBuffer.length, &error);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);


	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	pDecRDMMsg = &decRDMMsg.loginMsg.refresh;

	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);

	ASSERT_TRUE(pDecRDMMsg->flags == 0);


	/* Test set 2: Ensure that Decode properly errors out when blank types are present */
	/* AppID */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRefreshMsg((RsslRefreshMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;

	encMsg.refreshMsg.state.dataState = RSSL_DATA_OK;
	encMsg.refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_APPID;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.refresh.flags |= RDM_LG_RFF_HAS_APPLICATION_ID && decRDMMsg.loginMsg.refresh.applicationId.length == 0);

	/* AppName */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRefreshMsg((RsslRefreshMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;

	encMsg.refreshMsg.state.dataState = RSSL_DATA_OK;
	encMsg.refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_APPNAME;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.refresh.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME && decRDMMsg.loginMsg.refresh.applicationName.length == 0);

	/* Position */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRefreshMsg((RsslRefreshMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;

	encMsg.refreshMsg.state.dataState = RSSL_DATA_OK;
	encMsg.refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_POSITION;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.refresh.flags |= RDM_LG_RFF_HAS_POSITION && decRDMMsg.loginMsg.refresh.position.length == 0);

	/* Authentication Extended Resp */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRefreshMsg((RsslRefreshMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;

	encMsg.refreshMsg.state.dataState = RSSL_DATA_OK;
	encMsg.refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_BUFFER;
	elementEntry.name = RSSL_ENAME_AUTHN_EXTENDED_RESP;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.refresh.flags |= RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP && decRDMMsg.loginMsg.refresh.authenticationExtendedResp.length == 0);

	/* Authentication Error Text */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearRefreshMsg((RsslRefreshMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;

	encMsg.refreshMsg.state.dataState = RSSL_DATA_OK;
	encMsg.refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_AUTHN_ERROR_TEXT;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.refresh.flags |= RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT && decRDMMsg.loginMsg.refresh.authenticationErrorText.length == 0);

}

void loginStatusMsgTests()
{
	RsslRDMLoginStatus encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMLoginStatus *pDecRDMMsg;
	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_LG_STF_HAS_STATE,
		RDM_LG_STF_HAS_USERNAME,
		RDM_LG_STF_HAS_USERNAME_TYPE,
		RDM_LG_STF_HAS_AUTHN_ERROR_CODE,
		RDM_LG_STF_HAS_AUTHN_ERROR_TEXT
	};
	RsslUInt32 *flagsList, flagsListCount;
	RsslUInt authnErrorCode = 404;
	RsslBuffer authnErrorText = rssl_init_buffer_from_string(const_cast<char*>("ErrorText"));

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslBuffer userName = rssl_init_buffer_from_string(const_cast<char*>("userName"));
	RsslUInt8 userNameType = 2;
	RsslState state = { RSSL_STREAM_OPEN, RSSL_DATA_SUSPECT, RSSL_SC_FAILOVER_COMPLETED, rssl_init_buffer_from_string(const_cast<char*>("In Soviet Russia, state tests you!")) };

	clearTypedMessageStats(&stats);

	/* Status */
	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	for (i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslUInt32 statusFlags;
			RsslRet ret;

			testWriteAction = testWriteActions[j];

			/*** Encode ***/
			rsslClearRDMLoginStatus(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);

			encRDMMsg.flags = flagsList[i];

			encRDMMsg.rdmMsgBase.streamId = streamId;

			encRDMMsg.state = state;

			/* Set parameters based on flags */
			if (encRDMMsg.flags & RDM_LG_STF_HAS_USERNAME)
			{
				encRDMMsg.userName = userName;
				if (encRDMMsg.flags & RDM_LG_STF_HAS_USERNAME_TYPE)
					encRDMMsg.userNameType = userNameType;
			}
			
			if (encRDMMsg.flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE)
			{
				encRDMMsg.authenticationErrorCode = authnErrorCode;
			}
			
			if (encRDMMsg.flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT)
			{
				encRDMMsg.authenticationErrorText = authnErrorText;
			}

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.loginMsg.status;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMLoginStatus*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);


			/* When encoded, the userNameType will be dropped if userName is not present. */
			statusFlags = flagsList[i]; if (!(statusFlags & RDM_LG_STF_HAS_USERNAME) && (testWriteAction == TEST_EACTION_ENCODE)) statusFlags &= ~RDM_LG_STF_HAS_USERNAME_TYPE;

			ASSERT_TRUE(pDecRDMMsg->flags == statusFlags);
			

			/* Check parameters */
			if (pDecRDMMsg->flags & RDM_LG_STF_HAS_USERNAME)
			{
				ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->userName, &userName));
				ASSERT_TRUE(pDecRDMMsg->userName.data != userName.data); /* deep-copy check */

				if (pDecRDMMsg->flags & RDM_LG_STF_HAS_USERNAME_TYPE)
					ASSERT_TRUE(pDecRDMMsg->userNameType == userNameType);

			}

			if (pDecRDMMsg->flags & RDM_LG_STF_HAS_STATE)
			{
				ASSERT_TRUE(pDecRDMMsg->state.streamState == state.streamState);
				ASSERT_TRUE(pDecRDMMsg->state.dataState == state.dataState);
				ASSERT_TRUE(pDecRDMMsg->state.code == state.code);
				ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->state.text, &state.text));
				ASSERT_TRUE(pDecRDMMsg->state.text.data != state.text.data); /* deep-copy check */
			}
			
			if (pDecRDMMsg->flags & RDM_LG_STF_HAS_AUTHN_ERROR_CODE)
			{
				ASSERT_TRUE(pDecRDMMsg->authenticationErrorCode == authnErrorCode);
			}
			
			if (pDecRDMMsg->flags & RDM_LG_STF_HAS_AUTHN_ERROR_TEXT)
			{
				ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->authenticationErrorText, &authnErrorText));
				ASSERT_TRUE(pDecRDMMsg->authenticationErrorText.data != authnErrorText.data); /* deep-copy check */
			}

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);
		}
	}

	free(flagsList);

	//printTypedMessageStats(&stats);
}


void loginStatusBlankTests()
{
	RsslRDMLoginStatus encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMLoginStatus *pDecRDMMsg;
	RsslMsg encMsg;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslEncodeIterator eIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslDecodeIterator dIter = RSSL_INIT_DECODE_ITERATOR;
	RsslBuffer msgBuffer, memoryBuffer;
	RsslErrorInfo error;
	RsslRet ret;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt allowSuspectData = 2;
	RsslBuffer userName = rssl_init_buffer_from_string(const_cast<char*>("userName"));
	RsslBuffer blankBuffer = { 0,0 };

	/*** Encode test to make sure no 0 length buffers go on the wire ***/
	rsslClearRDMLoginStatus((RsslRDMLoginStatus*)&encRDMMsg);

	encRDMMsg.flags |= RDM_LG_STF_HAS_AUTHN_ERROR_TEXT;

	encRDMMsg.rdmMsgBase.streamId = streamId;

	encRDMMsg.userName = userName;

	encRDMMsg.authenticationErrorText = blankBuffer;

	setupEncodeIterator(&eIter, &msgBuffer);
	ret = rsslEncodeRDMMsg(&eIter, (RsslRDMMsg*)&encRDMMsg, &msgBuffer.length, &error);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);


	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	pDecRDMMsg = &decRDMMsg.loginMsg.status;

	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);

	ASSERT_TRUE(pDecRDMMsg->flags == 0);


	/* Test set 2: Ensure that Decode properly errors out when blank types are present */
	/* AuthenticationErrorText */
	setupEncodeIterator(&eIter, &msgBuffer);
	rsslClearStatusMsg((RsslStatusMsg*)&encMsg);
	encMsg.msgBase.msgClass = RSSL_MC_STATUS;
	encMsg.msgBase.streamId = -5;
	encMsg.msgBase.domainType = RSSL_DMT_LOGIN;
	encMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	encMsg.statusMsg.flags = RSSL_STMF_HAS_MSG_KEY | RSSL_STMF_HAS_STATE;

	encMsg.statusMsg.state.dataState = RSSL_DATA_OK;
	encMsg.statusMsg.state.streamState = RSSL_STREAM_OPEN;
	/* Populate key with userName */
	encMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME;

	encMsg.msgBase.msgKey.name = userName;

	encMsg.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;

	/* Begin encoding message */
	ret = rsslEncodeMsgInit(&eIter, &encMsg, 0);

	ASSERT_TRUE(ret == RSSL_RET_ENCODE_MSG_KEY_OPAQUE);

	/*** Encode login attributes in msgKey.attrib ***/

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	ret = rsslEncodeElementListInit(&eIter, &elementList, 0, 0);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	rsslClearElementEntry(&elementEntry);

	elementEntry.dataType = RSSL_DT_ASCII_STRING;
	elementEntry.name = RSSL_ENAME_AUTHN_ERROR_TEXT;
	ret = rsslEncodeElementEntry(&eIter, &elementEntry, &blankBuffer);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	ret = rsslEncodeElementListComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	/* complete encode key */
	/* rsslEncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
	for us to encode our container/msg payload */
	ret = rsslEncodeMsgKeyAttribComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);


	ret = rsslEncodeMsgComplete(&eIter, RSSL_TRUE);
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);

	msgBuffer.length = rsslGetEncodedBufferLength(&eIter);

	setupDecodeIterator(&dIter, &msgBuffer, &memoryBuffer);
	rsslClearRDMMsg(&decRDMMsg);
	ASSERT_TRUE(rsslDecodeRDMMsg(&dIter, &rsslMsg, &decRDMMsg, &memoryBuffer, &error) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(decRDMMsg.loginMsg.status.flags |= RDM_LG_STF_HAS_AUTHN_ERROR_TEXT && decRDMMsg.loginMsg.status.authenticationErrorText.length == 0);

}

void loginRTTMsgTests()
{
	RsslRDMLoginRTT encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMLoginRTT *pDecRDMMsg;
	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_LG_RTT_NONE,
		RDM_LG_RTT_HAS_TCP_RETRANS,
		RDM_LG_RTT_HAS_LATENCY
	};

	RsslUInt32 *flagsList, flagsListCount;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt ticks = 135000000012;
	RsslUInt tcpRetrans = 150;
	RsslUInt latency = 1400006;


	clearTypedMessageStats(&stats);

	/* Request */
	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase) / sizeof(RsslUInt32), RSSL_FALSE);

	for (i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslRet ret;

			testWriteAction = testWriteActions[j];

			/*** Encode ***/
			rsslClearRDMLoginRTT(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_LOGIN);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_LG_MT_RTT);

			encRDMMsg.flags = flagsList[i];

			encRDMMsg.rdmMsgBase.streamId = streamId;

			encRDMMsg.ticks = ticks;

			/* Set parameters based on flags */
			if (encRDMMsg.flags & RDM_LG_RTT_HAS_LATENCY)
			{
				encRDMMsg.lastLatency = latency;
			}

			if (encRDMMsg.flags & RDM_LG_RTT_HAS_TCP_RETRANS)
			{
				encRDMMsg.tcpRetrans = tcpRetrans;
			}

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
					&rsslMsg, &decRDMMsg,
					&stats);
				pDecRDMMsg = &decRDMMsg.loginMsg.RTT;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMLoginRTT*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_RTT);

			ASSERT_EQ(pDecRDMMsg->flags, encRDMMsg.flags);

			/* Check parameters */
			ASSERT_EQ(pDecRDMMsg->ticks, ticks);
			
			if (pDecRDMMsg->flags & RDM_LG_RTT_HAS_LATENCY)
			{
				ASSERT_EQ(pDecRDMMsg->lastLatency, latency);
			}

			if (pDecRDMMsg->flags & RDM_LG_RTT_HAS_TCP_RETRANS)
			{
				ASSERT_EQ(pDecRDMMsg->tcpRetrans, tcpRetrans);
			}

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);
		}
	}

	free(flagsList);

	////printTypedMessageStats(&stats);
}

