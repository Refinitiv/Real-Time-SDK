/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Msg;
import org.junit.Before;
import org.junit.Test;

import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class BaseMsgNegativeTest {

    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    Buffer jsonBuffer;
    Msg msg = CodecFactory.createMsg();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();

    @Before
    public void init() {
        msg.clear();
        jsonMsg.clear();
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
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        conversionResults.setLength(0);
    }

    @Test
    public void givenIncorrectJsonProtocol_whenMsgIsEncodedToJson_thenRwfToJsonEncodingFails() {
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_UNKNOWN);
        assertEquals(FAILURE, converter.convertRWFToJson(msg, rwfToJsonOptions, conversionResults, convError));
        assertTrue(convError.isFailed());
        assertJsonError("Invalid protocol type.", convError);
    }

    @Test
    public void givenMsgWithEmptyDataBuffer_whenMsgIsEncodedToJson_thenRwfToJsonEncodingFails() {
        //We haven't initialized inner buffer of Msg bean.
        assertEquals(FAILURE, converter.convertRWFToJson(msg, rwfToJsonOptions, conversionResults, convError));
        assertJsonError("RWF Msg encodedDataBody() is not initialized.", convError);
    }

    @Test
    public void givenMsgWithoutClass_whenMsgIsEncodedToJson_thenRwfToJsonEncodingFails() {
        msg.encodedDataBody(CodecFactory.createBuffer());
        msg.encodedDataBody().data(ByteBuffer.allocate(0));
        assertEquals(FAILURE, converter.convertRWFToJson(msg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenInvalidJsonStructure_whenJsonIsParsedToRwf_thenParsingFails() {
        jsonBuffer.data("{\"InvalidJSON\"");
        assertEquals(FAILURE, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_PARSE_ERROR, convError.getCode());
    }

    @Test
    public void givenIncorrectJsonProtocol_whenJsonIsParsedToRwf_thenParsingFails() {
        jsonBuffer.data("{\"key\":\"Valid JSON\"}");
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_UNKNOWN);
        assertEquals(FAILURE, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR, convError.getCode());
        assertJsonError("Protocol type not supported: 0", convError);
    }

    @Test
    public void givenIncorrectJsonProtocol_whenJsonIsDecodedToRwf_thenDecodingFails() {
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_UNKNOWN);
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR, convError.getCode());
        assertJsonError("Protocol type not supported: 0", convError);
    }

    @Test
    public void givenMsgWithNotExistedType_whenJsonIsDecodingToRwf_thenDecodingFails() {
        final String undefinedMsg = "{\"Type\": \"undefined\"}";
        jsonBuffer.data(undefinedMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_MSG_TYPE, convError.getCode());
        assertJsonError("Message type is not supported: \"undefined\"", convError);
    }

    @Test
    public void givenJsonWithoutMsgType_whenJsonKeyIsMissing_thenDecodingFails() {
        final String undefinedMsg = "{\"key\": \"undefined\"}";
        jsonBuffer.data(undefinedMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, convError.getCode());
    }

    public static void assertJsonError(String expectedText, JsonConverterError error) {
        assertEquals("JSON Converter error: " + expectedText, error.getText());
    }
}
