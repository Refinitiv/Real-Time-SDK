/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 Refinitiv. All rights reserved. –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"

/* Fixture for ErrorMsgTests that has conversion code. */
class ErrorMsgTests : public MsgConversionTestBase
{
};

/* Parameters for ErrorMsg tests. */
class ErrorMsgTestParams
{
	public:

	/* Boolean parameters for testing optional members. */
	jsonToRwfBase::errorCodes	errorCode;
	char customJsonMessage[256];
	jsmntype_t firstTokenType;
	jsmntype_t secondTokenType;
	const RsslBuffer *errorParentKey;
	char * unexpectedString;
	const RsslBuffer *missingKey;
	int expectedId;
	bool setJson;

	ErrorMsgTestParams(jsonToRwfBase::errorCodes errorCode, jsmntype_t firstTokenType, jsmntype_t secondTokenType, int expectedId, char *customJsonMessage, const RsslBuffer *errorParentKey = 0, bool setJson = false)
	{
		this->errorCode = errorCode;
		this->expectedId = expectedId;
		(void)strcpy(this->customJsonMessage, customJsonMessage);
		this->firstTokenType = firstTokenType;
		this->secondTokenType = secondTokenType;
		this->errorParentKey = errorParentKey;
		this->setJson = 0 < strlen(customJsonMessage) ? true : false;
	}

	ErrorMsgTestParams(jsonToRwfBase::errorCodes errorCode, char * unexpectedString, int expectedId, char *customJsonMessage, const RsslBuffer *errorParentKey = 0, bool setJson = false)
	{
		this->errorCode = errorCode;
		this->unexpectedString = unexpectedString;
		this->expectedId = expectedId;
		(void)strcpy(this->customJsonMessage, customJsonMessage);
		this->errorParentKey = errorParentKey;
		this->setJson = 0 < strlen(customJsonMessage) ? true : false;
	}

	ErrorMsgTestParams(jsonToRwfBase::errorCodes errorCode, const RsslBuffer *missingKey, int expectedId, char *customJsonMessage, const RsslBuffer *errorParentKey = 0, bool setJson = false)
	{
		this->errorCode = errorCode;
		this->missingKey = missingKey;
		this->expectedId = expectedId;
		(void)strcpy(this->customJsonMessage, customJsonMessage);
		this->errorParentKey = errorParentKey;
		this->setJson = 0 < strlen(customJsonMessage) ? true : false;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const ErrorMsgTestParams& params)
	{
		out << "["
			"errorCode:" << params.errorCode << ","
			"customJsonMessage:" << params.customJsonMessage
			//"firstTokenType:" << params.firstTokenType << ","
			//"secondTokenType:" << params.secondTokenType << ","
			//"errorParentKey:" << (params.errorParentKey ? params.errorParentKey->data : "null")  << ","
			//"unexpectedString:" << params.unexpectedString << ","
			//"missingKey:" << (params.missingKey ? params.missingKey->data : "null") << ","
			//"expectedId:" << (params.expectedId ? params.expectedId
			<< "]";
		return out;
	}
};

class ErrorMsgParamFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<ErrorMsgTestParams>
{
};

/* Test that fires the specified Error and verifies the resulting Error message */
TEST_P(ErrorMsgParamFixture, ErrorMsgParamTest)
{
	ErrorMsgTestParams const &params = GetParam();

	_jsonDocument.SetObject();

	if(params.setJson)
	{
		_jsonBuffer.data = (char*)params.customJsonMessage;
		_jsonBuffer.length = (rtrUInt32)strlen(params.customJsonMessage);
	}

	/* Check Error */
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	/* Check message type. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());

	if(params.errorCode == jsonToRwfBase::INVALID_TOKEN_TYPE)
	{
		/* Check Stream ID. */
		ASSERT_TRUE(_jsonDocument.HasMember("ID"));
		ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
		EXPECT_EQ(params.expectedId, _jsonDocument["ID"].GetInt());


		/* Check Text. */
		ASSERT_TRUE(_jsonDocument.HasMember("Text"));
		ASSERT_TRUE(_jsonDocument["Text"].IsString());

		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Converter Token Type error: "));
		switch (params.firstTokenType)
		{
			case JSMN_PRIMITIVE :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "Expected 'PRIMITIVE'"));
				break;
			case JSMN_OBJECT :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "Expected 'OBJECT'"));
				break;
			case JSMN_ARRAY :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "Expected 'ARRAY'"));
				break;
			case JSMN_STRING :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "Expected 'STRING'"));
				break;
		}
		if (params.errorParentKey)
		{
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.errorParentKey->data));
		}
		switch (params.secondTokenType)
		{
			case JSMN_PRIMITIVE :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " Received 'PRIMITIVE'"));
				break;
			case JSMN_OBJECT :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " Received 'OBJECT'"));
				break;
			case JSMN_ARRAY :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " Received 'ARRAY'"));
				break;
			case JSMN_STRING :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " Received 'STRING'"));
				break;
		}

		/* Check Debug. */
		ASSERT_TRUE(_jsonDocument.HasMember("Debug"));
		ASSERT_TRUE(_jsonDocument["Debug"].IsObject());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("File"));
		ASSERT_TRUE(_jsonDocument["Debug"]["File"].IsString());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Line"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Line"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Offset"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Offset"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Message"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Message"].IsString());
	}

	if(params.errorCode == jsonToRwfBase::UNEXPECTED_VALUE)
	{
		/* Check Stream ID. */
		ASSERT_TRUE(_jsonDocument.HasMember("ID"));
		ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
		EXPECT_EQ(params.expectedId, _jsonDocument["ID"].GetInt());


		/* Check Text. */
		ASSERT_TRUE(_jsonDocument.HasMember("Text"));
		ASSERT_TRUE(_jsonDocument["Text"].IsString());

		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Unexpected Value. Received "));
		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.unexpectedString));
		if(params.errorParentKey)
		{
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " for key "));
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.errorParentKey->data));
		}

		/* Check Debug. */
		ASSERT_TRUE(_jsonDocument.HasMember("Debug"));
		ASSERT_TRUE(_jsonDocument["Debug"].IsObject());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("File"));
		ASSERT_TRUE(_jsonDocument["Debug"]["File"].IsString());
		EXPECT_STREQ("Converter/jsonToRwfSimple.C", _jsonDocument["Debug"]["File"].GetString());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Line"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Line"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Offset"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Offset"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Message"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Message"].IsString());
	}
	else if(params.errorCode == jsonToRwfBase::MISSING_KEY)
	{
		/* Check Stream ID. */
		ASSERT_TRUE(_jsonDocument.HasMember("ID"));
		ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
		EXPECT_EQ(params.expectedId, _jsonDocument["ID"].GetInt());


		/* Check Text. */
		ASSERT_TRUE(_jsonDocument.HasMember("Text"));
		ASSERT_TRUE(_jsonDocument["Text"].IsString());

		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key "));
		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.missingKey->data));

		if(params.errorParentKey)
		{
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " for "));
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.errorParentKey->data));
		}

		/* Check Debug. */
		ASSERT_TRUE(_jsonDocument.HasMember("Debug"));
		ASSERT_TRUE(_jsonDocument["Debug"].IsObject());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("File"));
		ASSERT_TRUE(_jsonDocument["Debug"]["File"].IsString());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Line"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Line"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Message"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Message"].IsString());
	}
	else if(params.errorCode == jsonToRwfBase::UNEXPECTED_KEY)
	{
		/* Check Stream ID. */
		ASSERT_TRUE(_jsonDocument.HasMember("ID"));
		ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
		EXPECT_EQ(params.expectedId, _jsonDocument["ID"].GetInt());


		/* Check Text. */
		ASSERT_TRUE(_jsonDocument.HasMember("Text"));
		ASSERT_TRUE(_jsonDocument["Text"].IsString());

		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Unexpected Key. Received "));
		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.unexpectedString));

		if(params.errorParentKey)
		{
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " for key "));
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.errorParentKey->data));
		}

		/* Check Debug. */
		ASSERT_TRUE(_jsonDocument.HasMember("Debug"));
		ASSERT_TRUE(_jsonDocument["Debug"].IsObject());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("File"));
		ASSERT_TRUE(_jsonDocument["Debug"]["File"].IsString());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Line"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Line"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Offset"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Offset"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Message"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Message"].IsString());
	}
	else if(params.errorCode == jsonToRwfBase::TYPE_MISMATCH)
	{
		/* Check Stream ID. */
		ASSERT_TRUE(_jsonDocument.HasMember("ID"));
		ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
		EXPECT_EQ(params.expectedId, _jsonDocument["ID"].GetInt());


		/* Check Text. */
		ASSERT_TRUE(_jsonDocument.HasMember("Text"));
		ASSERT_TRUE(_jsonDocument["Text"].IsString());

		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Mixed Types in OMM Array: "));
		switch (params.firstTokenType)
		{
			case JSMN_PRIMITIVE :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "Received 'PRIMITIVE'"));
				break;
			case JSMN_OBJECT :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "Received 'OBJECT'"));
				break;
			case JSMN_ARRAY :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "Received 'ARRAY'"));
				break;
			case JSMN_STRING :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "Received 'STRING'"));
				break;
		}
		if (params.errorParentKey)
		{
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.errorParentKey->data));
		}
		switch (params.secondTokenType)
		{
			case JSMN_PRIMITIVE :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " and 'PRIMITIVE'"));
				break;
			case JSMN_OBJECT :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " and 'OBJECT'"));
				break;
			case JSMN_ARRAY :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " and 'ARRAY'"));
				break;
			case JSMN_STRING :
				ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " and 'STRING'"));
				break;
		}

		if(params.errorParentKey)
		{
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " for key "));
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.errorParentKey->data));
		}

		/* Check Debug. */
		ASSERT_TRUE(_jsonDocument.HasMember("Debug"));
		ASSERT_TRUE(_jsonDocument["Debug"].IsObject());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("File"));
		ASSERT_TRUE(_jsonDocument["Debug"]["File"].IsString());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Line"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Line"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Offset"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Offset"].IsNumber());
		ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Message"));
		ASSERT_TRUE(_jsonDocument["Debug"]["Message"].IsString());
	}
	else if(params.errorCode == jsonToRwfBase::UNEXPECTED_FID)
	{
	  /* Check Stream ID. */
	  ASSERT_TRUE(_jsonDocument.HasMember("ID"));
	  ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
	  EXPECT_EQ(params.expectedId, _jsonDocument["ID"].GetInt());


	  /* Check Text. */
	  ASSERT_TRUE(_jsonDocument.HasMember("Text"));
	  ASSERT_TRUE(_jsonDocument["Text"].IsString());

		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Unexpected FID. Received "));
		ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.unexpectedString));

		if(params.errorParentKey)
		{
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), " for key "));
			ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), params.errorParentKey->data));
		}

	  /* Check Debug. */
	  ASSERT_TRUE(_jsonDocument.HasMember("Debug"));
	  ASSERT_TRUE(_jsonDocument["Debug"].IsObject());
	  ASSERT_TRUE(_jsonDocument["Debug"].HasMember("File"));
	  ASSERT_TRUE(_jsonDocument["Debug"]["File"].IsString());
	  ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Line"));
	  ASSERT_TRUE(_jsonDocument["Debug"]["Line"].IsNumber());
	  ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Offset"));
	  ASSERT_TRUE(_jsonDocument["Debug"]["Offset"].IsNumber());
	  ASSERT_TRUE(_jsonDocument["Debug"].HasMember("Message"));
	  ASSERT_TRUE(_jsonDocument["Debug"]["Message"].IsString());
	}
}



INSTANTIATE_TEST_CASE_P(ErrorTestsBaseMessage, ErrorMsgParamFixture, ::testing::Values(
	/** BASE MESSAGE TESTS **/

	/* ID with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 0, (char *)"{\"ID\":\"2\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ID),
	/* ID with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 0, (char *)"{\"ID\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ID),
	/* Type with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_TYPE),
	/* Type with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_TYPE),
	/* Domain with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Domain\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_DOMAIN),
	/* Domain with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Domain\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_DOMAIN)

	//ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_VALUE, (char *)"Electric/Flying", 2, (char *)"{\"ID\":2,\"Type\":\"Electric/Flying\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_TYPE),
	//ErrorMsgTestParams(jsonToRwfBase::MISSING_KEY, &JSON_ID, 0, (char *)"{\"Key\":{\"Name\":\"TRI.N\"}}"),
	//ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char *)"PowerLevel", 2, (char *)"{\"ID\":2,\"PowerLevel\":9001,\"Key\":{\"Name\":\"TRI.N\"}}"),
	//ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_FID, (char *)"BID_CUSTOM", 3, (char *)"{\"Type\": \"Post\",\"Message\": { \"Type\": \"Update\",\"Fields\": {\"ASKSIZE\": 19,\"ASK\": 0,\"BIDSIZE\": 18,\"BID_CUSTOM\": 45.55},\"ID\": 0,     \"Domain\": \"MarketPrice\",     \"Key\":{\"Service\":60000,\"Name\":\"TRI.N\"}},\"Ack\":true,\"PostUserInfo\":{\"Address\":806749471,\"UserID\":1},\"ID\": 3,\"Domain\":\"MarketPrice\",\"Key\": {\"Service\":257,\"Name\":\"TRI.N\"},\"PostID\":2}"),
	//ErrorMsgTestParams(jsonToRwfBase::TYPE_MISMATCH, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{   \"ID\": 2,   \"Key\": {     \"Name\": \"TRI.N\"   },   \"Map\": {     \"Entries\": [       {         \"Action\": \"Add\",         \"Fields\": {           \"ORDER_PRC\": 326.3,           \"ORDER_SIDE\": 1,           \"ORDER_SIZE\": 100,           \"QUOTIM_MS\": 78398067         },         \"Key\": {           \"Type\": \"Int\",           \"Length\": 3,           \"Data\": [             1,             2,             \"3\"           ]         }       }     ],     \"KeyType\": \"Array\"   } }", &JSON_DATA)

));

INSTANTIATE_TEST_CASE_P(ErrorTestsRequestMessage, ErrorMsgParamFixture, ::testing::Values(
	/** REQUEST MESSAGE TESTS **/

	/* RequestMsg ConfInfoInUpdates with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"ConfInfoInUpdates\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_CONFINFOINUPDATES),
	/* RequestMsg ConfInfoInUpdates with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"ConfInfoInUpdates\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_CONFINFOINUPDATES),
	/* RequestMsg ConfInfoInUpdates with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"ConfInfoInUpdates\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_CONFINFOINUPDATES),
	/* RequestMsg ExtendedHeader with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"ExtHdr\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_EXTHDR),
	/* RequestMsg ExtendedHeader with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"ExtHdr\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_EXTHDR),
	/* RequestMsg ExtendedHeader with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"ExtHdr\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_EXTHDR),
	/* RequestMsg KeyInUpdates with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"KeyInUpdates\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_KEYINUPDATES),
	/* RequestMsg KeyInUpdates with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"KeyInUpdates\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_KEYINUPDATES),
	/* RequestMsg KeyInUpdates with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"KeyInUpdates\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_KEYINUPDATES),
	/* RequestMsg View with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"View\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_VIEW),
	/* RequestMsg View with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"View\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_VIEW),
	/* RequestMsg View with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_STRING, 2, (char *)"{\"ID\":2,\"View\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_VIEW),
	/* RequestMsg Streaming with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Streaming\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_STREAMING),
	/* RequestMsg Streaming with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Streaming\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_STREAMING),
	/* RequestMsg Streaming with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Streaming\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_STREAMING),
	/* RequestMsg Refresh with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Refresh\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_REFRESH),
	/* RequestMsg Refresh with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Refresh\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_REFRESH),
	/* RequestMsg Refresh with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Refresh\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_REFRESH),
	/* RequestMsg Private with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Private\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PRIVATE),
	/* RequestMsg Private with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Private\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PRIVATE),
	/* RequestMsg Private with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Private\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PRIVATE),
	/* RequestMsg Pause with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Pause\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PAUSE),
	/* RequestMsg Pause with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Pause\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PAUSE),
	/* RequestMsg Pause with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Pause\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PAUSE),
	/* RequestMsg Priority with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Priority\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PRIORITY),
	/* RequestMsg Priority with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Priority\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PRIORITY),
	/* RequestMsg Priority with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Priority\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PRIORITY),
	/* RequestMsg Priority Count with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Priority\":{\"Count\":{\"Test\":1},\"Class\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNT),
	/* RequestMsg Priority Count with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Priority\":{\"Count\":[\"Test\"],\"Class\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNT),
	/* RequestMsg Priority Count with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Priority\":{\"Count\":\"Test\",\"Class\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNT),
	/* RequestMsg Priority Class with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Priority\":{\"Class\":{\"Test\":1},\"Count\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_CLASS),
	/* RequestMsg Priority Class with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Priority\":{\"Class\":[\"Test\"],\"Count\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_CLASS),
	/* RequestMsg Priority Class with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Priority\":{\"Class\":\"Test\",\"Count\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_CLASS),
	/* RequestMsg Qos with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Qos\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_QOS),
	/* RequestMsg Qos with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Qos\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_QOS),
	/* RequestMsg Qos with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Qos\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_QOS),
	/* RequestMsg Qualified with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Qualified\":{\"Test\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_QUALIFIED),
	/* RequestMsg Qualified with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Qualified\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_QUALIFIED),
	/* RequestMsg Qualified with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Qualified\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_QUALIFIED),
	/* RequestMsg WorstQos with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"WorstQos\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_WORSTQOS),
	/* RequestMsg WorstQos with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"WorstQos\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_WORSTQOS),
	/* RequestMsg WorstQos with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"WorstQos\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_WORSTQOS)

));

INSTANTIATE_TEST_CASE_P(ErrorTestsRefreshMessage, ErrorMsgParamFixture, ::testing::Values(
	/** REFRESH MESSAGE TESTS **/

	/* RefreshMsg DoNotCache with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"DoNotCache\":{\"Test\":1}}", &JSON_DONOTCACHE),
	/* RefreshMsg DoNotCache with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"DoNotCache\":[\"Test\"]}", &JSON_DONOTCACHE),
	/* RefreshMsg DoNotCache with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"DoNotCache\":\"Test\"}", &JSON_DONOTCACHE),
	/* RefreshMsg Key with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Key\":1}", &JSON_KEY),
	/* RefreshMsg Key with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Refresh\",\"Key\":[\"Test\"]}", &JSON_KEY),
	/* RefreshMsg Key with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Key\":\"Test\"}", &JSON_KEY),
	/* RefreshMsg ExtendedHeader with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"ExtHdr\":{\"Test\":1}}", &JSON_EXTHDR),
	/* RefreshMsg ExtendedHeader with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"ExtHdr\":[\"Test\"]}", &JSON_EXTHDR),
	/* RefreshMsg ExtendedHeader with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"ExtHdr\":1}", &JSON_EXTHDR),
	/* RefreshMsg PostUserInfo with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"PostUserInfo\":1}", &JSON_POSTUSERINFO),
	/* RefreshMsg PostUserInfo with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Refresh\",\"PostUserInfo\":[\"Test\"]}", &JSON_POSTUSERINFO),
	/* RefreshMsg PostUserInfo with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"PostUserInfo\":\"Test\"}", &JSON_POSTUSERINFO),
	/* RefreshMsg PermData with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"PermData\":{\"Test\":1}}", &JSON_PERMDATA),
	/* RefreshMsg PermData with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"PermData\":[\"Test\"]}", &JSON_PERMDATA),
	/* RefreshMsg PermData with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"PermData\":1}", &JSON_PERMDATA),
	/* RefreshMsg Private with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Private\":{\"Test\":1}}", &JSON_PRIVATE),
	/* RefreshMsg Private with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Private\":[\"Test\"]}", &JSON_PRIVATE),
	/* RefreshMsg Private with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Private\":\"Test\"}", &JSON_PRIVATE),
	/* RefreshMsg State with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"State\":1}", &JSON_STATE),
	/* RefreshMsg State with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Refresh\",\"State\":[\"Test\"]}", &JSON_STATE),
	/* RefreshMsg State with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"State\":\"Test\"}", &JSON_STATE),
	/* RefreshMsg SeqNumber with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"SeqNumber\":{\"Test\":1}}", &JSON_SEQNUM),
	/* RefreshMsg SeqNumber with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"SeqNumber\":[\"Test\"]}", &JSON_SEQNUM),
	/* RefreshMsg SeqNumber with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"SeqNumber\":\"Test\"}", &JSON_SEQNUM),
	/* RefreshMsg Solicited with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Solicited\":{\"Test\":1}}", &JSON_SOLICITED),
	/* RefreshMsg Solicited with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Solicited\":[\"Test\"]}", &JSON_SOLICITED),
	/* RefreshMsg Solicited with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Solicited\":\"Test\"}", &JSON_SOLICITED),
	/* RefreshMsg Complete with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Complete\":{\"Test\":1}}", &JSON_COMPLETE),
	/* RefreshMsg Complete with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Complete\":[\"Test\"]}", &JSON_COMPLETE),
	/* RefreshMsg Complete with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Complete\":\"Test\"}", &JSON_COMPLETE),
	/* RefreshMsg ClearCache with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"ClearCache\":{\"Test\":1}}", &JSON_CLEARCACHE),
	/* RefreshMsg ClearCache with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"ClearCache\":[\"Test\"]}", &JSON_CLEARCACHE),
	/* RefreshMsg ClearCache with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"ClearCache\":\"Test\"}", &JSON_CLEARCACHE),
	/* RefreshMsg Qos with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Qos\":1}", &JSON_QOS),
	/* RefreshMsg Qos with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Refresh\",\"Qos\":[\"Test\"]}", &JSON_QOS),
	/* RefreshMsg Qos with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Qos\":\"Test\"}", &JSON_QOS),
	/* RefreshMsg Qualified with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Qualified\":{\"Test\":1}}", &JSON_QUALIFIED),
	/* RefreshMsg Qualified with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Qualified\":[\"Test\"]}", &JSON_QUALIFIED),
	/* RefreshMsg Qualified with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Qualified\":\"Test\"}", &JSON_QUALIFIED),
	/* RefreshMsg ReqKey with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"ReqKey\":1}", &JSON_REQKEY),
	/* RefreshMsg ReqKey with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Refresh\",\"ReqKey\":[\"Test\"]}", &JSON_REQKEY),
	/* RefreshMsg ReqKey with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"ReqKey\":\"Test\"}", &JSON_REQKEY)

));

INSTANTIATE_TEST_CASE_P(ErrorTestsStatusMessage, ErrorMsgParamFixture, ::testing::Values(
	/** STATUS MESSAGE TESTS **/

	/* StatusMsg Key with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Key\":1}", &JSON_KEY),
	/* StatusMsg Key with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Status\",\"Key\":[\"Test\"]}", &JSON_KEY),
	/* StatusMsg Key with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Key\":\"Test\"}", &JSON_KEY),
	/* StatusMsg ReqKey with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"ReqKey\":1}", &JSON_REQKEY),
	/* StatusMsg ReqKey with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Status\",\"ReqKey\":[\"Test\"]}", &JSON_REQKEY),
	/* StatusMsg ReqKey with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"ReqKey\":\"Test\"}", &JSON_REQKEY),
	/* StatusMsg ExtendedHeader with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"ExtHdr\":{\"Test\":1}}", &JSON_EXTHDR),
	/* StatusMsg ExtendedHeader with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"ExtHdr\":[\"Test\"]}", &JSON_EXTHDR),
	/* StatusMsg ExtendedHeader with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"ExtHdr\":1}", &JSON_EXTHDR),
	/* StatusMsg PostUserInfo with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"PostUserInfo\":1}", &JSON_POSTUSERINFO),
	/* StatusMsg PostUserInfo with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Status\",\"PostUserInfo\":[\"Test\"]}", &JSON_POSTUSERINFO),
	/* StatusMsg PostUserInfo with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"PostUserInfo\":\"Test\"}", &JSON_POSTUSERINFO),
	/* StatusMsg PermData with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"PermData\":{\"Test\":1}}", &JSON_PERMDATA),
	/* StatusMsg PermData with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"PermData\":[\"Test\"]}", &JSON_PERMDATA),
	/* StatusMsg PermData with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"PermData\":1}", &JSON_PERMDATA),
	/* StatusMsg Private with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Private\":{\"Test\":1}}", &JSON_PRIVATE),
	/* StatusMsg Private with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Private\":[\"Test\"]}", &JSON_PRIVATE),
	/* StatusMsg Private with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Private\":\"Test\"}", &JSON_PRIVATE),
	/* StatusMsg State with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"State\":1}", &JSON_STATE),
	/* StatusMsg State with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Status\",\"State\":[\"Test\"]}", &JSON_STATE),
	/* StatusMsg State with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"State\":\"Test\"}", &JSON_STATE),
	/* StatusMsg ClearCache with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"ClearCache\":{\"Test\":1}}", &JSON_CLEARCACHE),
	/* StatusMsg ClearCache with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"ClearCache\":[\"Test\"]}", &JSON_CLEARCACHE),
	/* StatusMsg ClearCache with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"ClearCache\":\"Test\"}", &JSON_CLEARCACHE),
	/* StatusMsg Qualified with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Qualified\":{\"Test\":1}}", &JSON_QUALIFIED),
	/* StatusMsg Qualified with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Qualified\":[\"Test\"]}", &JSON_QUALIFIED),
	/* StatusMsg Qualified with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Qualified\":\"Test\"}", &JSON_QUALIFIED)
));

INSTANTIATE_TEST_CASE_P(ErrorTestsUpdateMessage, ErrorMsgParamFixture, ::testing::Values(
	/** UPDATE MESSAGE TESTS **/

	/* UpdateMsg Key with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Key\":1}", &JSON_KEY),
	/* UpdateMsg Key with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Update\",\"Key\":[\"Test\"]}", &JSON_KEY),
	/* UpdateMsg Key with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Key\":\"Test\"}", &JSON_KEY),
	/* UpdateMsg ExtendedHeader with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"ExtHdr\":{\"Test\":1}}", &JSON_EXTHDR),
	/* UpdateMsg ExtendedHeader with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"ExtHdr\":[\"Test\"]}", &JSON_EXTHDR),
	/* UpdateMsg ExtendedHeader with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"ExtHdr\":1}", &JSON_EXTHDR),
	/* UpdateMsg PostUserInfo with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"PostUserInfo\":1}", &JSON_POSTUSERINFO),
	/* UpdateMsg PostUserInfo with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Update\",\"PostUserInfo\":[\"Test\"]}", &JSON_POSTUSERINFO),
	/* UpdateMsg PostUserInfo with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"PostUserInfo\":\"Test\"}", &JSON_POSTUSERINFO),
	/* UpdateMsg PermData with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"PermData\":{\"Test\":1}}", &JSON_PERMDATA),
	/* UpdateMsg PermData with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"PermData\":[\"Test\"]}", &JSON_PERMDATA),
	/* UpdateMsg PermData with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"PermData\":1}", &JSON_PERMDATA),
	/* UpdateMsg SeqNumber with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"SeqNumber\":{\"Test\":1}}", &JSON_SEQNUM),
	/* UpdateMsg SeqNumber with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"SeqNumber\":[\"Test\"]}", &JSON_SEQNUM),
	/* UpdateMsg SeqNumber with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"SeqNumber\":\"Test\"}", &JSON_SEQNUM),
	/* UpdateMsg ConflationInfo with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"ConflationInfo\":1}", &JSON_CONFINFO),
	/* UpdateMsg ConflationInfo with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Update\",\"ConflationInfo\":[\"Test\"]}", &JSON_CONFINFO),
	/* UpdateMsg ConflationInfo with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"ConflationInfo\":\"Test\"}", &JSON_CONFINFO),
	/* UpdateMsg DoNotConflate with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotConflate\":{\"Test\":1}}", &JSON_DONOTCONFLATE),
	/* UpdateMsg DoNotConflate with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotConflate\":[\"Test\"]}", &JSON_DONOTCONFLATE),
	/* UpdateMsg DoNotConflate with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotConflate\":\"Test\"}", &JSON_DONOTCONFLATE),
	/* UpdateMsg DoNotCache with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotCache\":{\"Test\":1}}", &JSON_DONOTCACHE),
	/* UpdateMsg DoNotCache with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotCache\":[\"Test\"]}", &JSON_DONOTCACHE),
	/* UpdateMsg DoNotCache with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotCache\":\"Test\"}", &JSON_DONOTCACHE),
	/* UpdateMsg DoNotRipple with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotRipple\":{\"Test\":1}}", &JSON_DONOTRIPPLE),
	/* UpdateMsg DoNotRipple with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotRipple\":[\"Test\"]}", &JSON_DONOTRIPPLE),
	/* UpdateMsg DoNotRipple with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"DoNotRipple\":\"Test\"}", &JSON_DONOTRIPPLE),
	/* UpdateMsg Discardable with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Discardable\":{\"Test\":1}}", &JSON_DISCARDABLE),
	/* UpdateMsg Discardable with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Discardable\":[\"Test\"]}", &JSON_DISCARDABLE),
	/* UpdateMsg Discardable with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Discardable\":\"Test\"}", &JSON_DISCARDABLE),
	/* UpdateMsg UpdateType with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"UpdateType\":{\"Test\":1}}", &JSON_UPDATETYPE),
	/* UpdateMsg UpdateType with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"UpdateType\":[\"Test\"]}", &JSON_UPDATETYPE)
));

INSTANTIATE_TEST_CASE_P(ErrorTestsCloseMessage, ErrorMsgParamFixture, ::testing::Values(
	/** CLOSE MESSAGE TESTS **/

	/* CloseMsg ExtendedHeader with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"ExtHdr\":{\"Test\":1}}", &JSON_EXTHDR),
	/* CloseMsg ExtendedHeader with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"ExtHdr\":[\"Test\"]}", &JSON_EXTHDR),
	/* CloseMsg ExtendedHeader with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"ExtHdr\":1}", &JSON_EXTHDR)

));

INSTANTIATE_TEST_CASE_P(ErrorTestsAckMessage, ErrorMsgParamFixture, ::testing::Values(
	/** ACK MESSAGE TESTS **/

	/* AckMsg Text with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Text\":{\"Test\":1},\"AckID\":1}", &JSON_TEXT),
	/* AckMsg Text with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Text\":[\"Test\"],\"AckID\":1}", &JSON_TEXT),
	/* AckMsg Text with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Text\":1,\"AckID\":1}", &JSON_TEXT),
	/* AckMsg Key with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Key\":1,\"AckID\":1}", &JSON_KEY),
	/* AckMsg Key with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Key\":[\"Test\"],\"AckID\":1}", &JSON_KEY),
	/* AckMsg Key with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Key\":\"Test\",\"AckID\":1}", &JSON_KEY),
	/* AckMsg SeqNumber with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"SeqNumber\":{\"Test\":1},\"AckID\":1}", &JSON_SEQNUM),
	/* AckMsg SeqNumber with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"SeqNumber\":[\"Test\"],\"AckID\":1}", &JSON_SEQNUM),
	/* AckMsg SeqNumber with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"SeqNumber\":\"Test\",\"AckID\":1}", &JSON_SEQNUM),
	/* AckMsg AckID with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"AckID\":{\"Test\":1},\"AckID\":1}", &JSON_ACKID),
	/* AckMsg AckID with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"AckID\":[\"Test\"],\"AckID\":1}", &JSON_ACKID),
	/* AckMsg AckID with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"AckID\":\"Test\",\"AckID\":1}", &JSON_ACKID),
	/* AckMsg Private with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Private\":{\"Test\":1},\"AckID\":1}", &JSON_PRIVATE),
	/* AckMsg Private with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Private\":[\"Test\"],\"AckID\":1}", &JSON_PRIVATE),
	/* AckMsg Private with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Private\":\"Test\",\"AckID\":1}", &JSON_PRIVATE),
	/* AckMsg Qualified with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Qualified\":{\"Test\":1},\"AckID\":1}", &JSON_QUALIFIED),
	/* AckMsg Qualified with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Qualified\":[\"Test\"],\"AckID\":1}", &JSON_QUALIFIED),
	/* AckMsg Qualified with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Qualified\":\"Test\",\"AckID\":1}", &JSON_QUALIFIED),
	/* AckMsg ExtendedHeader with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"ExtHdr\":{\"Test\":1},\"AckID\":1}", &JSON_EXTHDR),
	/* AckMsg ExtendedHeader with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"ExtHdr\":[\"Test\"],\"AckID\":1}", &JSON_EXTHDR),
	/* AckMsg ExtendedHeader with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"ExtHdr\":1,\"AckID\":1}", &JSON_EXTHDR),
	/* AckMsg NakCode with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"NakCode\":{\"Test\":1},\"AckID\":1}", &JSON_NAKCODE),
	/* AckMsg NakCode with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"NakCode\":[\"Test\"],\"AckID\":1}", &JSON_NAKCODE)
));

INSTANTIATE_TEST_CASE_P(ErrorTestsGenericMessage, ErrorMsgParamFixture, ::testing::Values(
	/** GENERIC MESSAGE TESTS **/

	/* GenericMsg Key with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Key\":1}", &JSON_KEY),
	/* GenericMsg Key with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Generic\",\"Key\":[\"Test\"]}", &JSON_KEY),
	/* GenericMsg Key with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Key\":\"Test\"}", &JSON_KEY),
	/* GenericMsg ReqKey with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"ReqKey\":1}", &JSON_REQKEY),
	/* GenericMsg ReqKey with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Generic\",\"ReqKey\":[\"Test\"]}", &JSON_REQKEY),
	/* GenericMsg ReqKey with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"ReqKey\":\"Test\"}", &JSON_REQKEY),
	/* GenericMsg Complete with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Complete\":{\"Test\":1}}", &JSON_COMPLETE),
	/* GenericMsg Complete with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Complete\":[\"Test\"]}", &JSON_COMPLETE),
	/* GenericMsg Complete with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Complete\":\"Test\"}", &JSON_COMPLETE),
	/* GenericMsg PartNumber with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"PartNumber\":{\"Test\":1}}", &JSON_PARTNUMBER),
	/* GenericMsg PartNumber with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"PartNumber\":[\"Test\"]}", &JSON_PARTNUMBER),
	/* GenericMsg PartNumber with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"PartNumber\":\"Test\"}", &JSON_PARTNUMBER),
	/* GenericMsg PermData with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"PermData\":{\"Test\":1}}", &JSON_PERMDATA),
	/* GenericMsg PermData with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"PermData\":[\"Test\"]}", &JSON_PERMDATA),
	/* GenericMsg PermData with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"PermData\":1}", &JSON_PERMDATA),
	/* GenericMsg SeqNumber with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"SeqNumber\":{\"Test\":1}}", &JSON_SEQNUM),
	/* GenericMsg SeqNumber with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"SeqNumber\":[\"Test\"]}", &JSON_SEQNUM),
	/* GenericMsg SeqNumber with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"SeqNumber\":\"Test\"}", &JSON_SEQNUM),
	/* GenericMsg SecSeqNumber with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"SecSeqNumber\":{\"Test\":1}}", &JSON_SECSEQNUM),
	/* GenericMsg SecSeqNumber with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"SecSeqNumber\":[\"Test\"]}", &JSON_SECSEQNUM),
	/* GenericMsg SecSeqNumber with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"SecSeqNumber\":\"Test\"}", &JSON_SECSEQNUM),
	/* GenericMsg ExtHdr with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"ExtHdr\":{\"Test\":1}}", &JSON_EXTHDR),
	/* GenericMsg ExtHdr with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"ExtHdr\":[\"Test\"]}", &JSON_EXTHDR),
	/* GenericMsg ExtHdr with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"ExtHdr\":1}", &JSON_EXTHDR)

));

INSTANTIATE_TEST_CASE_P(ErrorTestsPostMessage, ErrorMsgParamFixture, ::testing::Values(
	/** POST MESSAGE TESTS **/

	/* PostMsg Key with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Key\":1}", &JSON_KEY),
	/* PostMsg Key with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Post\",\"Key\":[\"Test\"]}", &JSON_KEY),
	/* PostMsg Key with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Key\":\"Test\"}", &JSON_KEY),
	/* PostMsg PostUserInfo with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PostUserInfo\":1}", &JSON_POSTUSERINFO),
	/* PostMsg PostUserInfo with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,,\"Type\":\"Post\",\"PostUserInfo\":[\"Test\"]}", &JSON_POSTUSERINFO),
	/* PostMsg PostUserInfo with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PostUserInfo\":\"Test\"}", &JSON_POSTUSERINFO),
	/* PostMsg PostID with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PostID\":{\"Test\":1}}", &JSON_POSTID),
	/* PostMsg PostID with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PostID\":[\"Test\"]}", &JSON_POSTID),
	/* PostMsg PostID with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PostID\":\"Test\"}", &JSON_POSTID),
	/* PostMsg PartNumber with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PartNumber\":{\"Test\":1}}", &JSON_PARTNUMBER),
	/* PostMsg PartNumber with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PartNumber\":[\"Test\"]}", &JSON_PARTNUMBER),
	/* PostMsg PartNumber with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PartNumber\":\"Test\"}", &JSON_PARTNUMBER),
	/* PostMsg PostUserRights with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PostUserRights\":{\"Test\":1}}", &JSON_POSTUSERRIGHTS),
	/* PostMsg PostUserRights with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PostUserRights\":[\"Test\"]}", &JSON_POSTUSERRIGHTS),
	/* PostMsg PostUserRights with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PostUserRights\":\"Test\"}", &JSON_POSTUSERRIGHTS),
	/* PostMsg PermData with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PermData\":{\"Test\":1}}", &JSON_PERMDATA),
	/* PostMsg PermData with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PermData\":[\"Test\"]}", &JSON_PERMDATA),
	/* PostMsg PermData with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"PermData\":1}", &JSON_PERMDATA),
	/* PostMsg SeqNumber with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"SeqNumber\":{\"Test\":1}}", &JSON_SEQNUM),
	/* PostMsg SeqNumber with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"SeqNumber\":[\"Test\"]}", &JSON_SEQNUM),
	/* PostMsg SeqNumber with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"SeqNumber\":\"Test\"}", &JSON_SEQNUM),
	/* PostMsg Complete with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Complete\":{\"Test\":1}}", &JSON_COMPLETE),
	/* PostMsg Complete with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Complete\":[\"Test\"]}", &JSON_COMPLETE),
	/* PostMsg Complete with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Complete\":\"Test\"}", &JSON_COMPLETE),
	/* PostMsg Ack with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Ack\":{\"Test\":1}}", &JSON_ACK),
	/* PostMsg Ack with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Ack\":[\"Test\"]}", &JSON_ACK),
	/* PostMsg Ack with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Ack\":\"Test\"}", &JSON_ACK),
	/* PostMsg ExtHdr with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"ExtHdr\":{\"Test\":1}}", &JSON_EXTHDR),
	/* PostMsg ExtHdr with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"ExtHdr\":[\"Test\"]}", &JSON_EXTHDR),
	/* PostMsg ExtHdr with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"ExtHdr\":1}", &JSON_EXTHDR)

));

INSTANTIATE_TEST_CASE_P(ErrorTestsMap, ErrorMsgParamFixture, ::testing::Values(
	/** MAP TESTS **/
	/* Map with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Map\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_MAP),
	/* Map with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Map\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_MAP),
	/* Map with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Map\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_MAP),
	/* Map Entries with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Map\":{\"Entries\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Map Entries with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Map\":{\"Entries\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Map Entries with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Map\":{\"Entries\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Map Summary with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Map\":{\"Summary\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Map Summary with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Map\":{\"Summary\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Map Summary with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Map\":{\"Summary\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Map CountHint with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Map\":{\"CountHint\":\"Type\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* Map CountHint with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Map\":{\"CountHint\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* Map CountHint with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Map\":{\"CountHint\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* Map KeyFieldID with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Map\":{\"KeyFieldID\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_KEYFIELDID),
	/* Map KeyFieldId with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Map\":{\"KeyFieldID\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_KEYFIELDID),
	/* Map KeyFieldId with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Map\":{\"KeyFieldID\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_KEYFIELDID)
));


INSTANTIATE_TEST_CASE_P(ErrorTestsVector, ErrorMsgParamFixture, ::testing::Values(
	/** VECTOR TESTS **/

	/* Vector with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Vector\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_VECTOR),
	/* Vector Priority with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Vector\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_VECTOR),
	/* Vector Priority with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Vector\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_VECTOR),
	/* Vector Summary with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Vector\":{\"Summary\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Vector Summary with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Vector\":{\"Summary\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Vector Summary with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Vector\":{\"Summary\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Vector SupportSorting with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Vector\":{\"SupportSorting\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUPPORTSORTING),
	/* Vector SupportSorting with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Vector\":{\"SupportSorting\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUPPORTSORTING),
	/* Vector SupportSorting with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Vector\":{\"SupportSorting\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUPPORTSORTING),
	/* Vector CountHint with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Vector\":{\"CountHint\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* Vector CountHint with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Vector\":{\"CountHint\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* Vector CountHint with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Vector\":{\"CountHint\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* Vector Entries with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Vector Entries with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Vector Entries with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Vector Entries Action with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":{\"Test\":1},\"Index\":1,\"PermData\":\"abcdefg\"}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ACTION),
	/* Vector Entries Action with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":[\"Test\"],\"Index\":1,\"PermData\":\"abcdefg\"}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ACTION),
	/* Vector Entries Action with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":1,\"Index\":1,\"PermData\":\"abcdefg\"}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ACTION),
	/* Vector Entries Index with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":\"Update\",\"Index\":{\"Test\"},\"PermData\":\"abcdefg\"}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_INDEX),
	/* Vector Entries Index with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":\"Update\",\"Index\":[\"Test\"],\"PermData\":\"abcdefg\"}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_INDEX),
	/* Vector Entries Index with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":\"Update\",\"Index\":\"Test\",\"PermData\":\"abcdefg\"}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_INDEX),
	/* Vector Entries PermData with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":\"Update\",\"Index\":1,\"PermData\":{\"Test\":1}}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PERMDATA),
	/* Vector Entries PermData with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":\"Update\",\"Index\":1,\"PermData\":[\"Test\"]}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PERMDATA),
	/* Vector Entries PermData with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_STRING, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Vector\":{\"Entries\":[{\"Action\":\"Update\",\"Index\":1,\"PermData\":1}]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_PERMDATA)

));

INSTANTIATE_TEST_CASE_P(ErrorTestsSeries, ErrorMsgParamFixture, ::testing::Values(
	/** SERIES TESTS **/
	/* Series with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Series\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SERIES),
	/* Series with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Series\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SERIES),
	/* Series with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Series\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SERIES),
	/* Series Entries with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Series\":{\"Entries\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Series Entries with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Series\":{\"Entries\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Series Entries with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Series\":{\"Entries\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* Series Summary with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"Series\":{\"Summary\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Series Summary with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Series\":{\"Summary\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Series Summary with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Series\":{\"Summary\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_SUMMARY),
	/* Series CountHint with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"Series\":{\"CountHint\":\"Type\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* Series CountHint with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"Series\":{\"CountHint\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* Series CountHint with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"Series\":{\"CountHint\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT)
));

INSTANTIATE_TEST_CASE_P(ErrorTestsFilterList, ErrorMsgParamFixture, ::testing::Values(
	/** FILTERLIST TESTS **/
	/* FilterList with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"FilterList\":1,\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_FILTERLIST),
	/* FilterList with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"FilterList\":[\"Test\"],\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_FILTERLIST),
	/* FilterList with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_OBJECT, JSMN_STRING, 2, (char *)"{\"ID\":2,\"FilterList\":\"Test\",\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_FILTERLIST),
	/* FilterList Entries with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"FilterList\":{\"Entries\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* FilterList Entries with primitive */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_PRIMITIVE, 2, (char *)"{\"ID\":2,\"FilterList\":{\"Entries\":1},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* FilterList Entries with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_ARRAY, JSMN_STRING, 2, (char *)"{\"ID\":2,\"FilterList\":{\"Entries\":\"Test\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_ENTRIES),
	/* FilterList CountHint with string */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_STRING, 2, (char *)"{\"ID\":2,\"FilterList\":{\"CountHint\":\"Type\"},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* FilterList CountHint with array */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_ARRAY, 2, (char *)"{\"ID\":2,\"FilterList\":{\"CountHint\":[\"Test\"]},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT),
	/* FilterList CountHint with object */
	ErrorMsgTestParams(jsonToRwfBase::INVALID_TOKEN_TYPE, JSMN_PRIMITIVE, JSMN_OBJECT, 2, (char *)"{\"ID\":2,\"FilterList\":{\"CountHint\":{\"Test\":1}},\"Key\":{\"Name\":\"TRI.N\"}}", &JSON_COUNTHINT)
));

/* Test unexpected keys in each type of container and entry.
 * Converter identifies parameters initially by looking at the first letter (before doing an actually string comparison). So test a word starting with each letter in the alphabet. */

INSTANTIATE_TEST_CASE_P(MapUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"1\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Alpha\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Bravo\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Charlie\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Delta\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Echo\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Foxtrot\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Golf\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Hotel\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"India\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Juliet\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Kilo\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Mike\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"November\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Oscar\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Papa\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Quebec\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Romeo\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Sierra\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Tango\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Uniform\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Victor\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Whiskey\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"X-ray\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Yankee\":{}}}", &JSON_MAP),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[], \"Zulu\":{}}}", &JSON_MAP)
));

INSTANTIATE_TEST_CASE_P(MapEntryUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"1\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Alpha\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Bravo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Charlie\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Delta\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Echo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Foxtrot\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Golf\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Hotel\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"India\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Juliet\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Kilo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Mike\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"November\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Oscar\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Papa\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Quebec\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Romeo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Sierra\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Tango\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Uniform\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Victor\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Whiskey\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"X-ray\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Yankee\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[{\"Action\":\"Add\", \"Key\":1,\"Zulu\":{}}]}}")
));

INSTANTIATE_TEST_CASE_P(VectorUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"1\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Alpha\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Bravo\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Charlie\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Delta\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Echo\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Foxtrot\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Golf\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Hotel\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"India\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Juliet\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Kilo\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Mike\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"November\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Oscar\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Papa\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Quebec\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Romeo\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Sierra\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Tango\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Uniform\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Victor\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Whiskey\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"X-ray\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Yankee\":{}}}", &JSON_VECTOR),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[], \"Zulu\":{}}}", &JSON_VECTOR)
));

INSTANTIATE_TEST_CASE_P(VectorEntryUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"1\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Alpha\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Bravo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Charlie\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Delta\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Echo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Foxtrot\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Golf\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Hotel\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"India\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Juliet\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Kilo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Mike\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"November\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Oscar\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Papa\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Quebec\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Romeo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Sierra\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Tango\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Uniform\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Victor\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Whiskey\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"X-ray\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Yankee\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Vector\":{\"Entries\":[{\"Action\":\"Set\",\"Index\":1, \"Zulu\":{}}]}}")
));

INSTANTIATE_TEST_CASE_P(SeriesUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"1\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Alpha\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Bravo\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Charlie\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Delta\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Echo\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Foxtrot\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Golf\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Hotel\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"India\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Juliet\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Kilo\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Mike\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"November\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Oscar\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Papa\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Quebec\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Romeo\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Sierra\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Tango\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Uniform\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Victor\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Whiskey\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"X-ray\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Yankee\":{}}}", &JSON_SERIES),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[], \"Zulu\":{}}}", &JSON_SERIES)
));

INSTANTIATE_TEST_CASE_P(SeriesEntryUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"1\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Alpha\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Bravo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Charlie\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Delta\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Echo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Foxtrot\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Golf\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Hotel\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"India\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Juliet\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Kilo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Mike\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"November\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Oscar\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Papa\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Quebec\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Romeo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Sierra\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Tango\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Uniform\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Victor\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Whiskey\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"X-ray\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Yankee\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Series\":{\"Entries\":[{\"Zulu\":{}}]}}")
));

INSTANTIATE_TEST_CASE_P(FilterListUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"1\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Alpha\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Bravo\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Charlie\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Delta\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Echo\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Foxtrot\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Golf\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Hotel\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"India\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Juliet\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Kilo\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Mike\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"November\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Oscar\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Papa\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Quebec\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Romeo\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Sierra\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Tango\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Uniform\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Victor\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Whiskey\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"X-ray\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Yankee\":{}}}", &JSON_FILTERLIST),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[], \"Zulu\":{}}}", &JSON_FILTERLIST)
));

INSTANTIATE_TEST_CASE_P(FilterEntryUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"1\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Alpha\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Bravo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Charlie\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Delta\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Echo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Foxtrot\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Golf\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Hotel\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"India\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Juliet\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Kilo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Mike\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"November\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Oscar\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Papa\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Quebec\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Romeo\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Sierra\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Tango\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Uniform\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Victor\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Whiskey\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"X-ray\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Yankee\":{}}]}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"FilterList\":{\"Entries\":[{\"Action\":\"Set\",\"ID\":1, \"Zulu\":{}}]}}")
));

INSTANTIATE_TEST_CASE_P(ArrayUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"1\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Alpha\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Bravo\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Charlie\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Delta\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Echo\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Foxtrot\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Golf\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Hotel\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"India\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Juliet\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Kilo\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Mike\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"November\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Oscar\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Papa\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Quebec\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Romeo\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Sierra\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Tango\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Uniform\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Victor\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Whiskey\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"X-ray\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Yankee\":{}}}}", &RSSL_OMMSTR_DT_ARRAY),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2,  (char *)"{\"ID\":2,\"Type\":\"Update\",\"Fields\":{\"ARRAY\":{\"Type\":\"Int\",\"Data\":[], \"Zulu\":{}}}}", &RSSL_OMMSTR_DT_ARRAY)
));

INSTANTIATE_TEST_CASE_P(RequestMessageUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"1\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Alpha\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Bravo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Charlie\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Delta\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Echo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Foxtrot\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Golf\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Hotel\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"India\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Juliet\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Kilo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Mike\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"November\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Oscar\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Papa\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Quebec\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Romeo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Sierra\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Tango\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Uniform\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Victor\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Whiskey\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"X-ray\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Yankee\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Request\",\"Zulu\":{}}")
));

INSTANTIATE_TEST_CASE_P(RefreshMessageUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"1\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Alpha\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Bravo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Charlie\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Delta\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Echo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Foxtrot\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Golf\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Hotel\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"India\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Juliet\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Kilo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Mike\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"November\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Oscar\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Papa\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Quebec\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Romeo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Sierra\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Tango\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Uniform\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Victor\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Whiskey\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"X-ray\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Yankee\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Refresh\",\"Zulu\":{}}")
));

INSTANTIATE_TEST_CASE_P(StatusMessageUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"1\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Alpha\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Bravo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Charlie\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Delta\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Echo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Foxtrot\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Golf\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Hotel\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"India\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Juliet\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Kilo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Mike\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"November\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Oscar\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Papa\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Quebec\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Romeo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Sierra\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Tango\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Uniform\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Victor\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Whiskey\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"X-ray\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Yankee\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Status\",\"Zulu\":{}}")
));

INSTANTIATE_TEST_CASE_P(UpdateMessageUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"1\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Alpha\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Bravo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Charlie\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Delta\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Echo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Foxtrot\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Golf\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Hotel\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"India\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Juliet\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Kilo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Mike\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"November\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Oscar\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Papa\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Quebec\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Romeo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Sierra\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Tango\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Uniform\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Victor\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Whiskey\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"X-ray\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Yankee\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Update\",\"Zulu\":{}}")
));

INSTANTIATE_TEST_CASE_P(CloseMessageUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"1\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Alpha\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Bravo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Charlie\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Delta\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Echo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Foxtrot\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Golf\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Hotel\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"India\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Juliet\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Kilo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Mike\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"November\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Oscar\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Papa\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Quebec\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Romeo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Sierra\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Tango\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Uniform\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Victor\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Whiskey\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"X-ray\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Yankee\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Close\",\"Zulu\":{}}")
));

INSTANTIATE_TEST_CASE_P(AckMessageUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"1\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Alpha\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Bravo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Charlie\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Delta\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Echo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Foxtrot\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Golf\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Hotel\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"India\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Juliet\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Kilo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Mike\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"November\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Oscar\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Papa\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Quebec\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Romeo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Sierra\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Tango\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Uniform\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Victor\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Whiskey\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"X-ray\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Yankee\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Ack\",\"Zulu\":{}}")
));

INSTANTIATE_TEST_CASE_P(GenericMessageUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"1\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Alpha\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Bravo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Charlie\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Delta\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Echo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Foxtrot\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Golf\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Hotel\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"India\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Juliet\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Kilo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Mike\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"November\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Oscar\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Papa\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Quebec\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Romeo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Sierra\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Tango\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Uniform\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Victor\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Whiskey\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"X-ray\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Yankee\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Generic\",\"Zulu\":{}}")
));

INSTANTIATE_TEST_CASE_P(PostMessageUnexpectedKeyTests, ErrorMsgParamFixture, ::testing::Values(
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"1", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"1\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Alpha", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Alpha\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Bravo", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Bravo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Charlie", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Charlie\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Delta", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Delta\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Echo", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Echo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Foxtrot", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Foxtrot\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Golf", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Golf\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Hotel", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Hotel\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"India", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"India\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Juliet", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Juliet\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Kilo", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Kilo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Mike", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Mike\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"November", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"November\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Oscar", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Oscar\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Papa", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Papa\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Quebec", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Quebec\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Romeo", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Romeo\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Sierra", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Sierra\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Tango", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Tango\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Uniform", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Uniform\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Victor", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Victor\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Whiskey", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Whiskey\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"X-ray", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"X-ray\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Yankee", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Yankee\":{}}"),
	ErrorMsgTestParams(jsonToRwfBase::UNEXPECTED_KEY, (char*)"Zulu", 2, (char *)"{\"ID\":2,\"Type\":\"Post\",\"Zulu\":{}}")
));

/* Test a JSON1 message with no base. */
TEST_F(ErrorMsgTests, Json1NoMsgBase)
{
	RsslJsonConverterError converterError;

	setJsonBufferToString("{\"u\":1,\"k\":{\"s\":555,\"n\":\"TINY\"},\"d\":{\"d\":{}}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError(RSSL_JSON_JPT_JSON, &converterError));
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(converterError.text, "JSON message with no message Base."));

	setJsonBufferToString("{[\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\",\"a\"]}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError(RSSL_JSON_JPT_JSON, &converterError));
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(converterError.text, "JSON message with no message Base."));
}
