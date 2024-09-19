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
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.UpdateMsg;
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

public class UpdateMsgNegativeTest {

    Buffer jsonBuffer;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    UpdateMsg updateMsg = (UpdateMsg) CodecFactory.createMsg();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();
    int streamId = MsgClasses.UPDATE;
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
        updateMsg.clear();
        updateMsg.streamId(streamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.msgClass(streamId);
        updateMsg.encodedDataBody(CodecFactory.createBuffer());
        updateMsg.encodedDataBody().data(ByteBuffer.allocate(0));
    }

    @Test
    public void givenEmptyExtendedHdrBuffer_whenExtendedHeaderApplied_thenRwfToJsonConversionFails() {
        updateMsg.applyHasExtendedHdr();

        //Failure because we must specify data buffer for extendedHdr.
        assertEquals(FAILURE, converter.convertRWFToJson(updateMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenEmptyPermData_whenPermDataApplied_thenRwfToJsonConversionFails() {
        updateMsg.applyHasPermData();
        assertTrue(Objects.isNull(updateMsg.permData().data()));

        //Failure because we must specify data buffer for permdata.
        assertEquals(FAILURE, converter.convertRWFToJson(updateMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenJsonWithEmptyStreamId_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String update = "{\"Type\":\"Update\"}";
        jsonBuffer.data(update);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY, convError.getCode());
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        checkJsonErrorMsg(errorBuffer);
    }
}
