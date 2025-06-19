/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "testFramework.h"
#include "rsslVATestUtil.h"
#include "gtest/gtest.h"
#include "rtr/rsslVAUtils.h"

static TypedMessageStats stats;
static TestWriteAction testWriteAction;

void dictionaryRequestMsgTests();
void dictionaryCloseMsgTests();
void dictionaryRefreshMsgTests();
void dictionaryStatusMsgTests();

TEST(DictionaryMsgTest, RequestMsgTests)
{
	dictionaryRequestMsgTests();
}

TEST(DictionaryMsgTest, CloseMsgTests)
{
	dictionaryCloseMsgTests();
}

TEST(DictionaryMsgTest, RefreshMsgTests)
{
	dictionaryRefreshMsgTests();
}

TEST(DictionaryMsgTest, StatusMsgTests)
{
	dictionaryStatusMsgTests();
}

void dictionaryRequestMsgTests()
{
	RsslRDMDictionaryRequest encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDictionaryRequest *pDecRDMMsg;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt16 serviceId = 273;
	RsslUInt32 verbosity = RDM_DICTIONARY_VERBOSE;
	RsslBuffer dictionaryName = rssl_init_buffer_from_string(const_cast<char*>("RWFFld"));

	RsslUInt32 flagsBase[] =
	{
		RDM_DC_RQF_STREAMING
	};
	RsslUInt32 *flagsList, flagsListCount;

	RsslUInt32 i, j;

	clearTypedMessageStats(&stats);

	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	for(i = 0; i < flagsListCount; ++i)
	{
		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslRet ret;

			testWriteAction = testWriteActions[j];


			/*** Encode ***/
			rsslClearRDMDictionaryRequest(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DC_MT_REQUEST);

			encRDMMsg.rdmMsgBase.streamId = streamId;

			encRDMMsg.flags = flagsList[i];
			encRDMMsg.serviceId = serviceId;
			encRDMMsg.verbosity = verbosity;
			encRDMMsg.dictionaryName = dictionaryName;

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.dictionaryMsg.request;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMDictionaryRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DC_MT_REQUEST);

			ASSERT_TRUE(pDecRDMMsg->flags == flagsList[i]);
			ASSERT_TRUE(pDecRDMMsg->serviceId == serviceId);
			ASSERT_TRUE(pDecRDMMsg->verbosity == verbosity);
			ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->dictionaryName, &dictionaryName));
			ASSERT_TRUE(pDecRDMMsg->dictionaryName.data != dictionaryName.data); /* deep-copy check */

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);
		}
	}

	free(flagsList);
	//printTypedMessageStats(&stats);
}

void dictionaryCloseMsgTests()
{
	RsslRDMDictionaryClose encRDMMsg, *pDecRDMMsg;

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
		rsslClearRDMDictionaryClose(&encRDMMsg);
		ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
		ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DC_MT_CLOSE);

		encRDMMsg.rdmMsgBase.streamId = streamId;

		if (testWriteAction != TEST_EACTION_CREATE_COPY)
		{
			writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
					&rsslMsg, &decRDMMsg,
					&stats);
			pDecRDMMsg = &decRDMMsg.dictionaryMsg.close;
		}
		else
			ASSERT_TRUE((pDecRDMMsg = (RsslRDMDictionaryClose*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DC_MT_CLOSE);

		if (testWriteAction == TEST_EACTION_CREATE_COPY)
			free(pDecRDMMsg);
	}

	//printTypedMessageStats(&stats);
}

void dictionaryRefreshMsgTests()
{
	char enumDictionaryText[] =
		"!tag Filename    ENUMTYPE.001\n"
		"!tag Desc        IDN Marketstream enumerated tables\n"
		"!tag RT_Version  4.00\n"
		"!tag DT_Version  12.11\n"
		"!tag Date        13-Aug-2010\n"
		"PRCTCK_1      14\n"
		"      0          \" \"   no tick\n"
		"      1         #DE#   up tick or zero uptick\n"
		"      2         #FE#   down tick or zero downtick\n"
		"      3          \" \"   unchanged tick\n";

	RsslDataDictionary encDictionary, decDictionary;

	RsslRDMDictionaryRefresh encRDMMsg;
	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDictionaryRefresh *pDecRDMMsg;

	RsslInt32 streamId = -5;
	RsslUInt32 seqNum = 11152011;
	RsslState state = { RSSL_STREAM_OPEN, RSSL_DATA_SUSPECT, RSSL_SC_FAILOVER_COMPLETED, rssl_init_buffer_from_string(const_cast<char*>("In Soviet Russia, state tests you!")) };

	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_DC_RFF_SOLICITED,
		RDM_DC_RFF_HAS_SEQ_NUM,
		RDM_DC_RFF_CLEAR_CACHE
	};
	RsslUInt32 *flagsList, flagsListCount;

	RsslBuffer fieldDictName = rssl_init_buffer_from_string(const_cast<char*>("RWFFld"));
	RsslBuffer enumDictName = rssl_init_buffer_from_string(const_cast<char*>("RWFEnum"));
	RsslBuffer errorText = { 255, (char*)alloca(255) };

	clearTypedMessageStats(&stats);

	createFileFromString("tmpEnumDictionary.txt", enumDictionaryText, sizeof(enumDictionaryText));

	rsslClearDataDictionary(&encDictionary);
	ASSERT_TRUE(rsslLoadEnumTypeDictionary("tmpEnumDictionary.txt", &encDictionary, &errorText) == RSSL_RET_SUCCESS);

	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	clearTypedMessageStats(&stats);

	/* Test flags with enum dictionary */
	for(i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslDecodeIterator dIter;
			RsslRet ret;

			testWriteAction = testWriteActions[j];

			rsslClearRDMDictionaryRefresh(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DC_MT_REFRESH);

			encRDMMsg.flags = flagsList[i];
			encRDMMsg.rdmMsgBase.streamId = streamId;
			encRDMMsg.state = state;
			encRDMMsg.pDictionary = &encDictionary;
			encRDMMsg.dictionaryName = enumDictName;
			encRDMMsg.type = RDM_DICTIONARY_ENUM_TABLES;

			if (encRDMMsg.flags & RDM_DC_RFF_HAS_SEQ_NUM)
				encRDMMsg.sequenceNumber = seqNum;

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.dictionaryMsg.refresh;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMDictionaryRefresh*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DC_MT_REFRESH);


			switch(testWriteAction)
			{
			case TEST_EACTION_ENCODE: ASSERT_TRUE(pDecRDMMsg->flags == (flagsList[i] | RDM_DC_RFF_IS_COMPLETE | RDM_DC_RFF_HAS_INFO)); break;
			case TEST_EACTION_COPY: case TEST_EACTION_CREATE_COPY: ASSERT_TRUE(pDecRDMMsg->flags == flagsList[i]); break;
			default: abort();
			}

			if (pDecRDMMsg->flags & RDM_DC_RFF_HAS_SEQ_NUM)
				ASSERT_TRUE(pDecRDMMsg->sequenceNumber == seqNum);
				
			ASSERT_TRUE(pDecRDMMsg->type == RDM_DICTIONARY_ENUM_TABLES);
			ASSERT_TRUE(pDecRDMMsg->state.streamState == state.streamState);
			ASSERT_TRUE(pDecRDMMsg->state.dataState == state.dataState);
			ASSERT_TRUE(pDecRDMMsg->state.code == state.code);
			ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->dictionaryName, &enumDictName));
			ASSERT_TRUE(pDecRDMMsg->dictionaryName.data != enumDictName.data); /* deep-copy check */
			ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->state.text, &state.text));
			ASSERT_TRUE(pDecRDMMsg->state.text.data != state.text.data); /* deep-copy check */

			if (testWriteAction == TEST_EACTION_ENCODE)
			{
				/* Message was encoded and decoded. Try to decode the payload. */
				rsslClearDecodeIterator(&dIter);
				rsslSetDecodeIteratorBuffer(&dIter, &pDecRDMMsg->dataBody);
				rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

				rsslClearDataDictionary(&decDictionary);
				ASSERT_TRUE(rsslDecodeEnumTypeDictionary(&dIter, &decDictionary, RDM_DICTIONARY_VERBOSE, &errorText) == RSSL_RET_SUCCESS);
				ASSERT_TRUE(rsslDeleteDataDictionary(&decDictionary) == RSSL_RET_SUCCESS);
			}

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);

		}
	}


	/* TODO Testing multipart refreshes would be nice, however for now this is already covered by the provider example. */

	rsslDeleteDataDictionary(&encDictionary);
	free(flagsList);

	deleteFile("tmpEnumDictionary.txt");

	//printTypedMessageStats(&stats);
}

void dictionaryStatusMsgTests()
{
	RsslRDMDictionaryStatus encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDictionaryStatus *pDecRDMMsg;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslState state = { RSSL_STREAM_OPEN, RSSL_DATA_SUSPECT, RSSL_SC_FAILOVER_COMPLETED, rssl_init_buffer_from_string(const_cast<char*>("In Soviet Russia, state tests you!")) };

	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_DC_STF_HAS_STATE
	};
	RsslUInt32 *flagsList, flagsListCount;

	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	clearTypedMessageStats(&stats);

	for(i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslRet ret;

			testWriteAction = testWriteActions[j];


			rsslClearRDMDictionaryStatus(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DC_MT_STATUS);

			encRDMMsg.flags = flagsList[i];
			encRDMMsg.rdmMsgBase.streamId = streamId;

			if (flagsList[i] & RDM_DC_STF_HAS_STATE)
				encRDMMsg.state = state;

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.dictionaryMsg.status;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMDictionaryStatus*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_DICTIONARY);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DC_MT_STATUS);

			ASSERT_TRUE(pDecRDMMsg->flags == flagsList[i]);

			if (flagsList[i] & RDM_DC_STF_HAS_STATE)
			{
				ASSERT_TRUE(pDecRDMMsg->state.streamState == state.streamState);
				ASSERT_TRUE(pDecRDMMsg->state.dataState == state.dataState);
				ASSERT_TRUE(pDecRDMMsg->state.code == state.code);
				ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->state.text, &state.text));
				ASSERT_TRUE(pDecRDMMsg->state.text.data != state.text.data); /* deep-copy check */
			}

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);
		}
	}

	//printTypedMessageStats(&stats);

	free(flagsList);
}


