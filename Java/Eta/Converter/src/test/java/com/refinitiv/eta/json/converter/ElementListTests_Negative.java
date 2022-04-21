/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.math.BigInteger;
import java.nio.ByteBuffer;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class ElementListTests_Negative {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    @Before
    public void init() {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "src/test/resources/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

        convError = ConverterFactory.createJsonConverterError();

        converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, true)
                .setDictionary(dictionary)
                .build(convError);
    }

    @After
    public void tearDown() {
        convError = null;
        converter = null;
    }

    @Test
    public void testError_notObject() throws JsonProcessingException {
        String wrongJson = "[{}]";
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.ELEMENT_LIST).encodeRWF(wrongNode, "", null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testError_entryNotObject() throws JsonProcessingException {
        String wrongJson = "{\"name\":[]}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        testWrongJsonData(iter, wrongJson, converter.getContainerHandler(DataTypes.ELEMENT_LIST), JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE);
    }

    @Test
    public void testError_entryMissingType() throws JsonProcessingException {
        String wrongJson = "{\"name\":{\"Data\":25}}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        testWrongJsonData(iter, wrongJson, converter.getContainerHandler(DataTypes.ELEMENT_LIST), JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY);
    }

    @Test
    public void testError_entryMissingData() throws JsonProcessingException {
        String wrongJson = "{\"name\":{\"Type\":\"UInt\"}}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        testWrongJsonData(iter, wrongJson, converter.getContainerHandler(DataTypes.ELEMENT_LIST), JsonConverterErrorCodes.JSON_ERROR_MISSING_KEY);
    }

    @Test
    public void testError_entryUnsupportedDataType() throws JsonProcessingException {
        String wrongJson = "{\"name\":{\"Type\":\"Unsupported\",\"Data\":1234}}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        testWrongJsonData(iter, wrongJson, converter.getContainerHandler(DataTypes.ELEMENT_LIST), JsonConverterErrorCodes.JSON_ERROR_UNSUPPORTED_DATA_TYPE);
    }

    @Test
    public void testError_entryUintValueOutsideRange() throws JsonProcessingException {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        AbstractContainerTypeConverter containerHandler = converter.getContainerHandler(DataTypes.ELEMENT_LIST);
        int expectedErrorCode = JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE;

        String wrongJson = "{\"name\":{\"Type\":\"UInt\",\"Data\": 18446744073709551616}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"UInt1\",\"Data\": -1}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"UInt1\",\"Data\": 256}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"UInt2\",\"Data\": 65536}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"UInt4\",\"Data\": 4294967296}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"UInt8\",\"Data\": 18446744073709551616}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);
    }

    @Test
    public void testError_entryIntValueOutsideRange() throws JsonProcessingException {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        AbstractContainerTypeConverter containerHandler = converter.getContainerHandler(DataTypes.ELEMENT_LIST);
        int expectedErrorCode = JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE;

        String wrongJson = "{\"name\":{\"Type\":\"Int\",\"Data\": 18446744073709551616}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int\",\"Data\": 125.1212}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int\",\"Data\": 9223372036854775808}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int\",\"Data\": -9223372036854775809}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int1\",\"Data\": 9223372036854775807}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int1\",\"Data\": -129}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int2\",\"Data\": 32768}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int2\",\"Data\": -32769}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int4\",\"Data\": 2147483648}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int4\",\"Data\": -2147483649}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int8\",\"Data\": 9223372036854775808}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);

        wrongJson = "{\"name\":{\"Type\":\"Int8\",\"Data\": -9223372036854775809}}";
        testWrongJsonData(iter, wrongJson, containerHandler, expectedErrorCode);
    }

    private void testWrongJsonData(EncodeIterator iter, String wrongJson,
            AbstractContainerTypeConverter containerHandler, int jsonConverterErrorCode) throws JsonProcessingException {
        convError.clear();
        JsonNode wrongNode = mapper.readTree(wrongJson);
        containerHandler.encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(jsonConverterErrorCode, convError.getCode());
    }
}
