/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collection;

import static com.refinitiv.eta.json.converter.ConstCharArrays.*;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class SeriesTests {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    public class MessageParameters
    {
        public int id;
        public int containerType;
        public int length;
        public int countHint;
        public boolean hasSummary;
        public boolean hasTotalHintCount;

    }
    SeriesTests.MessageParameters msgParameters = new SeriesTests.MessageParameters();

    public SeriesTests(
            int id,
            int containerType,
            int length,
            int countHint,
            boolean hasSummary,
            boolean hasTotalHintCount
    ) {
        msgParameters.id = id;
        msgParameters.length = length;
        msgParameters.countHint = countHint;
        msgParameters.hasSummary = hasSummary;
        msgParameters.containerType = containerType;
        msgParameters.hasTotalHintCount = hasTotalHintCount;
    }

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
                .setDictionary(dictionary)
                .build(convError);
    }

    @Test
    public void testRwfToJson_Sunny() throws IOException {
        DecodeIterator decIter = generateSeriesDecodeIterator();
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[10000];
        assertEquals(true, converter.getContainerHandler(DataTypes.SERIES).encodeJson(decIter, outBuffer, false, null, convError));

        JsonNode root = mapper.readTree(outBuffer.data);
        checkJsonNode(root);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.SERIES).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());
        checkRWFData(buffer);
    }

    private void checkJsonNode(JsonNode root) {

        JsonNode summary = root.path(JSON_SUMMARY);
        assertEquals(!msgParameters.hasSummary, summary.isMissingNode());
        JsonNode entries = root.path(JSON_ENTRIES);
        assertTrue(!entries.isMissingNode());
        assertTrue(entries.isArray());
        JsonNode countHint = root.path(JSON_COUNTHINT);
        assertEquals(!countHint.isMissingNode(), msgParameters.hasTotalHintCount);
    }

    private void checkRWFData(Buffer buffer) {

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.decodeAndCheckSeries(decIter,
                msgParameters.containerType,
                msgParameters.countHint,
                msgParameters.length,
                msgParameters.hasSummary,
                msgParameters.hasTotalHintCount);
    }

    private DecodeIterator generateSeriesDecodeIterator() {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.encodeSimpleSeries(encIter,
                msgParameters.containerType,
                msgParameters.hasTotalHintCount,
                msgParameters.hasSummary,
                msgParameters.countHint,
                msgParameters.length);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        return decIter;
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][] {
//        int id,
//        int containerType,
//        int length,
//        int countHint,
//        boolean hasSummary,
//        boolean hasTotalHintCount
                {0, DataTypes.VECTOR, 2, 3, true, true},
                {1, DataTypes.MAP, 3, 3, false, true},
                {2, DataTypes.ELEMENT_LIST, 2, 3, true, false},
                {3, DataTypes.FIELD_LIST, 2, 3, false, false},
                {4, DataTypes.FILTER_LIST, 3, 2, true, false},
                {5, DataTypes.OPAQUE, 3, 2, true, false},
                {6, DataTypes.JSON, 3, 2, true, false},
                {7, DataTypes.XML, 3, 2, true, false},
                {8, DataTypes.MSG, 3, 2, true, false},
                {9, DataTypes.ANSI_PAGE, 3, 2, true, false}
        });
    }
}

