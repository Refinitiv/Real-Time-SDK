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

import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_ENTRIES;
import static com.refinitiv.eta.json.converter.ConstCharArrays.JSON_SUMMARY;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class VectorTests {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    public class MessageParameters
    {
        public int id;
        public int containerType;
        public int[] entryActions;
        public boolean[] permDataPresent;
        public boolean supportSorting;
        public boolean hasSummary;
        public boolean hasTotalHintCount;

    }
    VectorTests.MessageParameters msgParameters = new VectorTests.MessageParameters();

    public VectorTests(
            int id,
            int containerType,
            int[] entryActions,
            boolean[] permDataPresent,
            boolean supportSorting,
            boolean hasSummary,
            boolean hasTotalHintCount
    ) {
        msgParameters.id = id;
        msgParameters.entryActions = entryActions;
        msgParameters.permDataPresent = permDataPresent;
        msgParameters.supportSorting = supportSorting;
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
        DecodeIterator decIter = generateMapDecodeIterator();
        JsonBuffer outBuffer = new JsonBuffer();
        outBuffer.data = new byte[10000];
        assertEquals(true, converter.getContainerHandler(DataTypes.VECTOR).encodeJson(decIter, outBuffer, false, null, convError));

        JsonNode root = mapper.readTree(outBuffer.data);
        checkJsonNode(root);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.VECTOR).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());
        checkRWFData(buffer);
    }

    private void checkJsonNode(JsonNode root) {

        JsonNode summary = root.path(JSON_SUMMARY);
        assertEquals(!msgParameters.hasSummary, summary.isMissingNode());
        JsonNode entries = root.path(JSON_ENTRIES);
        assertTrue(!entries.isMissingNode());
        assertTrue(entries.isArray());
        assertTrue(entries.size() == msgParameters.entryActions.length);
    }

    private void checkRWFData(Buffer buffer) {

        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.decodeAndCheckVector(decIter,
                msgParameters.containerType,
                msgParameters.hasSummary,
                msgParameters.hasTotalHintCount,
                msgParameters.supportSorting,
                msgParameters.entryActions,
                msgParameters.permDataPresent);
    }

    private DecodeIterator generateMapDecodeIterator() {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.encodeSimpleVector(encIter,
                msgParameters.containerType,
                msgParameters.hasSummary,
                msgParameters.hasTotalHintCount,
                msgParameters.supportSorting,
                msgParameters.entryActions,
                msgParameters.permDataPresent
               );

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        return decIter;
    }

    @Parameterized.Parameters
    public static Collection testCases() {

        return Arrays.asList(new Object[][] {
                //id, containerType,   actions, hasPermData, supportSorting, hasSummary, hasTotalCount
                {0, DataTypes.FIELD_LIST, new int[]{VectorEntryActions.DELETE, VectorEntryActions.DELETE, VectorEntryActions.DELETE}, new boolean[] {true, false, true}, true, true, true  },
                {1, DataTypes.SERIES, new int[]{VectorEntryActions.SET, VectorEntryActions.DELETE, VectorEntryActions.CLEAR}, new boolean[] {true, false, true}, true, false, true  },
                {2, DataTypes.ELEMENT_LIST, new int[]{VectorEntryActions.SET, VectorEntryActions.UPDATE, VectorEntryActions.CLEAR}, new boolean[] {false, false, true}, false, true, false  },
                {3, DataTypes.FILTER_LIST, new int[]{VectorEntryActions.SET, VectorEntryActions.UPDATE, VectorEntryActions.CLEAR}, new boolean[] {false, false, true}, false, true, false  },
                {4, DataTypes.MAP, new int[]{VectorEntryActions.SET, VectorEntryActions.INSERT, VectorEntryActions.CLEAR}, new boolean[] {false, false, true}, false, true, false  },
                {5, DataTypes.MSG, new int[]{VectorEntryActions.SET, VectorEntryActions.INSERT, VectorEntryActions.CLEAR}, new boolean[] {false, false, true}, false, true, false  },
                {6, DataTypes.OPAQUE, new int[]{VectorEntryActions.SET, VectorEntryActions.INSERT, VectorEntryActions.CLEAR}, new boolean[] {false, false, true}, false, true, false  },
                {7, DataTypes.XML, new int[]{VectorEntryActions.SET, VectorEntryActions.INSERT, VectorEntryActions.CLEAR}, new boolean[] {false, false, true}, false, true, false  },
                {8, DataTypes.JSON, new int[]{VectorEntryActions.SET, VectorEntryActions.INSERT, VectorEntryActions.CLEAR}, new boolean[] {false, false, true}, false, true, false  },
                {9, DataTypes.ANSI_PAGE, new int[]{VectorEntryActions.SET, VectorEntryActions.INSERT, VectorEntryActions.CLEAR}, new boolean[] {false, false, true}, false, true, false  }
        });
    }
}

