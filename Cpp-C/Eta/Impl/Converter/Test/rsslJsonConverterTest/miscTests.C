#include "rsslJsonConverterTestBase.h"
#include <cstdarg>

using namespace std;
using namespace rapidjson; 

/* Fixture for MiscTests that has conversion code. */
class MiscTests : public MsgConversionTestBase
{
};


/* Send a message with one space. */
TEST_F(MiscTests, OneSpaceMsg)
{
	RsslDecodeJsonMsgOptions decodeJsonMsgOptions;
	RsslJsonMsg jsonMsg;
	RsslJsonConverterError converterError;
	RsslParseJsonBufferOptions parseOptions;

	_jsonBuffer.data = (char*)" ";
	_jsonBuffer.length = 1;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
#endif

	rsslClearDecodeJsonMsgOptions(&decodeJsonMsgOptions);
	decodeJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

}

/* Test ping and pong messages */
TEST_F(MiscTests, PingAndPong)
{
	RsslDecodeJsonMsgOptions decodeJsonMsgOptions;
	RsslJsonMsg jsonMsg;
	RsslJsonConverterError converterError;
	RsslParseJsonBufferOptions parseOptions;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	rsslClearDecodeJsonMsgOptions(&decodeJsonMsgOptions);
	decodeJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	/* Ping */
	_jsonBuffer.data = (char*)"{\"Type\":\"Ping\"}";
	_jsonBuffer.length = strlen(_jsonBuffer.data);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_PING, jsonMsg.msgBase.msgClass);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	/* Pong */
	_jsonBuffer.data = (char*)"{\"Type\":\"Pong\"}";
	_jsonBuffer.length = strlen(_jsonBuffer.data);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_PONG, jsonMsg.msgBase.msgClass);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
}

/* Test packed messages */
TEST_F(MiscTests, PackedMessages)
{
	RsslDecodeJsonMsgOptions decodeJsonMsgOptions;
	RsslJsonMsg jsonMsg;
	RsslJsonConverterError converterError;
	RsslParseJsonBufferOptions parseOptions;
	RsslBuffer keyName = {4, (char*)"ROLL"};
	RsslBuffer keyName2 = {4, (char*)"TINY"};

	/* Empty pack. */
	_jsonBuffer.data = (char*)"[]";
	_jsonBuffer.length = 2;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	rsslClearDecodeJsonMsgOptions(&decodeJsonMsgOptions);
	decodeJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	/* One message */
	_jsonBuffer.data = (char*)"[{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}}]";
	_jsonBuffer.length = strlen(_jsonBuffer.data);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	ASSERT_EQ(RSSL_JSON_MC_RSSL_MSG, jsonMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_MC_REQUEST, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId);
	ASSERT_TRUE(rsslMsgKeyCheckHasName(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey));
	EXPECT_TRUE(rsslBufferIsEqual(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey.name, &keyName));

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	/* Two messages */
	_jsonBuffer.data = (char*)"[{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}},{\"ID\":5,\"Type\":\"Close\"}]";
	_jsonBuffer.length = strlen(_jsonBuffer.data);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_RSSL_MSG, jsonMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_MC_REQUEST, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId);
	ASSERT_TRUE(rsslMsgKeyCheckHasName(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey));
	EXPECT_TRUE(rsslBufferIsEqual(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey.name, &keyName));

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_RSSL_MSG, jsonMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_MC_CLOSE, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	/* Three messages */
	_jsonBuffer.data = (char*)"[{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}},{\"ID\":5,\"Type\":\"Close\"}],{\"ID\":6,\"Key\":{\"Name\":\"TINY\"}}";
	_jsonBuffer.length = strlen(_jsonBuffer.data);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_RSSL_MSG, jsonMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_MC_REQUEST, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId);
	ASSERT_TRUE(rsslMsgKeyCheckHasName(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey));
	EXPECT_TRUE(rsslBufferIsEqual(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey.name, &keyName));

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_RSSL_MSG, jsonMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_MC_CLOSE, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_RSSL_MSG, jsonMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_MC_REQUEST, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgClass);
	EXPECT_EQ(6, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId);
	ASSERT_TRUE(rsslMsgKeyCheckHasName(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey));
	EXPECT_TRUE(rsslBufferIsEqual(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey.name, &keyName2));

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	/* Message and Ping */
	_jsonBuffer.data = (char*)"[{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}},{\"Type\":\"Ping\"}]";
	_jsonBuffer.length = strlen(_jsonBuffer.data);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_RSSL_MSG, jsonMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_MC_REQUEST, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId);
	ASSERT_TRUE(rsslMsgKeyCheckHasName(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey));
	EXPECT_TRUE(rsslBufferIsEqual(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey.name, &keyName));

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_PING, jsonMsg.msgBase.msgClass);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	/* Pong and Message */
	_jsonBuffer.data = (char*)"[{\"Type\":\"Pong\"},{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}}]";
	_jsonBuffer.length = strlen(_jsonBuffer.data);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
	ASSERT_EQ(RSSL_JSON_MC_PONG, jsonMsg.msgBase.msgClass);

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	ASSERT_EQ(RSSL_JSON_MC_RSSL_MSG, jsonMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_MC_REQUEST, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, jsonMsg.jsonRsslMsg.rsslMsg.msgBase.streamId);
	ASSERT_TRUE(rsslMsgKeyCheckHasName(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey));
	EXPECT_TRUE(rsslBufferIsEqual(&jsonMsg.jsonRsslMsg.rsslMsg.msgBase.msgKey.name, &keyName));

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif
}

