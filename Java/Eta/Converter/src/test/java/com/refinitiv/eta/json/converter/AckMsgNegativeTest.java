/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.rdm.DomainTypes;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;
import java.util.Objects;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class AckMsgNegativeTest {
    Buffer jsonBuffer;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    AckMsg ackMsg = (AckMsg) CodecFactory.createMsg();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();
    int streamId = MsgClasses.ACK;
    Buffer errorBuffer = CodecFactory.createBuffer();
    GetJsonErrorParams errorParams = ConverterFactory.createJsonErrorParams();

    @Before
    public void init() {
        jsonMsg.clear();
        initMsg();
        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE));
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
        ackMsg.clear();
        ackMsg.streamId(streamId);
        ackMsg.domainType(DomainTypes.MARKET_PRICE);
        ackMsg.msgClass(streamId);
        ackMsg.encodedDataBody(CodecFactory.createBuffer());
        ackMsg.encodedDataBody().data(ByteBuffer.allocate(0));
    }

    @Test
    public void givenEmptyExtendedHdrBuffer_whenExtendedHeaderApplied_thenRwfToJsonConversionFails() {
        ackMsg.applyHasExtendedHdr();

        //Failure because we must specify data buffer for extendedHdr.
        assertEquals(FAILURE, converter.convertRWFToJson(ackMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenNullableTextData_whenTextApplied_thenRwfToJsonConversionSucceeds() {
        ackMsg.applyHasText();
        assertTrue(Objects.isNull(ackMsg.text().data()));

        //SUCCESS because we can write "null" text data into msg.
        assertEquals(SUCCESS, converter.convertRWFToJson(ackMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenEmptyMsgKeyName_whenMsgKeyNameIsApplied_thenRwfToJsonConversionSucceeds() {
        ackMsg.applyHasMsgKey();
        ackMsg.msgKey().applyHasName();
        assertEquals(SUCCESS, converter.convertRWFToJson(ackMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenJsonWithBooleanNakCode_whenJsonIsDecodingToRwf_thenDecodingFails() {
        final String ackMsg = "{\"ID\":6,\"Type\":\"Ack\",\"AckID\":0,\"NakCode\":false,\"Key\":{\"Service\":555,\"Name\":\"TINY\"}}";
        jsonBuffer.data(ackMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
    }

    @Test
    public void givenJsonWithIntegerText_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String ackMsg = "{\"ID\":6,\"Type\":\"Ack\",\"AckID\":0,\"NakCode\":\"Text\",\"Text\":50,\"Key\":{\"Service\":555,\"Name\":\"TINY\"}}";
        jsonBuffer.data(ackMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        JsonConverterTestUtils.checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenNonBase64String_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException  {
        //Any string could be decoded as from base 64 string representation.
        final String ackMsg = "{\"ID\":6,\"Type\":\"Ack\",\"AckID\":0,\"ExtHdr\":\"\u0020\u0030\",\"Qualified\":true}";
        jsonBuffer.data(ackMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        JsonConverterTestUtils.checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenIntegerValueForExtendedHdr_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String ackMsg = "{\"ID\":6,\"Type\":\"Ack\",\"AckID\":0,\"ExtHdr\":50,\"Qualified\":true}";

        jsonBuffer.data(ackMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        //Integer cannot be decoded as base 64 string.
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        JsonConverterTestUtils.checkJsonErrorMsg(errorBuffer);
    }
}
