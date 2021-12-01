/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 Refinitiv. All rights reserved. –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"
#include <cstdarg>

using namespace std;
using namespace json; 

/* Fixture for MiscTests that has conversion code. */
class MiscTests : public MsgConversionTestBase
{
};


/* Send an ampty message. */
TEST_F(MiscTests, EmptyMsg)
{
	RsslJsonConverterError converterError;
	RsslParseJsonBufferOptions parseOptions;

	/* General empty buffer case */
	_jsonBuffer.data = NULL;
	_jsonBuffer.length = 0;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_LE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_FAILURE);
#else
	ASSERT_LE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_FAILURE);
#endif
	EXPECT_EQ(0, strncmp(converterError.text, "Empty JSON Message", MAX_CONVERTER_ERROR_TEXT));

	/* Empty buffer inconsistency case: zero length */
	_jsonBuffer.data = (char*)"";
	_jsonBuffer.length = 0;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_LE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_FAILURE);
#else
	ASSERT_LE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_FAILURE);
#endif
	EXPECT_EQ(0, strncmp(converterError.text, "Empty JSON Message", MAX_CONVERTER_ERROR_TEXT));

	/* Empty buffer inconsistency case: NULL pointer */
	_jsonBuffer.data = NULL;
	_jsonBuffer.length = 42;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_LE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_FAILURE);
#else
	ASSERT_LE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_FAILURE);
#endif
	EXPECT_EQ(0, strncmp(converterError.text, "Empty JSON Message", MAX_CONVERTER_ERROR_TEXT));
}

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
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

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
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

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
	char oneMsg[] = "[{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}}]";
	_jsonBuffer.data = oneMsg;
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

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
	char twoMsg[] = "[{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}},{\"ID\":5,\"Type\":\"Close\"}]";
	_jsonBuffer.data = twoMsg;
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

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
	char threeMsg[] = "[{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}},{\"ID\":5,\"Type\":\"Close\"}],{\"ID\":6,\"Key\":{\"Name\":\"TINY\"}}";
	_jsonBuffer.data = threeMsg;
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

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
	char pingMsg[] = "[{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}},{\"Type\":\"Ping\"}]";
	_jsonBuffer.data = pingMsg;
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

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
	char pongMsg[] = "[{\"Type\":\"Pong\"},{\"ID\":5,\"Key\":{\"Name\":\"ROLL\"}}]";
	_jsonBuffer.data = pongMsg;
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

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

