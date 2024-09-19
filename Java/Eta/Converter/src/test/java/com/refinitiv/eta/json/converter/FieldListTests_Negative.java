/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.core.json.JsonReadFeature;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.*;

public class FieldListTests_Negative {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    @Before
    public void init() {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "../../etc/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

        convError = ConverterFactory.createJsonConverterError();

        converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, true)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, true)
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
        converter.getContainerHandler(DataTypes.FIELD_LIST).encodeRWF(wrongNode, "", null, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testError_catchUnexpectedFid() throws JsonProcessingException {

        String wrongJson = "{\"CASH_BASIS\":\"ascii string\",\"UNEXPECTED_FID\":\"2020-01-02\",\"TRDTIM_1\":\"00:00:02.0000025\"}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        converter.getContainerHandler(DataTypes.FIELD_LIST).encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_FID, convError.getCode());
    }

    @Test
    public void testError_fieldsOutsideBounds() throws JsonProcessingException {
        String wrongJson = "{\"RDN_EXCHID\":-100,\"PRCTCK_1\":65537,\"TRD_UNITS\":00}";
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(200));
        EncodeIterator iter = CodecFactory.createEncodeIterator();
        iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        mapper.enable(JsonReadFeature.ALLOW_LEADING_ZEROS_FOR_NUMBERS.mappedFeature());
        JsonNode wrongNode = mapper.readTree(wrongJson);
        AbstractContainerTypeConverter containerHandler = converter.getContainerHandler(DataTypes.FIELD_LIST);
        containerHandler.encodeRWF(wrongNode, "", iter, convError);
        assertTrue(convError.isFailed());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, convError.getCode());
    }

    @Test
    public void testRwfToJson_UnexpectedFid() {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(100));
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[100];
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Buffer rmtes = CodecFactory.createBuffer();
        rmtes.data("rmtes string");

        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        fieldList.applyHasStandardData();
        fieldList.encodeInit(encIter, null, 0);
        fieldEntry.clear();
        fieldEntry.dataType(DataTypes.RMTES_STRING);
        fieldEntry.fieldId(-32766);
        fieldEntry.encode(encIter, rmtes);
        fieldList.encodeComplete(encIter, true);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        assertFalse(converter.getContainerHandler(DataTypes.FIELD_LIST).encodeJson(decIter, outBuffer, false, null, convError));
        assertFalse(convError.isSuccessful());
        assertEquals(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_FID, convError.getCode());
        assertTrue(convError.getText().contains("-32766"));
    }
}