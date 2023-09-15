/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportFactory;
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

public class RefreshMsgNegativeTest {
    Buffer jsonBuffer;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    RefreshMsg refreshMsg = (RefreshMsg) CodecFactory.createMsg();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();
    int streamId = MsgClasses.REFRESH;
    Buffer errorBuffer = CodecFactory.createBuffer();
    GetJsonErrorParams errorParams = ConverterFactory.createJsonErrorParams();

    @Before
    public void init() {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "../../etc/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);
        jsonMsg.clear();
        initMsg();
        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .setDictionary(dictionary)
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
        refreshMsg.clear();
        refreshMsg.streamId(streamId);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.msgClass(streamId);
        refreshMsg.encodedDataBody(CodecFactory.createBuffer());
        refreshMsg.encodedDataBody().data(ByteBuffer.allocate(0));
    }

    @Test
    public void givenEmptyExtendedHdrBuffer_whenExtendedHeaderApplied_thenRwfToJsonConversionFails() {
        refreshMsg.applyHasExtendedHdr();

        //Failure because we must specify data buffer for extendedHdr.
        assertEquals(FAILURE, converter.convertRWFToJson(refreshMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenEmptyPermData_whenPermDataApplied_thenRwfToJsonConversionFails() {
        refreshMsg.applyHasPermData();
        assertTrue(Objects.isNull(refreshMsg.permData().data()));

        //Failure because we must specify data buffer for permdata.
        assertEquals(FAILURE, converter.convertRWFToJson(refreshMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenJsonWithEmptyStreamId_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String refreshMsg = "{\"Type\":\"Refresh\"}";
        jsonBuffer.data(refreshMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, convError.getCode());
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenJsonWithInvalidUserData_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String statusMsg = "{\"ID\":3,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"PostUserInfo\":{\"Address\":\"no_address\",\"UserID\":0}}";
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
        final String statusMsg = "{\"ID\":3,\"Type\":\"Refresh\",\"Key\":{\"Service\":555,\"Name\":\"TINY\"},\"State\":{\"Stream\":\"no\",\"Data\":\"data\",\"Code\":\"undefined\",\"Text\":\"All is well\"}}";
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
    public void givenJsonDateHasNoDay_decodingDoesntThrowError() throws IOException {
        final String refreshMsg = "{\"ID\":3,\"Type\":\"Refresh\",\"Key\": " +
                "{\"Service\":\"ELEKTRON_DD\",\"Name\":\"ALINN.PA\"}," +
                "\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"*All is well\"}," +
                "\"Qos\":{\"Timeliness\":\"Realtime\",\"Rate\":\"TickByTick\"},\"PermData\":\"AxHygTLA\",\"SeqNumber\":4768," +
                "\"Fields\":{\"DIVPAYDATE\":\"2022-11\"}}";
        jsonBuffer.data(refreshMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }
}
