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
import static org.junit.Assert.*;

@RunWith(Parameterized.class)
public class MapTests {

    JsonConverterError convError;
    JsonAbstractConverter converter;
    ObjectMapper mapper = new ObjectMapper();

    public class MessageParameters
    {
        public int id;
        public int containerType;
        public int[] entryActions;
        public boolean[] permDataPresent;
        public int keyType;
        public boolean hasSummary;
        public boolean hasKeyFieldId;
        public boolean hasTotalHintCount;

    }
    MapTests.MessageParameters msgParameters = new MapTests.MessageParameters();

    public MapTests(
            int id,
            int containerType,
            int[] entryActions,
            boolean[] permDataPresent,
            int keyType,
            boolean hasSummary,
            boolean hasKeyFieldId,
            boolean hasTotalHintCount
    ) {
        msgParameters.id = id;
        msgParameters.entryActions = entryActions;
        msgParameters.permDataPresent = permDataPresent;
        msgParameters.keyType = keyType;
        msgParameters.hasSummary = hasSummary;
        msgParameters.containerType = containerType;
        msgParameters.hasKeyFieldId = hasKeyFieldId;
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
        assertEquals(true, converter.getContainerHandler(DataTypes.MAP).encodeJson(decIter, outBuffer, false, null, convError));

        JsonNode root = mapper.readTree(outBuffer.data);
        checkJsonNode(root);

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        converter.getContainerHandler(DataTypes.MAP).encodeRWF(root, null, encIter, convError);
        assertEquals(true, convError.isSuccessful());
        checkRWFData(buffer);
    }

    private void checkJsonNode(JsonNode root) {

        JsonNode keyType = root.path(JSON_KEYTYPE);
        assertFalse(keyType.isMissingNode());
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

        CodecUtil.decodeAndCheckMap(decIter,
                msgParameters.entryActions,
                msgParameters.permDataPresent,
                msgParameters.keyType,
                msgParameters.containerType, msgParameters.hasSummary,
                msgParameters.hasTotalHintCount,
                msgParameters.hasKeyFieldId);
    }

    private DecodeIterator generateMapDecodeIterator() {
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10000));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        CodecUtil.encodeSimpleMap(encIter,
                msgParameters.containerType,
                msgParameters.entryActions,
                msgParameters.permDataPresent,
                msgParameters.keyType,
                msgParameters.hasSummary,
                msgParameters.hasKeyFieldId,
                msgParameters.hasTotalHintCount);

        decIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        return decIter;
    }

    @Parameterized.Parameters
    public static Collection testCases() {
        return Arrays.asList(new Object[][] {
                //{id, containerType, entryActions, permDataPresent, int keyType, boolean hasSummary, boolean hasKeyFieldId, boolean hasTotalHintCount
                {0, DataTypes.VECTOR, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.STATE, false, true, true },
                {1, DataTypes.VECTOR, new int[]{MapEntryActions.DELETE, MapEntryActions.DELETE}, new boolean[]{true, false}, DataTypes.QOS, true, true, false },
                {2, DataTypes.ELEMENT_LIST, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.INT, false, true, false },
                {3, DataTypes.VECTOR, new int[]{MapEntryActions.UPDATE, MapEntryActions.UPDATE}, new boolean[]{true, false}, DataTypes.STATE, false, true, false },
                {4, DataTypes.SERIES, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.INT, true, true, false },
                {5, DataTypes.FILTER_LIST, new int[]{MapEntryActions.UPDATE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.DATE, false, true, true },
                {6, DataTypes.SERIES, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.TIME, true, true, false },
                {7, DataTypes.VECTOR, new int[]{MapEntryActions.DELETE, MapEntryActions.UPDATE}, new boolean[]{true, false}, DataTypes.DATETIME,  true, false, false },
                {8, DataTypes.ELEMENT_LIST, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.REAL, true, true, true },
                {9, DataTypes.FIELD_LIST, new int[]{MapEntryActions.ADD, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.FLOAT, true, true, false },
                {10, DataTypes.VECTOR, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD, MapEntryActions.UPDATE}, new boolean[]{true, false, true}, DataTypes.DOUBLE, true, true, false },
                {11, DataTypes.VECTOR, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD, MapEntryActions.UPDATE}, new boolean[]{true, false, true}, DataTypes.REAL, false, false, false },
                {12, DataTypes.ELEMENT_LIST, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD, MapEntryActions.UPDATE}, new boolean[]{true, false, true}, DataTypes.TIME, true, true, true },
                {13, DataTypes.MSG, new int[]{MapEntryActions.ADD, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.STATE, false, true, true },
                {14, DataTypes.ELEMENT_LIST, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.UINT, false, true, false },
                {15, DataTypes.OPAQUE, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.UINT, false, true, false },
                {16, DataTypes.XML, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.UINT, false, true, false },
                {17, DataTypes.JSON, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.UINT, false, true, false },
                {18, DataTypes.ANSI_PAGE, new int[]{MapEntryActions.DELETE, MapEntryActions.ADD}, new boolean[]{true, false}, DataTypes.UINT, false, true, false }
        });
    }
}

