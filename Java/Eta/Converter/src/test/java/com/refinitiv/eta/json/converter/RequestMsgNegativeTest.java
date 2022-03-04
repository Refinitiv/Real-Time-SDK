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
import com.refinitiv.eta.rdm.ViewTypes;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;

import static com.refinitiv.eta.codec.CodecReturnCodes.FAILURE;
import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.BaseMsgNegativeTest.assertJsonError;
import static com.refinitiv.eta.json.converter.ConstCharArrays.*;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.checkJsonErrorMsg;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;

public class RequestMsgNegativeTest {
    Buffer jsonBuffer;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    RequestMsg requestMsg = (RequestMsg) CodecFactory.createMsg();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();
    int streamId = MsgClasses.REQUEST;
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
        requestMsg.clear();
        requestMsg.streamId(streamId);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgClass(streamId);
        requestMsg.encodedDataBody(CodecFactory.createBuffer());
        requestMsg.encodedDataBody().data(ByteBuffer.allocate(0));
    }

    @Test
    public void givenUndefinedQos_whenQosIsApplied_thenRwfToJsonConversionSucceeds() {
        requestMsg.applyHasQos();
        assertEquals(SUCCESS, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenUndefinedWorstQos_whenWorstQosIsApplied_thenRwfToJsonConversionSucceeds() {
        requestMsg.applyHasWorstQos();
        assertEquals(SUCCESS, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenEmptyMsgKeyName_whenMsgKeyNameIsApplied_thenRwfToJsonConversionSucceeds() {
        requestMsg.msgKey().applyHasName();
        assertEquals(SUCCESS, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenEmptyExtendedHdr_whenExtendedHdrIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasExtendedHdr();
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenEmptyContainerBody_whenBatchIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasBatch();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        //empty data length with batch flag
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void givenEmptyContainerBody_whenViewIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasView();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        //empty data length with view flag
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void givenMsgContainerNoData_whenBatchIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasBatch();
        requestMsg.containerType(DataTypes.NO_DATA);
        encodeBatchView(requestMsg, BATCH_REQUEST_NAME_STR, null, DataTypes.ARRAY, false, "One", "Two");
        //container = NO_DATA
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void givenInvalidEntryName_whenBatchIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasBatch();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        encodeBatchView(requestMsg, "InvalidName", null, DataTypes.ARRAY, false);
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, convError.getCode());
        assertJsonError("Failed decoding Request message: batch data missing", convError);
    }

    @Test
    public void givenNoDataContainerType_whenBatchIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasBatch();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        //Because we in any case add array.
        encodeBatchView(requestMsg, BATCH_REQUEST_NAME_STR, null, DataTypes.NO_DATA, false);
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, convError.getCode());
        assertJsonError("Failed decoding Request message: batch data missing", convError);
        System.nanoTime();
    }

    @Test
    public void givenEmptyContainer_whenBatchIsApplied_thenRwfToJsonConversionFails() {
        //No array data
        requestMsg.applyHasBatch();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        encodeBatchView(requestMsg, BATCH_REQUEST_NAME_STR, null, DataTypes.ARRAY, true);
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, convError.getCode());
    }

    @Test
    public void givenInvalidViewNameEntry_whenBatchIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasView();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        encodeBatchView(requestMsg, "InvalidViewName", VIEW_TYPE_STR, DataTypes.ARRAY, false);
        //Invalid name is similar to absence of view entry data
        assertEquals(SUCCESS, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenNoDataContainer_whenViewIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasView();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        //Invalid name is similar to absence of view entry data
        encodeBatchView(requestMsg, VIEW_NAME_STR, VIEW_TYPE_STR, DataTypes.NO_DATA, false);
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
    }

    @Test
    public void givenInvalidEntryType_whenViewIsApplied_thenRwfToJsonConversionFails() {
        requestMsg.applyHasView();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);
        encodeBatchView(requestMsg, VIEW_NAME_STR, "InvalidViewType", DataTypes.ARRAY, false);
        assertEquals(FAILURE, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, convError.getCode());
        assertJsonError("Missing view type entry", convError);
    }

    @Test
    public void givenEmptyContainerBody_whenViewIsEnabled_thenRwfToJsonConversionSucceeds() {
        requestMsg.applyHasView();
        requestMsg.containerType(DataTypes.ELEMENT_LIST);

        //No array data
        encodeBatchView(requestMsg, VIEW_TYPE_STR, VIEW_NAME_STR, DataTypes.ARRAY, true);

        //If we don't have array, it is supposed that data empty and view elements is absence (ignoring)
        assertEquals(SUCCESS, converter.convertRWFToJson(requestMsg, rwfToJsonOptions, conversionResults, convError));
    }

    private void encodeBatchView(RequestMsg requestMsg, String entryName, String entryType, int dataType,
                                 boolean ignoreArray, String... names) {
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        Buffer dataBuffer = CodecFactory.createBuffer();
        Buffer itemBuffer = CodecFactory.createBuffer();
        dataBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        iter.setBufferAndRWFVersion(dataBuffer, Codec.majorVersion(), Codec.minorVersion());

        ElementList elementList = CodecFactory.createElementList();
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
        assertEquals(SUCCESS, elementList.encodeInit(iter, null, 0));

        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Buffer nameBuffer = CodecFactory.createBuffer();

        Array array = CodecFactory.createArray();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        //batch entry
        if (requestMsg.checkHasBatch()) {
            nameBuffer.data(entryName);
            elementEntry.name(nameBuffer);
            elementEntry.dataType(dataType);
            assertEquals(SUCCESS, elementEntry.encodeInit(iter, 0));

            if (!ignoreArray) {
                array.primitiveType(DataTypes.ASCII_STRING);
                assertEquals(SUCCESS, array.encodeInit(iter));

                for (String batchName : names) {
                    itemBuffer.clear();
                    itemBuffer.data(batchName);
                    assertEquals(SUCCESS, arrayEntry.encode(iter, itemBuffer));
                }

                assertEquals(SUCCESS, array.encodeComplete(iter, true));
            }

            assertEquals(SUCCESS, elementEntry.encodeComplete(iter, true));
        }

        //view entry
        if (requestMsg.checkHasView()) {
            nameBuffer.data(entryName);
            elementEntry.name(nameBuffer);
            elementEntry.dataType(DataTypes.UINT);
            UInt viewType = CodecFactory.createUInt();
            viewType.value(ViewTypes.FIELD_ID_LIST);
            assertEquals(SUCCESS, elementEntry.encode(iter, viewType));

            nameBuffer.data(entryType);
            elementEntry.name(nameBuffer);
            elementEntry.dataType(dataType);
            assertEquals(SUCCESS, elementEntry.encodeInit(iter, 0));

            if (!ignoreArray) {
                array.primitiveType(DataTypes.INT);
                assertEquals(SUCCESS, array.encodeInit(iter));

                for (String viewName : names) {
                    itemBuffer.clear();
                    itemBuffer.data(viewName);
                    assertEquals(SUCCESS, arrayEntry.encode(iter, itemBuffer));
                }
                assertEquals(SUCCESS, array.encodeComplete(iter, true));
            }

            assertEquals(SUCCESS, elementEntry.encodeComplete(iter, true));
        }

        assertEquals(SUCCESS, elementList.encodeComplete(iter, true));

        requestMsg.encodedDataBody(dataBuffer);
    }


    @Test
    //View element cannot be strings without dictionary.
    public void givenJsonWithStringView_whenJsonIsDecodingToRwf_thenDecodingSucceeds() throws IOException {
        final String requestMsg = "{\"ID\":5,\"Type\":\"Request\",\"Key\":{\"Service\":555,\"Name\":[\"SIX\",\"TWELVE\",\"FOURTEEN\"]},\"View\":[\"22\",\"25\"]}";
        jsonBuffer.data(requestMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenJsonWithIncorrect_whenJsonIsDecodingToRwf_thenDecodingSucceedsAndParameterIsFalse() {
        final String requestMsg = "{\"ID\":5,\"Type\":\"Request\",\"Qualified\":\"1\", \"Key\":{\"Service\":555,\"Identifier\":1205,\"Name\":[\"SIX\",\"TWELVE\",\"FOURTEEN\"]}}";
        jsonBuffer.data(requestMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertFalse(((RequestMsg) jsonMsg.rwfMsg()).checkQualifiedStream());
    }

    @Test
    public void givenNonBase64String_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        //Any string could be decoded as from base 64 string representation.
        final String requestMsg = "{\"ID\":5,\"Type\":\"Request\", \"ExtHdr\":\"\u0020\", \"Key\":{\"Service\":555,\"Identifier\":1205,\"Name\":[\"SIX\",\"TWELVE\",\"FOURTEEN\"]}}";
        jsonBuffer.data(requestMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenReqMsgWithoutMsgKey_whenJsonIsDecodingToRwf_thenDecodingFails() throws IOException {
        final String requestMsg = "{\"ID\":5,\"Type\":\"Request\"}";

        jsonBuffer.data(requestMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));

        //Integer cannot be decoded as base 64 string.
        assertEquals(FAILURE, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        errorParams.fillParams(convError, 5);
        convError.clear();
        assertEquals(SUCCESS, converter.getErrorMessage(errorBuffer, errorParams, convError));
        checkJsonErrorMsg(errorBuffer);
    }

    @Test
    public void givenMsgWithoutMsgType_whenJsonIsDecodingToRwf_thenDecodingSuccessful() throws IOException {
        final String requestMsg = "{\"ID\":1,\"Domain\":\"Login\",\"Key\":{\"Name\":\"root\",\"Elements\":{\"ApplicationId\":\"256\",\"Position\":\"10.185.225.161\"}}}";
        jsonBuffer.data(requestMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        assertEquals(SUCCESS, converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError));
        assertEquals(MsgClasses.REQUEST, jsonMsg.rwfMsg().msgClass());
    }
}
