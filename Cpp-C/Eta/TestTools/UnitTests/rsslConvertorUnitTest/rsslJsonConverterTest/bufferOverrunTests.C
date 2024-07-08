/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 LSEG. All rights reserved.      –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"

using namespace std;
using namespace json;

/* Fixture for BufferOverrunTests that has conversion code. */
class BufferOverrunTests : public MsgConversionTestBase
{
};

TEST_F(BufferOverrunTests, DictionaryTest)
{
	RsslRefreshMsg refreshMsg;
	
	const RsslUInt32 TOTAL_COUNT_HINT = 5;
	const RsslUInt MAP_ENTRY_KEY = 6;
	const RsslFieldId KEY_FIELD_ID = UINT_FIELD.fieldId;

	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.streamId = 5;
	refreshMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	refreshMsg.msgBase.containerType = RSSL_DT_SERIES;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.text = STATE_TEXT;

	rsslRefreshMsgApplyHasMsgKey(&refreshMsg);
	rsslMsgKeyApplyHasName(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.name = MSG_KEY_NAME;
	rsslMsgKeyApplyHasServiceId(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&refreshMsg, 0));

	// Encode Dictionary
	RsslInt32 currentFid = getRsslDataDictionary()->minFid;
	
	ASSERT_EQ(RSSL_RET_DICT_PART_ENCODED, rsslEncodeFieldDictionary(&_eIter, getRsslDataDictionary(), &currentFid, RDM_DICTIONARY_MINIMAL, 0));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));
	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());
	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Refresh", _jsonDocument["Type"].GetString());

	/* Check Dictionary. */
	ASSERT_TRUE(_jsonDocument.HasMember("Domain"));
	EXPECT_STREQ("Dictionary", _jsonDocument["Domain"].GetString());

	ASSERT_TRUE(_jsonDocument.HasMember("Series"));

}



