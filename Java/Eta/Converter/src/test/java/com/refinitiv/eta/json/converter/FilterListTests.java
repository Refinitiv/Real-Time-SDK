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

import static org.junit.Assert.assertEquals;

@RunWith(Parameterized.class)
public class FilterListTests {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    public class MessageParameters
    {
        public int id;
        public int[] dataTypes;
        public int[] filterActions;
        public boolean[] hasPermCount;
        public int containerType;
        public int countHint;
        public boolean hasCountHint;
    }
    FilterListTests.MessageParameters msgParameters = new FilterListTests.MessageParameters();

    public FilterListTests(
            int id,
            int[] dataTypes,
            int[] filterActions,
            boolean[] hasPermCount,
            int containerType,
            int countHint,
            boolean hasCountHint
    ) {
        msgParameters.id = id;
        msgParameters.dataTypes = dataTypes;
        msgParameters.filterActions = filterActions;
        msgParameters.hasPermCount = hasPermCount;
        msgParameters.containerType = containerType;
        msgParameters.countHint = countHint;
        msgParameters.hasCountHint = hasCountHint;
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
        DecodeIterator decIter = generateFilterListDecodeIterator();
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[20000];
        assertEquals(true, converter.getContainerHandler(DataTypes.FILTER_LIST).encodeJson(decIter, outBuffer, false, null, convError));

        JsonNode root = mapper.readTree(outBuffer.data);
        checkJsonNode(root);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(5000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.FILTER_LIST).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());
        checkRWFData(buffer);
    }

    private void checkJsonNode(JsonNode root) {


    }

    private void checkRWFData(Buffer buffer) {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.decodeAndCheckFilterList(decIter,
                msgParameters.hasCountHint,
                msgParameters.countHint,
                msgParameters.containerType,
                msgParameters.filterActions,
                msgParameters.dataTypes,
                msgParameters.hasPermCount);
    }

    private DecodeIterator generateFilterListDecodeIterator() {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.encodeSimpleFilterList(encIter,
                msgParameters.containerType,
                msgParameters.hasCountHint,
                msgParameters.filterActions,
                msgParameters.dataTypes,
                msgParameters.hasPermCount,
                msgParameters.countHint);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        return decIter;
    }

    @Parameterized.Parameters
    public static Collection testCases() {

//        int id,
//        int[] dataTypes,
//        int[] filterActions,
//        boolean[] hasPermCount,
//        int containerType,
//        int countHint,
//        boolean hasCountHint

        return Arrays.asList(new Object[][] {
                {0,
                new int[] {DataTypes.ELEMENT_LIST, DataTypes.ELEMENT_LIST, DataTypes.ELEMENT_LIST},
                new int[] { FilterEntryActions.CLEAR, FilterEntryActions.UPDATE, FilterEntryActions.UPDATE},
                new boolean[] {true, false, true},
                DataTypes.ELEMENT_LIST,
                3,
                true },
                {1,
                new int[] {DataTypes.MAP, DataTypes.FIELD_LIST, DataTypes.MAP},
                new int[] { FilterEntryActions.SET, FilterEntryActions.UPDATE, FilterEntryActions.CLEAR},
                new boolean[] {true, false, true},
                DataTypes.VECTOR,
                10,
                true },
                {1,
                new int[] {DataTypes.VECTOR, DataTypes.VECTOR, DataTypes.VECTOR},
                new int[] { FilterEntryActions.SET, FilterEntryActions.UPDATE, FilterEntryActions.SET},
                new boolean[] {true, true, true},
                DataTypes.MAP,
                10,
                true },
                {0,
                new int[] {DataTypes.FIELD_LIST, DataTypes.ELEMENT_LIST, DataTypes.FIELD_LIST},
                new int[] { FilterEntryActions.UPDATE, FilterEntryActions.UPDATE, FilterEntryActions.UPDATE},
                new boolean[] {true, false, true},
                DataTypes.FIELD_LIST,
                3,
                true },
                {0,
                new int[] {DataTypes.MSG, DataTypes.ELEMENT_LIST, DataTypes.FIELD_LIST},
                new int[] { FilterEntryActions.UPDATE, FilterEntryActions.UPDATE, FilterEntryActions.UPDATE},
                new boolean[] {true, false, true},
                DataTypes.FIELD_LIST,
                3,
                true },
                {0,
                 new int[] {DataTypes.SERIES, DataTypes.SERIES, DataTypes.SERIES},
                 new int[] { FilterEntryActions.UPDATE, FilterEntryActions.UPDATE, FilterEntryActions.UPDATE},
                 new boolean[] {true, false, true},
                 DataTypes.SERIES,
                 3,
                 true },
                {0,
                 new int[] {DataTypes.FILTER_LIST, DataTypes.FILTER_LIST, DataTypes.FILTER_LIST},
                 new int[] { FilterEntryActions.UPDATE, FilterEntryActions.UPDATE, FilterEntryActions.UPDATE},
                 new boolean[] {true, false, true},
                 DataTypes.FILTER_LIST,
                 3,
                 true },
                {0,
                 new int[] {DataTypes.XML, DataTypes.XML, DataTypes.XML},
                 new int[] { FilterEntryActions.SET, FilterEntryActions.CLEAR, FilterEntryActions.UPDATE},
                 new boolean[] {true, false, true},
                 DataTypes.XML,
                 3,
                 true },
                {0,
                 new int[] {DataTypes.OPAQUE, DataTypes.OPAQUE, DataTypes.OPAQUE},
                 new int[] { FilterEntryActions.SET, FilterEntryActions.CLEAR, FilterEntryActions.UPDATE},
                 new boolean[] {true, false, true},
                 DataTypes.OPAQUE,
                 3,
                 true },
                {0,
                 new int[] {DataTypes.JSON, DataTypes.JSON, DataTypes.JSON},
                 new int[] { FilterEntryActions.SET, FilterEntryActions.CLEAR, FilterEntryActions.UPDATE},
                 new boolean[] {true, false, true},
                 DataTypes.JSON,
                 3,
                 true },
                {0,
                 new int[] {DataTypes.MSG, DataTypes.MSG, DataTypes.MSG},
                 new int[] { FilterEntryActions.SET, FilterEntryActions.CLEAR, FilterEntryActions.UPDATE},
                 new boolean[] {true, false, true},
                 DataTypes.MSG,
                 3,
                 true },
                {0,
                 new int[] {DataTypes.ANSI_PAGE, DataTypes.ANSI_PAGE, DataTypes.ANSI_PAGE},
                 new int[] { FilterEntryActions.SET, FilterEntryActions.CLEAR, FilterEntryActions.UPDATE},
                 new boolean[] {true, false, true},
                 DataTypes.ANSI_PAGE,
                 3,
                 true }
        });
    }
}
