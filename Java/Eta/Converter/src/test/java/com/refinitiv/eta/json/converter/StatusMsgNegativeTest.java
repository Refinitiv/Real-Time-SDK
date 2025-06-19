/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.rdm.DomainTypes;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;
import java.util.Objects;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.checkJsonErrorMsg;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class StatusMsgNegativeTest {
    Buffer jsonBuffer;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    StatusMsg statusMsg = (StatusMsg) CodecFactory.createMsg();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();
    int streamId = MsgClasses.STATUS;
    Buffer errorBuffer = CodecFactory.createBuffer();
    GetJsonErrorParams errorParams = ConverterFactory.createJsonErrorParams();

    @Before
    public void init() {
        jsonMsg.clear();
        initMsg();
        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .build(convError);
        JsonConverterProxy proxy = new JsonConverterProxy(converter);
        this.converter = (JsonConverter) Proxy.newProxyInstance(JsonConverter.class.getClassLoader(),
                new Class[]{JsonConverter.class},
                proxy);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(streamId);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        conversionResults.setLength(0);
        errorParams.clear();
    }

    private void initMsg() {
        statusMsg.clear();
        statusMsg.streamId(streamId);
        statusMsg.domainType(DomainTypes.MARKET_PRICE);
        statusMsg.msgClass(streamId);
        statusMsg.encodedDataBody(CodecFactory.createBuffer());
        statusMsg.encodedDataBody().data(ByteBuffer.allocate(0));
    }

    @Test
    public void givenEmptyExtendedHdrBuffer_whenExtendedHeaderApplied_thenRwfToJsonConversionFails() {
        statusMsg.applyHasExtendedHdr();

        //Failure because we must specify data buffer for extendedHdr.
        assertEquals(FAILURE, converter.convertRWFToJson(statusMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenNullableGroupId_whenGroupIdApplied_thenRwfToJsonConversionSucceeds() {
        statusMsg.applyHasGroupId();
        assertTrue(Objects.isNull(statusMsg.groupId().data()));

        //SUCCESS because we can write "null" group id data into msg.
        assertEquals(SUCCESS, converter.convertRWFToJson(statusMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenEmptyPermData_whenPermDataApplied_thenRwfToJsonConversionFails() {
        statusMsg.applyHasPermData();
        assertTrue(Objects.isNull(statusMsg.permData().data()));

        //Failure because we must specify data buffer for permdata.
        assertEquals(FAILURE, converter.convertRWFToJson(statusMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenUnfilledState_whenStateIsApplied_thenRwfToJsonConversionSucceeds() {
        statusMsg.applyHasState();
        assertEquals(SUCCESS, converter.convertRWFToJson(statusMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenUnfilledPostUserInfo_whenPostUserInfoIsApplied_thenRwfToJsonConversionSucceeds() {
        statusMsg.applyHasPostUserInfo();
        assertEquals(SUCCESS, converter.convertRWFToJson(statusMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenJsonWithInvalidUserData_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String statusMsg = "{\"ID\":5,\"Type\":\"Status\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"PostUserInfo\":{\"Address\":\"no_address\",\"UserID\":0}}";
        jsonBuffer.data(statusMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(convError.getCode(), JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE);
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenJsonWithInvalidStateData_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String statusMsg = "{\"ID\":5,\"Type\":\"Status\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"no\",\"Data\":\"data\",\"Code\":\"undefined\",\"Text\":\"All is well\"}}";
        jsonBuffer.data(statusMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenJsonWithEmptyStreamId_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String genericMsg = "{\"Type\":\"Status\"}";
        jsonBuffer.data(genericMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, convError.getCode());
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenJsonWithNullKeyName_whenJsonIsDecodingToRwf_thenDecodingFails() {
        final String statusMsg = "{\"ID\":5,\"Type\":\"Status\",\"Key\":{\"Name\":null}}";
        jsonBuffer.data(statusMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }
}
